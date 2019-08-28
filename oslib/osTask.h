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


#ifndef __FE_FASTEMBEDDED_OS_TASK_H__
#define __FE_FASTEMBEDDED_OS_TASK_H__

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "WatchDog.h"

#define FE_OSTASK_FLAG_DMA_STACK                (1)

namespace FEmbed {
class OSTask;
class OSTaskPrivateData;

class OSTaskPrivateData {
public:
    class OSTask *m_task;
    TaskHandle_t  handle;                    ///< use to handle current os tid.
    QueueHandle_t m_lock;
    bool m_is_run;
};

/**
 * Common thread work.
 */
class OSTask {
public:
    OSTask(const char* name,
            unsigned int stack_size = 4096,
            unsigned int priority = configMAX_PRIORITIES - 1,
            unsigned int flags = 0
            );
    virtual ~OSTask();

    void start(shared_ptr<FEmbed::WatchDog> wd = nullptr, uint32_t mask = 0x1);
    void stop();
    void exit(int signal);
    bool isRun();
    uint32_t priority();
    char *name();
    virtual void feedDog();
    virtual void delay(uint32_t ms);
    virtual void loop();

    /**
     * static delay function for RTOS.
     * @param ms delay millisec for current thread.
     */
    static void osInit();
    static OSTask* currentTask();
    static uint32_t currentTick();
protected:
    void lock();
    void unlock();

    shared_ptr<FEmbed::WatchDog> m_wd;
    uint32_t m_wd_mask;
    int m_exit;

private:
    OSTaskPrivateData *d_ptr;
};

    // static global delay
    void osDelay(uint32_t ms);
}
#endif /* __FE_FASTEMBEDDED_TASK_H__ */
