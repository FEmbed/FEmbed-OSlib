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

#ifndef __FE_FASTEMBEDDED_OS_RECURSIVEMUTEX_H__
#define __FE_FASTEMBEDDED_OS_RECURSIVEMUTEX_H__

#include <stdint.h>
#include <assert.h>

#include "FreeRTOS.h"
#include "semphr.h"

namespace FEmbed {

class OSRecursiveMutex {
public:
	OSRecursiveMutex()
	{
		this->m_mutex = xSemaphoreCreateRecursiveMutex();
		assert(this->m_mutex);
	}

	virtual ~OSRecursiveMutex()
	{
		vQueueDelete(this->m_mutex);
	}

	bool wait(uint32_t ms = 0xFFFFFFFF)
	{
		portTickType ticks;
		if(0xFFFFFFFF == ms)
		{
			ticks = portMAX_DELAY;
		}
		else
		{
			ticks = ms / portTICK_RATE_MS;
			if (ticks == 0) {
				ticks = 1;
			}
		}

		// TODO interrupt...
		if (xSemaphoreTakeRecursive(this->m_mutex, ticks) != pdTRUE) {
			return false;
		}
		return true;
	}

	bool release()
	{
		// TODO interrupt...
		if (xSemaphoreGiveRecursive(this->m_mutex) != pdTRUE) {
			return false;
		}
		return true;
	}

private:
	xSemaphoreHandle m_mutex;
};

} /* namespace FEmbed */

#endif /* MUTEX_H_ */
