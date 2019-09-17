/*
 * Mutex.cpp
 *
 *  Created on: 2018/11/13
 *      Author: Gene Kong
 */
#include <assert.h>

#include "osMutex.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#if USE_FEMBED
#include "driver.h"
#endif

namespace FEmbed {

class OSMutexPrivateData {
public:
	SemaphoreHandle_t m_mutex;
};

OSMutex::OSMutex() {
	this->d_ptr = new OSMutexPrivateData();
	assert(this->d_ptr);
	this->d_ptr->m_mutex = xSemaphoreCreateMutex();
	assert(this->d_ptr->m_mutex);
}

OSMutex::~OSMutex() {
	vQueueDelete(this->d_ptr->m_mutex);
	delete this->d_ptr;
}

void OSMutex::lock()
{
    BaseType_t TaskWoken = 0;
    if(xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
    {
        if(FE_IS_IN_ISR())
            xSemaphoreTakeFromISR(this->d_ptr->m_mutex, &TaskWoken);
        else
            xSemaphoreTake(this->d_ptr->m_mutex, portMAX_DELAY);
    }
}

bool OSMutex::tryLock(uint32_t ms)
{
    BaseType_t TaskWoken = 0;
    portTickType ticks;
    ticks = ms / portTICK_RATE_MS;
    if (ticks == 0) {
        ticks = 1;
    }

    if(xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
    {
        if(FE_IS_IN_ISR())
        {
            if(xSemaphoreTakeFromISR(this->d_ptr->m_mutex, &TaskWoken) != pdTRUE)
                return false;
        }
        else
        {
            if(xSemaphoreTake(this->d_ptr->m_mutex, ticks) != pdTRUE)
                return false;
        }
    }
    return true;
}

void OSMutex::unlock()
{
    BaseType_t TaskWoken = 0;
    if(xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
    {
        if(FE_IS_IN_ISR())
        {
            xSemaphoreGiveFromISR(this->d_ptr->m_mutex, &TaskWoken);
        }
        else
        {
            xSemaphoreGive(this->d_ptr->m_mutex);
        }
    }
}


} /* namespace FEmbed */
