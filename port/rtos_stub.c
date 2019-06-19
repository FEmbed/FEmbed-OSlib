/**
 *	FreeRTOS need port function
 */ 
#include <assert.h>
#include <malloc.h>
#include "FreeRTOS.h"
#include "task.h"

#if CONFIG_RTOS_LIB_FREERTOS

// We only use static create task method.
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
//#include "driver.h"
//void freertos_global_init()
//{
//    vTaskStartScheduler();
//}
//
//FE_INIT(freertos_global_init)
#endif
