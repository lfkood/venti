#include "misc.h"

static void blink_task(void *parameter) {
  (void)parameter;

  gpio_init(PICO_DEFAULT_LED_PIN);
  gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
  stdio_puts("FreeRTOS blink task started");

  for (;;) {
    gpio_put(PICO_DEFAULT_LED_PIN, 1);
    stdio_puts("LED on");
    vTaskDelay(pdMS_TO_TICKS(500));

    gpio_put(PICO_DEFAULT_LED_PIN, 0);
    stdio_puts("LED off");
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}