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
	vPortEnterCritical();
	ptr = malloc(xWantedSize);
	vPortExitCritical();
	return ptr;
}
void vPortFree(void *pv) {
	vPortEnterCritical();
	free(pv);
	vPortExitCritical();
}

void vApplicationMallocFailedHook( void )
{
    assert(0);
}

void vApplicationStackOverflowHook( TaskHandle_t xTask, char *pcTaskName )
{
    assert(0);
}

void vApplicationGetIdleTaskMemory(
        StaticTask_t **ppxIdleTaskTCBBuffer,
        StackType_t **ppxIdleTaskStackBuffer,
        uint32_t *pulIdleTaskStackSize )
{
    static __ccm StaticTask_t xIdleTaskTCB;
    static __ccm StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];

    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

void vApplicationGetTimerTaskMemory(
        StaticTask_t **ppxTimerTaskTCBBuffer,
        StackType_t **ppxTimerTaskStackBuffer,
        uint32_t *pulTimerTaskStackSize )
{
    static __ccm StaticTask_t xIdleTaskTCB;
    static __ccm StackType_t uxIdleTaskStack[ configTIMER_TASK_STACK_DEPTH ];

    *ppxTimerTaskTCBBuffer = &xIdleTaskTCB;
    *ppxTimerTaskStackBuffer = uxIdleTaskStack;
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

/******************************************************************************
 * freertos init function, will auto called by init function.
 ******************************************************************************/
#include "driver.h"
void freertos_global_init()
{
    vTaskStartScheduler();
}

FE_INIT(freertos_global_init)
#endif
