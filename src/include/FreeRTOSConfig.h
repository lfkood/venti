#ifndef VENTI_FREERTOS_CONFIG_H
#define VENTI_FREERTOS_CONFIG_H

// Start with scheduling on one core.
#define FREE_RTOS_KERNEL_SMP 1
#define configTASK_NOTIFICATION_ARRAY_ENTRIES 8

#include "FreeRTOSConfig_examples_common.h"
#include "SEGGER_SYSVIEW_FreeRTOS.h"

#if !USE_LEGACY_TRACE_API && TRACERETURN_ENABLE
#undef traceRETURN_vTaskEnterCriticalFromISR
#define traceRETURN_vTaskEnterCriticalFromISR(uxSavedInterruptStatus) \
    SEGGER_SYSVIEW_RecordEndCallU32(                                 \
        apiID_VTASKENTERCRITICALFROMISR, (U32)(uxSavedInterruptStatus))
#endif

#endif
