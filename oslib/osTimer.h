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

#ifndef __FE_FASTEMBEDDED_OS_TIMER_H__
#define __FE_FASTEMBEDDED_OS_TIMER_H__

#include <stdint.h>
#include <assert.h>

#include "FreeRTOS.h"
#include "timers.h"

namespace fastembedded {

template<typename callback_t = TimerCallbackFunction_t>
class OSTimer {
public:
	template<typename callback_t>
	OSTimer(callback_t cb, bool reload, void* argument)
	{
		this->m_timer = xTimerCreate(
				"FE:Timer"
                1,
				reload? pdTRUE : pdFALSE,
                argument,
                cb);
		assert(this->m_timer);
	}

	virtual ~OSTimer()
	{
		xTimerDelete(this->m_timer, portMAX_DELAY);
	}

	bool Start(uint32_t ms)
	{
		portTickType ticks = ms / portTICK_RATE_MS;
		// TODO interrupt
		if (xTimerChangePeriod(this->m_timer, ticks, 0) != pdPASS)
			return false;
		else
		{
		if (xTimerStart(this->m_timer, 0) != pdPASS)
			return false;
		}
		return true;
	}

	bool stop()
	{
		// TODO interrupt
	    if (xTimerStop(this->m_timer, 0) != pdPASS) {
	      return false;
	    }
	    return true;
	}

private:
	TimerHandle_t m_timer;
};

} /* namespace fastembedded */

#endif /* MUTEX_H_ */
