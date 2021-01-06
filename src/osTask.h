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
#ifndef __FE_FASTEMBEDDED_OS_TASK_H__
#define __FE_FASTEMBEDDED_OS_TASK_H__

#include "fe_os.h"
#include "osMutex.h"

#define OSTAK_RUNONCE_NAME                      "__?Run?__"

#if USE_FEMBED
#include <FEmbed.h>

#define FE_OSTASK_FEED_CURR_DOG                 do { if(FEmbed::OSTask::currentTask()) FEmbed::OSTask::currentTask()->feedDog(); } while(0)
#else
#define FE_OSTASK_FEED_CURR_DOG                 
#endif

#define FE_OSTAK_ENTER_CRITICAL                 taskENTER_CRITICAL
#define FE_OSTAK_EXIT_CRITICAL                  taskEXIT_CRITICAL

namespace FEmbed {
class OSTask;
class OSTaskPrivateData;

class OSTaskPrivateData {
public:
    class OSTask *m_task;
    TaskHandle_t  handle;                    ///< use to handle current os tid.
    fe_task_runable m_runable;
    void *m_runable_arg;
    bool m_is_run;
};

/**
 * Common thread work.
 */
class OSTask {
public:
    OSTask(const char* name,
            unsigned int stack_size = 4096,
            unsigned int priority = configMAX_PRIORITIES/2,
            unsigned int flags = 0
            );
    virtual ~OSTask();

#if USE_FEMBED
    void start(std::shared_ptr<FEmbed::WatchDog> wd = nullptr, uint32_t mask = 0x1);
#else
    void start();
#endif
    void stop();
    void exit(int signal);
    OSTask *setRunable(fe_task_runable runable, void *arg = NULL);
    bool isRun();
    uint32_t priority();
    char *name();
    virtual bool feedDog();
    virtual void delay(uint32_t ms);
    virtual void loop();

    ////////////////////////////////////////////////////////////////////////////
    /// All of these static methods are not safe.
    /// Users must know when the current task is created by using OSTask
    ///     instead of using CMSIS or other methods.
    ////////////////////////////////////////////////////////////////////////////
    /**
     * Runonce object will auto re-cycle memory after run out.
     * @param runable run object.
     */
    static void runOnce(fe_task_runable runable, const char *name = OSTAK_RUNONCE_NAME, void *arg = NULL,
                        uint32_t stack_size = 2048, uint32_t prio = configMAX_PRIORITIES/2,
                        uint32_t flag = FE_OSTASK_FLAG_DMA_STACK);

    static void osInit();
    static char *currentTaskName();
    static OSTask* currentTask();
    static uint32_t currentTick();
protected:
    void lock() { m_lock->lock(); }
    void unlock() { m_lock->unlock(); }
    std::shared_ptr<OSMutex> m_lock;
#if USE_FEMBED
    std::shared_ptr<FEmbed::WatchDog> m_wd;
#endif
    uint32_t m_wd_mask;
    int m_exit;

private:
    OSTaskPrivateData *d_ptr;
};

    // static global delay
    void osDelay(uint32_t ms);

/**
 * Fast memory alloc and auto free.
 */
class OSMemoryAllocator {
 public:
    OSMemoryAllocator(size_t size, uint8_t mem_type = 0) {
        _mem = NULL;
        if(mem_type == 1)
            _mem = DMA_MALLOC(size);
        else
            _mem = malloc(size);
    }

    void *address() {
        return _mem;
    }

    virtual ~OSMemoryAllocator() {
        if(_mem) free(_mem);
    }
    void *_mem;
};

}

#define FE_OS_MEMBER_BOOL(OBJ, NAME) \
 private: \
    bool OBJ; \
 public: \
    void set##NAME(bool val) \
    { \
        FEmbed::OSMutexLocker locker(m_lock); \
        OBJ = val; \
    } \
    bool is##NAME() \
    { \
        FEmbed::OSMutexLocker locker(m_lock); \
        return OBJ; \
    }

#define FE_OS_MEMBER_TYPE(OBJ, NAME, TYPE) \
 private: \
    TYPE OBJ; \
 public: \
    void set##NAME(TYPE val) \
    { \
        FEmbed::OSMutexLocker locker(m_lock); \
        OBJ = val; \
    } \
    TYPE get##NAME() \
    { \
        FEmbed::OSMutexLocker locker(m_lock); \
        return OBJ; \
    }

#define FEMBED_OS_LOCKER     FEmbed::OSMutexLocker _locker(this->m_lock.get())

#endif /* __FE_FASTEMBEDDED_TASK_H__ */
