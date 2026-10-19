#include "freertos_init.h"


#include "FreeRTOS.h" // IWYU pragma: keep
#include "task.h"

#include "SEGGER_SYSVIEW.h"
#define VENTI_SYSVIEW_IMPLEMENTATION
#include "SEGGER_SYSVIEW_Conf.h"

#include "tachometer.h"
#include "misc.h"


void freeRTOS_init(void) {
  SEGGER_SYSVIEW_Conf();
  SEGGER_SYSVIEW_Start();
  
  if (xTaskCreate(blink_task, "blink", 512, NULL, tskIDLE_PRIORITY + 1, NULL) !=
      pdPASS) {
    panic("Failed to create blink task");
  }

  if (xTaskCreate(tach_monitor, "tach_monitor", 512, NULL, tskIDLE_PRIORITY + 1, NULL) !=
      pdPASS) {
    panic("Failed to create tachometer task");
  }
  
  vTaskStartScheduler();
}