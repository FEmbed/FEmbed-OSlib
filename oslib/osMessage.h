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

#ifndef __FE_FASTEMBEDDED_OS_MESSAGE_H__
#define __FE_FASTEMBEDDED_OS_MESSAGE_H__

#include <stdint.h>
#include <assert.h>

#include "FreeRTOS.h"
#include "semphr.h"

namespace FEmbed {
class OSMessagePrivateData {
public:
	xQueueHandle m_msg;
};

template<typename T = uint32_t, int len = 16>
class OSMessage {
public:

	OSMessage() {
		this->d_ptr = new OSMessagePrivateData();
		assert(this->d_ptr);
		this->d_ptr->m_msg = xQueueCreate(len, sizeof(T));
		assert(this->d_ptr->m_msg);
	}

	virtual ~OSMessage()
	{
		vQueueDelete(this->d_ptr->m_msg);
		delete this->d_ptr;
	}

	void put(T& item)
	{
		// if(interrupt)
		// {
		// }
		// else
		xQueueSend(this->d_ptr->m_msg, &item, portMAX_DELAY);
	}

	bool tryPut(T& item, uint32_t ms = 0)
	{
		portTickType ticks;
		ticks = ms / portTICK_RATE_MS;
		if (ticks == 0) {
			ticks = 1;
		}

		// if(interrupt)
		// {
		// }
		// else
		if (xQueueSend(this->d_ptr->m_msg, &item, ticks) != pdTRUE) {
			return false;
		}
		return true;
	}

	void get(T *item)
	{
		// if(interrupt)
		// {
		// }
		// else
		xQueueReceive(this->d_ptr->m_msg, item, portMAX_DELAY);
	}

	bool tryGet(T *item, uint32_t ms = 0)
	{
		portTickType ticks;
		ticks = ms / portTICK_RATE_MS;
		if (ticks == 0) {
			ticks = 1;
		}

		// if(interrupt)
		// {
		// }
		// else
		if (xQueueSend(this->d_ptr->m_msg, item, ticks) != pdTRUE) {
			return false;
		}
		return true;
	}
private:
	OSMessagePrivateData *d_ptr;
};

} /* namespace FEmbed */

#endif /* MUTEX_H_ */
