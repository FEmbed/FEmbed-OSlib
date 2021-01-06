/*
 * fembed_os.h
 *
 * X-Cheng/XX Project Source
 *
 *  Created on: 2020
 *  Author: Gene Kong(gyx_edu@qq.com)
 */

#ifndef SRC_FEMBED_OS_H_
#define SRC_FEMBED_OS_H_

#ifdef __cplusplus
 extern "C" {
#endif /* __cplusplus */

#if USE_FEMBED
#define FE_OSTASK_FLAG_DMA_STACK                (1)
#include "driver.h"
#include "fe_target_ticks.h"
#else
#define FE_OSTASK_FLAG_DMA_STACK                (1)
#define DMA_MALLOC  malloc
#define DMA_FREE    free
#endif

#if defined(ESP_PLATFORM)
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#else
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "event_groups.h"
#endif

typedef void (*fe_task_runable)(void *arg);

void fe_os_thread_new(fe_task_runable runable, const char *name, void *arg,
                          uint32_t stack_size, uint32_t prio,
                          uint32_t flag);

#ifdef __cplusplus
 }
#endif /* __cplusplus */

#endif /* SRC_FE_OS_H_ */
