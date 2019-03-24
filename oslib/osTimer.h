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

namespace FEmbed {
class OSTimer;

class OSTimerCallback {
public:
	virtual void expired(OSTimer *timer) = 0;
	virtual ~OSTimerCallback() {};
};

class OSTimer {
public:
	OSTimer(OSTimerCallback *cb, bool reload, void* argument);
	virtual ~OSTimer();

	bool start(uint32_t ms);
	bool stop();
	bool reset();

	void expired();

private:
	StaticTimer_t m_timer;
	OSTimerCallback *m_cb;
};

} /* namespace FEmbed */

#endif /* MUTEX_H_ */
