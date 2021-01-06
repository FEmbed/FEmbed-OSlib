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

#ifndef __FE_FASTEMBEDDED_SEMAPHORE_H__
#define __FE_FASTEMBEDDED_SEMAPHORE_H__

#include <stdint.h>
#include <assert.h>

#include "fe_os.h"

namespace FEmbed {

class OSSemaphore {
public:
	OSSemaphore(int32_t count = 1, int32_t init = -1)
	{
		if(count == 1)
		{
			vSemaphoreCreateBinary(this->m_sem);
		}
		else
		{
			this->m_sem = xSemaphoreCreateCounting(count, init<0?count:init);
		}
		assert(this->m_sem);
	}

	virtual ~OSSemaphore()
	{
		vSemaphoreDelete(this->m_sem);
	}

	bool wait(uint32_t ms = 0xFFFFFFFF)
	{
		portTickType ticks;
	    BaseType_t TaskWoken = 0;
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

		if(FE_IS_IN_ISR())
		{
            if (xSemaphoreTakeFromISR(this->m_sem, &TaskWoken) != pdTRUE) return false;
		}
		else
		{
		    if (xSemaphoreTake(this->m_sem, ticks) != pdTRUE) return false;
		}
		return true;
	}

	bool release()
	{
        BaseType_t TaskWoken = 0;
        if(FE_IS_IN_ISR())
        {
            if (xSemaphoreGiveFromISR(this->m_sem, &TaskWoken) != pdTRUE)
                return false;
        }
        else
        {
            if (xSemaphoreGive(this->m_sem) != pdTRUE)
                return false;
        }
		return true;
	}
private:
	SemaphoreHandle_t m_sem;
};

} /* namespace FEmbed */

#endif /* Semaphore_H_ */
