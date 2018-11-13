/*
 * Mutex.cpp
 *
 *  Created on: 2018Äê11ÔÂ13ÈÕ
 *      Author: Gene Kong
 */
#include <assert.h>

#include "osMutex.h"

#include "FreeRTOS.h"
#include "semphr.h"

namespace fastembedded {

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
	xSemaphoreTake(this->d_ptr->m_mutex, portMAX_DELAY);
}

bool OSMutex::tryLock(uint32_t ms)
{
	  portTickType ticks;
	  ticks = ms / portTICK_RATE_MS;
	  if (ticks == 0) {
	    ticks = 1;
	  }

	if(xSemaphoreTake(this->d_ptr->m_mutex, ticks) != pdTRUE)
		return false;
	return true;
}

void OSMutex::unlock()
{
	xSemaphoreGive(this->d_ptr->m_mutex);
}


} /* namespace fastembedded */
