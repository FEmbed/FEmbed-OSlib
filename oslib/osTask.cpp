/*
 * OSTask.cpp
 *
 *  Created on: 2018��10��29��
 *      Author: Gene Kong
 */
#include <assert.h>
#include <malloc.h>
#include <string.h>

#include "OSTask.h"

static void OSTask_runable_wrap(void *arg)
{
	fastembedded::OSTask *OSTask = static_cast<fastembedded::OSTask *>(arg);
	if(OSTask->isRun() == false)
		OSTask->stop();
	OSTask->runable();
}

namespace fastembedded {

OSTask::OSTask(
		const char* name,
		unsigned int stack_size,
		unsigned int priority
		) {
#ifdef TRACE_MEM
	// Trace OSTask start memory map
	vPortMemInfoDetails();
#endif

	assert((stack_size % sizeof(StackType_t)) == 0);
	int len = sizeof(StaticTask_t) +
			stack_size +
			sizeof(OSTaskPrivateData);
	uint8_t *buf = (uint8_t *)malloc(len);

	assert(buf);
	memset(buf, 0, len);
	this->d_ptr = (OSTaskPrivateData *)(buf + sizeof(StaticTask_t) + stack_size);
	this->d_ptr->m_is_run = false;
	this->m_exit = 0;

	this->d_ptr->handle = xTaskCreateStatic(
			OSTask_runable_wrap,
			name,
			stack_size/sizeof(StackType_t),
			this,
			priority,
			(StackType_t * const)(buf + sizeof(StaticTask_t)),
			(StaticTask_t * const)buf);
	assert(this->d_ptr->handle);

	this->d_ptr->m_lock = xSemaphoreCreateMutex();
	assert(this->d_ptr->m_lock);
}

OSTask::~OSTask() {
	OSTaskPrivateData *ptr = this->d_ptr;
	TaskHandle_t handle = ptr->handle;
	//Need deleted OSTask from current RTOS
	vSemaphoreDelete(this->d_ptr->m_lock);

	// Don't interrupt this free process, else may get memory error.
	taskENTER_CRITICAL();
	// If we release object in current OSTask, manual force to free it.
	if(xTaskGetCurrentTaskHandle() == this->d_ptr->handle)
	{
		free((void *)this);
	}
	free(ptr);
	vTaskDelete(handle);
	taskEXIT_CRITICAL();
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
	this->m_exit = true;
}

void OSTask::runable()
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
	portTickType ticks = ms / portTICK_RATE_MS;
	vTaskDelay(ticks ? ticks : 1);
}

}
