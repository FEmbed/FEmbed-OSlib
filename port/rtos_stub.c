/**
 *	FreeRTOS need port function
 */ 
#include <assert.h>
#include <malloc.h>
#include "FreeRTOS.h"
#include "task.h"

#if CONFIG_RTOS_LIB_FREERTOS

/*******************************************************************************
 * Create New For some multi-region memory malloc and free.
 ******************************************************************************/
#define heapMINIMUM_BLOCK_SIZE  ( ( size_t ) ( xHeapStructSize << 1 ) )
/* Assumes 8bit bytes! */
#define heapBITS_PER_BYTE       ( ( size_t ) 8 )

/* Define the linked list structure.  This is used to link free blocks in order of their memory address. */
typedef struct A_BLOCK_LINK
{
    struct A_BLOCK_LINK *pxNextFreeBlock;       /*<< The next free block in the list. */
    size_t xBlockSize;                           /*<< The size of the free block. */
} BlockLink_t;

static void prvInsertBlockIntoFreeList( BlockLink_t *pxBlockToInsert );
static const size_t xHeapStructSize = ( sizeof( BlockLink_t ) + ( ( size_t ) ( portBYTE_ALIGNMENT - 1 ) ) ) & ~( ( size_t ) portBYTE_ALIGNMENT_MASK );
static BlockLink_t xStart, *pxEnd = NULL;

static size_t xFreeBytesRemaining = 0U;
static size_t xMinimumEverFreeBytesRemaining = 0U;
static size_t xBlockAllocatedBit = 0;

/* reference heap_5.c for details */
void *common_alloc(size_t xWantedSize, void *xWantedStart, void *xWantedEnd) {
    BlockLink_t *pxBlock, *pxPreviousBlock, *pxNewBlockLink;
    void *pvReturn = NULL;

    configASSERT( pxEnd );

    vTaskSuspendAll();
    {
        if( ( xWantedSize & xBlockAllocatedBit ) == 0 )
        {
            if( xWantedSize > 0 )
            {
                xWantedSize += xHeapStructSize;
                if( ( xWantedSize & portBYTE_ALIGNMENT_MASK ) != 0x00 )
                {
                    /* Byte alignment required. */
                    xWantedSize += ( portBYTE_ALIGNMENT - ( xWantedSize & portBYTE_ALIGNMENT_MASK ) );
                }
            }
            else
            {
                return NULL;    ///< no wanted size
            }

            if( ( xWantedSize > 0 ) && ( xWantedSize <= xFreeBytesRemaining ) )
            {
                /* Traverse the list from the start (lowest address) block until one of adequate size is found. */
                pxPreviousBlock = &xStart;
                pxBlock = xStart.pxNextFreeBlock;
                while( !(pxBlock >= xWantedStart && pxBlock < xWantedEnd) ||
                         (( pxBlock->xBlockSize < xWantedSize ) && ( pxBlock->pxNextFreeBlock != NULL )) )
                {
                    pxPreviousBlock = pxBlock;
                    pxBlock = pxBlock->pxNextFreeBlock;
                }

                /* If the end marker was reached then a block of adequate size was not found. */
                if( pxBlock != pxEnd )
                {
                    /* Return the memory space pointed to - jumping over the
                    BlockLink_t structure at its start. */
                    pvReturn = ( void * ) ( ( ( uint8_t * ) pxPreviousBlock->pxNextFreeBlock ) + xHeapStructSize );

                    /* This block is being returned for use so must be taken out
                    of the list of free blocks. */
                    pxPreviousBlock->pxNextFreeBlock = pxBlock->pxNextFreeBlock;

                    /* If the block is larger than required it can be split into
                    two. */
                    if( ( pxBlock->xBlockSize - xWantedSize ) > heapMINIMUM_BLOCK_SIZE )
                    {
                        /* This block is to be split into two.  Create a new
                        block following the number of bytes requested. The void
                        cast is used to prevent byte alignment warnings from the
                        compiler. */
                        pxNewBlockLink = ( void * ) ( ( ( uint8_t * ) pxBlock ) + xWantedSize );

                        /* Calculate the sizes of two blocks split from the
                        single block. */
                        pxNewBlockLink->xBlockSize = pxBlock->xBlockSize - xWantedSize;
                        pxBlock->xBlockSize = xWantedSize;

                        /* Insert the new block into the list of free blocks. */
                        prvInsertBlockIntoFreeList( ( pxNewBlockLink ) );
                    }

                    xFreeBytesRemaining -= pxBlock->xBlockSize;

                    if( xFreeBytesRemaining < xMinimumEverFreeBytesRemaining )
                    {
                        xMinimumEverFreeBytesRemaining = xFreeBytesRemaining;
                    }
                    else
                    {
                        mtCOVERAGE_TEST_MARKER();
                    }

                    /* The block is being returned - it is allocated and owned
                    by the application and has no "next" block. */
                    pxBlock->xBlockSize |= xBlockAllocatedBit;
                    pxBlock->pxNextFreeBlock = NULL;
                }
            }
        }

        traceMALLOC( pvReturn, xWantedSize );
    }
    ( void ) xTaskResumeAll();

    #if( configUSE_MALLOC_FAILED_HOOK == 1 )
    {
        if( pvReturn == NULL )
        {
            extern void vApplicationMallocFailedHook( void );
            vApplicationMallocFailedHook();
        }
    }
    #endif

    return pvReturn;
}

