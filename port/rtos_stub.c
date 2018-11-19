/**
 *	FreeRTOS need port function
 */ 
#include <assert.h>
#include "FreeRTOS.h"
#include "task.h"

#if CONFIG_RTOS_LIB_FREERTOS
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
#endif