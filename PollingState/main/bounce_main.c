#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"

#define TEST_GPIO GPIO_NUM_4
#define POLL_DELAY_MS 10

enum ButtonState {
    BUTTON_RELEASED,
    BUTTON_PRESSED
};

static int counter = 0;
static ButtonState button_state = BUTTON_RELEASED;

static void gpio_task(void *arg)
{
    while (1) {
        int state = gpio_get_level(TEST_GPIO);

        switch (button_state) {

            case BUTTON_RELEASED:
                if (state == 1) {
                    button_state = BUTTON_PRESSED;

                    counter++;

                    printf("PRESS #%d\n", counter);
                }
                break;

            case BUTTON_PRESSED:
                if (state == 0) {
                    button_state = BUTTON_RELEASED;
                }
                break;
        }

        vTaskDelay(pdMS_TO_TICKS(POLL_DELAY_MS));
    }
}

void app_main(void)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << TEST_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    ESP_ERROR_CHECK(gpio_config(&io_conf));

    xTaskCreate(
        gpio_task,
        "gpio_task",
        2048,
        NULL,
        10,
        NULL
    );

    printf("GPIO4 button polling ready.\n");
}