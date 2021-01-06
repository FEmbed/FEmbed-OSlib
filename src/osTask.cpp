/* FastEmbedded Microcontroller Library
 * Copyright (c) 2018 Gene Kong
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#include <assert.h>
#include <malloc.h>
#include <string.h>

#include "osTask.h"

#define STATICTASK_SIZE ((sizeof(StaticTask_t) + 15)&(~0xf))

static void OSTask_runable_wrap(void *arg)
{
    FEmbed::OSTask *task = static_cast<FEmbed::OSTask *>(arg);
    if(task->isRun() == false)
        task->stop();
    task->loop();

    if(strcmp(task->name(), OSTAK_RUNONCE_NAME) == 0)
    {
        delete task;
    }
    else
    {
        log_w("Please delete it from other tasks.");   // task run-out of loop.
        task->delay(0xffffffff);                       // delay forever.
    }
}

namespace FEmbed {

/**
 * OSTask will allocate 3 continue memory.
 *
 * +------------------------+
 * |   Stack                |
 * +------------------------+
 * |   StaticTask_t         |
 * +------------------------+
 * |   OSTaskPrivateData    |
 * +------------------------+
 * @param name
 * @param stack_size
 * @param priority
 */
void FEmbedDeleteCallbackFunction(int idx, void * data)
{
    OSTaskPrivateData *ptr = (OSTaskPrivateData *)data;
    TaskHandle_t handle = ptr->handle;

    //Need deleted OSTask from current RTOS
    rtos_free_delayed(((StaticTask_t *)handle)->pxDummy6);
    rtos_free_delayed(handle);
}

OSTask::OSTask(
        const char* name,
        unsigned int stack_size,
        unsigned int priority,
        unsigned int flags
        ) :
                m_lock(new OSMutex())
{
#ifdef TRACE_MEM
    // Trace OSTask start memory map
    vPortMemInfoDetails();
#endif
    StaticTask_t *task_ptr;
    StackType_t *stack_ptr;

    assert((stack_size % sizeof(StackType_t)) == 0);
    taskENTER_CRITICAL();
#if USE_FEMBED
    m_wd_mask = 0;
    if(FE_OSTASK_FLAG_DMA_STACK & flags)
    {
        stack_ptr = (StackType_t *) DMA_MALLOC(stack_size);
        task_ptr = (StaticTask_t *) DMA_MALLOC(STATICTASK_SIZE + sizeof(OSTaskPrivateData));
    }
    else
#else
        (void) flags;
#endif
    {

        stack_ptr = (StackType_t *) malloc(stack_size);
        task_ptr = (StaticTask_t *) malloc(STATICTASK_SIZE + sizeof(OSTaskPrivateData));
    }
    this->d_ptr = (OSTaskPrivateData *)((uint8_t *)task_ptr + STATICTASK_SIZE);
    this->d_ptr->m_is_run = false;
    this->d_ptr->m_task = this;

    this->m_exit = 0;

    this->d_ptr->handle = xTaskCreateStatic(
            OSTask_runable_wrap,
            name,
            stack_size/sizeof(StackType_t),
            this,
            priority,
            (StackType_t * const)stack_ptr,
            (StaticTask_t * const)task_ptr);
    assert(this->d_ptr->handle == (TaskHandle_t)task_ptr);
    taskEXIT_CRITICAL();
    vTaskSetThreadLocalStoragePointerAndDelCallback(this->d_ptr->handle,
                                                    configNUM_THREAD_LOCAL_STORAGE_POINTERS - 1,
                                                    this->d_ptr,
                                                    FEmbedDeleteCallbackFunction);
}

OSTask::~OSTask() {
    OSTaskPrivateData *ptr = this->d_ptr;
    TaskHandle_t handle = ptr->handle;

    // Don't interrupt this free process, else may get memory error.
    taskENTER_CRITICAL();
    // If we release object in current OSTask, manual force to free it.
    if((xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) &&
        (xTaskGetCurrentTaskHandle() == this->d_ptr->handle))
    {
        free((void *)this);
    }
    else
    {
        this->stop();
    }
    taskEXIT_CRITICAL();
    m_lock.reset();
#if USE_FEMBED
    m_wd.reset();
#endif
    vTaskDelete(handle);
}

