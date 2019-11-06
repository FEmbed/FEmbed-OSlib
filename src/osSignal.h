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

#ifndef __FE_FASTEMBEDDED_OS_SIGNAL_H__
#define __FE_FASTEMBEDDED_OS_SIGNAL_H__

#include <stdint.h>
#include <assert.h>

#include "FreeRTOS.h"
#include "semphr.h"
#include "event_groups.h"

namespace FEmbed {

class OSSignal {
public:
	OSSignal()
	{
		m_evt = xEventGroupCreate();
		assert(this->m_evt);
	}

	virtual ~OSSignal()
	{
		vEventGroupDelete(this->m_evt);
	}

	uint32_t set(uint32_t bits)
	{
		// TODO Interrupt.
		return xEventGroupSetBits(this->m_evt, bits);
	}

	uint32_t wait(uint32_t bits, uint32_t ms = 0xFFFFFFFF)
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

		// TODO Interrupt.
		return xEventGroupWaitBits(this->m_evt,
				bits,
				pdTRUE,
				pdFALSE,
				ticks);
	}

	uint32_t clear(uint32_t bits)
	{
		// TODO Interrupt.
		return xEventGroupClearBits(this->m_evt, bits);
	}
private:
	EventGroupHandle_t m_evt;
};

} /* namespace FEmbed */

#define FE_NOTIFY_SIGNAL(POS, NAME, SIGNAL) \
 public: \
    inline void notify##NAME() \
    { \
         SIGNAL->set(1<<(POS)); \
    } \
    inline bool is##NAME(uint32_t chk) \
    { \
        if(chk & (1<<(POS))) \
            return true; \
        return false; \
    }

#endif /* __FE_FASTEMBEDDED_OS_SIGNAL_H__ */
