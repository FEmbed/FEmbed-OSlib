/* FastEmbedded Microcontroller Library
 * Copyright (c) 2018-2028 Gene Kong
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
#include "driver.h"

#define STATICTASK_SIZE ((sizeof(StaticTask_t) + 15)&(~0xf))

static void OSTask_runable_wrap(void *arg)
{
	FEmbed::OSTask *task = static_cast<FEmbed::OSTask *>(arg);
	if(task->isRun() == false)
		task->stop();
	task->loop();
	// Auto free task information.
	delete task;
}

namespace FEmbed {

/**
 * OSTask will allocate 3 continue memory.
 * +------------------------+
 * |   StaticTask_t         |
 * +------------------------+
 * |   Stack                |
 * +------------------------+
 * |   OSTaskPrivateData    |
 * +------------------------+
 * @param name
 * @param stack_size
 * @param priority
 */
OSTask::OSTask(
		const char* name,
		unsigned int stack_size,
		unsigned int priority
		) {
#ifdef TRACE_MEM
	// Trace OSTask start memory map
	vPortMemInfoDetails();
#endif
	StaticTask_t *task_ptr;
	StackType_t *stack_ptr;

	assert((stack_size % sizeof(StackType_t)) == 0);
	vPortEnterCritical();
	stack_ptr = (StackType_t *) malloc(stack_size);

	task_ptr = (StaticTask_t *) malloc(STATICTASK_SIZE + sizeof(OSTaskPrivateData));
	this->d_ptr = (OSTaskPrivateData *)((uint8_t *)task_ptr + STATICTASK_SIZE);

	this->d_ptr->m_is_run = false;
	this->d_ptr->m_task = this;

	this->m_exit = 0;

    this->d_ptr->m_lock = xSemaphoreCreateMutex();
    assert(this->d_ptr->m_lock);

	this->d_ptr->handle = xTaskCreateStatic(
			OSTask_runable_wrap,
			name,
			stack_size/sizeof(StackType_t),
			this,
			priority,
			(StackType_t * const)stack_ptr,
			(StaticTask_t * const)task_ptr);
	assert(this->d_ptr->handle == task_ptr);
	vPortExitCritical();
}

OSTask::~OSTask() {
	OSTaskPrivateData *ptr = this->d_ptr;
	TaskHandle_t handle = ptr->handle;
	//Need deleted OSTask from current RTOS
	vSemaphoreDelete(this->d_ptr->m_lock);

	// Don't interrupt this free process, else may get memory error.
	taskENTER_CRITICAL();
	// If we release object in current OSTask, manual force to free it.
	if((xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) &&
		(xTaskGetCurrentTaskHandle() == this->d_ptr->handle))
	{
		free((void *)this);
	}
	taskEXIT_CRITICAL();
    vTaskDelete(handle);
}

void OSTask::start()
{
	this->d_ptr->m_is_run = true;
	vTaskResume(this->d_ptr->handle);
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
	delete this;
}

void OSTask::lock()
{
	xSemaphoreTake(this->d_ptr->m_lock, portMAX_DELAY);
}

void OSTask::unlock()
{
	xSemaphoreGive(this->d_ptr->m_lock);
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
OSTask* OSTask::currentTask()
{
	TaskHandle_t cur_handle = xTaskGetCurrentTaskHandle();
	if(cur_handle)
		return ((OSTaskPrivateData *)((uint8_t *)cur_handle + STATICTASK_SIZE))->m_task;
	return NULL;
}

int OSTask::currentTick()
{
	if(FE_IS_IN_ISR())
		return xTaskGetTickCountFromISR();
	else
		return xTaskGetTickCount();

}

void osDelay(uint32_t ms)
{
	portTickType ticks = ms / portTICK_RATE_MS;
	vTaskDelay(ticks ? ticks : 1);
}

}