void rtos_free(void *pv) {
    uint8_t *puc = ( uint8_t * ) pv;
    BlockLink_t *pxLink;

    if( pv != NULL )
    {
        /* The memory being freed will have an BlockLink_t structure immediately before it. */
        puc -= xHeapStructSize;

        /* This casting is to keep the compiler from issuing warnings. */
        pxLink = ( void * ) puc;

        /* Check the block is actually allocated. */
        configASSERT( ( pxLink->xBlockSize & xBlockAllocatedBit ) != 0 );
        configASSERT( pxLink->pxNextFreeBlock == NULL );

        if( ( pxLink->xBlockSize & xBlockAllocatedBit ) != 0 )
        {
            if( pxLink->pxNextFreeBlock == NULL )
            {
                /* The block is being returned to the heap - it is no longer allocated. */
                pxLink->xBlockSize &= ~xBlockAllocatedBit;

                vTaskSuspendAll();
                {
                    /* Add this block to the list of free blocks. */
                    xFreeBytesRemaining += pxLink->xBlockSize;
                    traceFREE( pv, pxLink->xBlockSize );
                    prvInsertBlockIntoFreeList( ( ( BlockLink_t * ) pxLink ) );
                }
                ( void ) xTaskResumeAll();
            }
        }
    }
}

extern char         DMA_START[];
extern char         DMA_LIMIT[];
void *rtos_alloc(size_t xWantedSize)
{
    return common_alloc(xWantedSize, NULL, (void *)0xffffffff);
}

void *dma_alloc(size_t xWantedSize)
{
    return common_alloc(xWantedSize, DMA_START, DMA_START + (uint32_t)DMA_LIMIT);
}

void dma_free(void *pv)
{
    return rtos_free(pv);
}

size_t xPortGetFreeHeapSize( void )
{
    return xFreeBytesRemaining;
}

size_t xPortGetMinimumEverFreeHeapSize( void )
{
    return xMinimumEverFreeBytesRemaining;
}

static void prvInsertBlockIntoFreeList( BlockLink_t *pxBlockToInsert )
{
BlockLink_t *pxIterator;
uint8_t *puc;

    /* Iterate through the list until a block is found that has a higher address
    than the block being inserted. */
    for( pxIterator = &xStart; pxIterator->pxNextFreeBlock < pxBlockToInsert; pxIterator = pxIterator->pxNextFreeBlock )
    {
        /* Nothing to do here, just iterate to the right position. */
    }

    /* Do the block being inserted, and the block it is being inserted after
    make a contiguous block of memory? */
    puc = ( uint8_t * ) pxIterator;
    if( ( puc + pxIterator->xBlockSize ) == ( uint8_t * ) pxBlockToInsert )
    {
        pxIterator->xBlockSize += pxBlockToInsert->xBlockSize;
        pxBlockToInsert = pxIterator;
    }
    else
    {
        mtCOVERAGE_TEST_MARKER();
    }

    /* Do the block being inserted, and the block it is being inserted before
    make a contiguous block of memory? */
    puc = ( uint8_t * ) pxBlockToInsert;
    if( ( puc + pxBlockToInsert->xBlockSize ) == ( uint8_t * ) pxIterator->pxNextFreeBlock )
    {
        if( pxIterator->pxNextFreeBlock != pxEnd )
        {
            /* Form one big block from the two blocks. */
            pxBlockToInsert->xBlockSize += pxIterator->pxNextFreeBlock->xBlockSize;
            pxBlockToInsert->pxNextFreeBlock = pxIterator->pxNextFreeBlock->pxNextFreeBlock;
        }
        else
        {
            pxBlockToInsert->pxNextFreeBlock = pxEnd;
        }
    }
    else
    {
        pxBlockToInsert->pxNextFreeBlock = pxIterator->pxNextFreeBlock;
    }

    /* If the block being inserted plugged a gab, so was merged with the block
    before and the block after, then it's pxNextFreeBlock pointer will have
    already been set, and should not be set here as that would make it point
    to itself. */
    if( pxIterator != pxBlockToInsert )
    {
        pxIterator->pxNextFreeBlock = pxBlockToInsert;
    }
    else
    {
        mtCOVERAGE_TEST_MARKER();
    }
}

void *pvPortMalloc(size_t xWantedSize) {
	void *ptr;
	ptr = malloc(xWantedSize);
	return ptr;
}

void vPortFree(void *pv) {
	free(pv);
}

void vApplicationMallocFailedHook( void )
{
    assert(0);
}

void vApplicationStackOverflowHook( TaskHandle_t xTask, char *pcTaskName )
{
    assert(0);
}