#if USE_FEMBED
void OSTask::start(std::shared_ptr<FEmbed::WatchDog> wd, uint32_t mask)
{
    m_wd = wd;
    m_wd_mask = mask;
    this->d_ptr->m_is_run = true;
    vTaskResume(this->d_ptr->handle);
}
#else
void OSTask::start()
{
    this->d_ptr->m_is_run = true;
    vTaskResume(this->d_ptr->handle);
}
#endif

void OSTask::runOnce(fe_task_runable runable, const char *name, void *arg,
                     uint32_t stack_size, uint32_t prio,
                     uint32_t flag)
{
    (new FEmbed::OSTask(name, stack_size, prio, flag))->setRunable(runable, arg)->start();
}

void OSTask::stop()
{
    this->d_ptr->m_is_run = false;
    vTaskSuspend(this->d_ptr->handle);
}

bool OSTask::isRun()
{
    return this->d_ptr->m_is_run;
}

uint32_t OSTask::priority()
{
    return uxTaskPriorityGet(this->d_ptr->handle);
}

OSTask *OSTask::setRunable(fe_task_runable runable, void *arg)
{
    if(this->d_ptr)
    {
        this->d_ptr->m_runable = runable;
        this->d_ptr->m_runable_arg = arg;
    }
    return this;
}

void OSTask::exit(int signal)
{
    (void) signal;
    this->m_exit = true;
}

char *OSTask::name()
{
    if(this->d_ptr->m_is_run)
    {
        return pcTaskGetName(this->d_ptr->handle);
    }

    return (char *)"?";
}

void OSTask::loop()
{
    //Must override this function for real do.
    if(this->d_ptr->m_runable)
        this->d_ptr->m_runable(this->d_ptr->m_runable_arg);
}

bool OSTask::feedDog()
{
#if USE_FEMBED
    if(m_wd)
        return m_wd->feedWatchDog(m_wd_mask);
#endif
    return true;
}

void OSTask::delay(uint32_t ms)
{
    osDelay(ms);
}

void OSTask::osInit()
{
    vTaskStartScheduler();
}

/**
 * Current OSTask handle will return.
 * Idle Task and Timer Task will case error!
 * @return
 */
extern "C" void *pvGetIdleTaskHandler();
extern "C" void *pvGetTimerTaskHandler();
OSTask* OSTask::currentTask()
{
    TaskHandle_t cur_handle = xTaskGetCurrentTaskHandle();
    if((pvGetTimerTaskHandler() == cur_handle) || (pvGetIdleTaskHandler() == cur_handle))
        return NULL;
    if(cur_handle)
        return ((OSTaskPrivateData *)((uint8_t *)cur_handle + STATICTASK_SIZE))->m_task;
    return NULL;
}

char *OSTask::currentTaskName()
{
    TaskHandle_t task_h = xTaskGetCurrentTaskHandle();
    if(task_h)
    {
        return pcTaskGetName(task_h);
    }
    return (char *)"???";
}

uint32_t OSTask::currentTick()
{
#if USE_FEMBED
    return fe_get_ticks();
#else
    if(FE_IS_IN_ISR())
        return xTaskGetTickCountFromISR();
    else
        return xTaskGetTickCount();
#endif
}

void osDelay(uint32_t ms)
{
#if USE_FEMBED   
    fe_delay(ms);
#else
    portTickType ticks = ms / portTICK_RATE_MS;
    vTaskDelay(ticks ? ticks : 1);
#endif
}

}

extern "C"
{
    void fe_os_thread_new(fe_task_runable runable, const char *name, void *arg,
                          uint32_t stack_size, uint32_t prio,
                          uint32_t flag)
    {
        FEmbed::OSTask::runOnce(runable, name, arg, stack_size, prio, flag);
    }
}
