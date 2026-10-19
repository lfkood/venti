#include "freertos_init.h"

void freeRTOS_init(void) {
  SEGGER_SYSVIEW_Conf();
  SEGGER_SYSVIEW_Start();
  
  if (xTaskCreate(blink_task, "blink", 512, NULL, tskIDLE_PRIORITY + 1, NULL) !=
      pdPASS) {
    panic("Failed to create blink task");
  }
  
  vTaskStartScheduler();
}