/**
 * Idle Task use mini stack size.
 */
static __ccm StaticTask_t xIdleTaskTCB;
static __ccm StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];
void vApplicationGetIdleTaskMemory(
        StaticTask_t **ppxIdleTaskTCBBuffer,
        StackType_t **ppxIdleTaskStackBuffer,
        uint32_t *pulIdleTaskStackSize )
{
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

void *pvGetIdleTaskHandler()
{
    return &xIdleTaskTCB;
}

/**
 * Must implement timer for common use.
 */
static __ccm StaticTask_t xTimerTaskTCB;
static __ccm StackType_t uxTimerTaskStack[ configTIMER_TASK_STACK_DEPTH ];
void vApplicationGetTimerTaskMemory(
        StaticTask_t **ppxTimerTaskTCBBuffer,
        StackType_t **ppxTimerTaskStackBuffer,
        uint32_t *pulTimerTaskStackSize )
{
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

void *pvGetTimerTaskHandler()
{
    return &xTimerTaskTCB;
}

/******************************************************************************
 * freertos init function, will auto called by init function.
 ******************************************************************************/
extern unsigned int __ram_regions_array_start;
extern unsigned int __ram_regions_array_end;

#include "driver.h"
void freertos_global_init()
{
    BlockLink_t *pxFirstFreeBlockInRegion = NULL, *pxPreviousFreeBlock;
    size_t xAlignedHeap;
    size_t xTotalRegionSize, xTotalHeapSize = 0;
    BaseType_t xDefinedRegions = 0;
    size_t xAddress;

    /* Can only call once! */
    configASSERT( pxEnd == NULL );

    for(unsigned *p = &__ram_regions_array_start;
                  p < &__ram_regions_array_end;)
    {
        unsigned *region_begin = (unsigned int*) (*p++);
        uint32_t region_size = (uint32_t) (*p++);

        if( region_size > 0 )
        {
            xTotalRegionSize = region_size;

            /* Ensure the heap region starts on a correctly aligned boundary. */
            xAddress = ( size_t ) region_begin;
            if( ( xAddress & portBYTE_ALIGNMENT_MASK ) != 0 )
            {
                xAddress += ( portBYTE_ALIGNMENT - 1 );
                xAddress &= ~portBYTE_ALIGNMENT_MASK;

                /* Adjust the size for the bytes lost to alignment. */
                xTotalRegionSize -= xAddress - ( size_t ) region_begin;
            }

            xAlignedHeap = xAddress;

            /* Set xStart if it has not already been set. */
            if( xDefinedRegions == 0 )
            {
                /* xStart is used to hold a pointer to the first item in the list of
                free blocks.  The void cast is used to prevent compiler warnings. */
                xStart.pxNextFreeBlock = ( BlockLink_t * ) xAlignedHeap;
                xStart.xBlockSize = ( size_t ) 0;
            }
            else
            {
                /* Should only get here if one region has already been added to the
                heap. */
                configASSERT( pxEnd != NULL );

                /* Check blocks are passed in with increasing start addresses. */
                configASSERT( xAddress > ( size_t ) pxEnd );
            }

            /* Remember the location of the end marker in the previous region, if
            any. */
            pxPreviousFreeBlock = pxEnd;

            /* pxEnd is used to mark the end of the list of free blocks and is
            inserted at the end of the region space. */
            xAddress = xAlignedHeap + xTotalRegionSize;
            xAddress -= xHeapStructSize;
            xAddress &= ~portBYTE_ALIGNMENT_MASK;
            pxEnd = ( BlockLink_t * ) xAddress;
            pxEnd->xBlockSize = 0;
            pxEnd->pxNextFreeBlock = NULL;

            /* To start with there is a single free block in this region that is
            sized to take up the entire heap region minus the space taken by the
            free block structure. */
            pxFirstFreeBlockInRegion = ( BlockLink_t * ) xAlignedHeap;
            pxFirstFreeBlockInRegion->xBlockSize = xAddress - ( size_t ) pxFirstFreeBlockInRegion;
            pxFirstFreeBlockInRegion->pxNextFreeBlock = pxEnd;

            /* If this is not the first region that makes up the entire heap space
            then link the previous region to this region. */
            if( pxPreviousFreeBlock != NULL )
            {
                pxPreviousFreeBlock->pxNextFreeBlock = pxFirstFreeBlockInRegion;
            }

            xTotalHeapSize += pxFirstFreeBlockInRegion->xBlockSize;

            /* Move onto the next HeapRegion_t structure. */
            xDefinedRegions++;
        }
    }

    xMinimumEverFreeBytesRemaining = xTotalHeapSize;
    xFreeBytesRemaining = xTotalHeapSize;

    /* Work out the position of the top bit in a size_t variable. */
    xBlockAllocatedBit = ( ( size_t ) 1 ) << ( ( sizeof( size_t ) * heapBITS_PER_BYTE ) - 1 );
}

FE_INIT(freertos_global_init, 1)
#endif
