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

#ifndef __FE_FASTEMBEDDED_OS_MUTEX_H__
#define __FE_FASTEMBEDDED_OS_MUTEX_H__

#include <cstdint>
#include <cassert>

#include "FreeRTOS.h"
#include "semphr.h"
#include <memory>

namespace FEmbed {
class OSMutexPrivateData;

class OSMutex {
public:
	OSMutex();
	virtual ~OSMutex();

	/**
	 * all lock/tryLock/unlock operation can't work
	 * in interrupt routine.
	 */
	void lock();
	bool tryLock(uint32_t ms = 0);
	void unlock();

private:
	OSMutexPrivateData *d_ptr;
};

class OSMutexLocker {
 public:
    OSMutexLocker(OSMutex *lk) {
        _lk = lk;
        assert(lk);
        _lk->lock();
    }

    OSMutexLocker(OSMutex &lk) {
        _lk = &lk;
        _lk->lock();
    }

    OSMutexLocker(std::shared_ptr<OSMutex> &slk) {
        _lk = slk.get();
        assert(_lk);
        _lk->lock();
    }

    virtual ~OSMutexLocker() {
        _lk->unlock();
    }
    OSMutex *_lk;
};

} /* namespace FEmbed */

#define FE_NOTIFY_BOOL_METHOD(OBJ, NAME, LOCK) \
 private: \
    bool OBJ; \
 public: \
    void notify##NAME() \
    { \
        FEmbed::OSMutexLocker locker(LOCK); \
        OBJ = true; \
    } \
    bool is##NAME() \
    { \
        FEmbed::OSMutexLocker locker(LOCK); \
        bool ret = OBJ; \
        OBJ = false; \
        return ret; \
    }

#endif /* MUTEX_H_ */
