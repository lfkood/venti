#include <stdio.h>

#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"

static void blink_task(void *parameter)
{
    (void)parameter;

    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    puts("FreeRTOS blink task started");

    for (;;) {
        gpio_put(PICO_DEFAULT_LED_PIN, 1);
        puts("LED on");
        vTaskDelay(pdMS_TO_TICKS(500));

        gpio_put(PICO_DEFAULT_LED_PIN, 0);
        puts("LED off");
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

int main(void)
{
    stdio_init_all();
    puts("venti: RTT ready; starting FreeRTOS");

    if (xTaskCreate(blink_task, "blink", 512, NULL,
                    tskIDLE_PRIORITY + 1, NULL) != pdPASS) {
        panic("Failed to create blink task");
    }

    vTaskStartScheduler();

    // Reached if the scheduler cannot allocate its startup tasks.
    panic("FreeRTOS scheduler failed to start");
}
