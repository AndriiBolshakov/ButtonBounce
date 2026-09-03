#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"

#define TEST_GPIO GPIO_NUM_4
#define DEBOUNCE_MS 50

static TaskHandle_t gpio_task_handle;
static int counter = 0;
static TickType_t last_edge_time = 0;

static void IRAM_ATTR gpio_isr_handler(void *arg)
{
    BaseType_t higher_priority_task_woken = pdFALSE;

    vTaskNotifyGiveFromISR(
        gpio_task_handle,
        &higher_priority_task_woken
    );

    if (higher_priority_task_woken) {
        portYIELD_FROM_ISR();
    }
}

static void gpio_task(void *arg)
{
    while (1) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        TickType_t now = xTaskGetTickCount();

        if ((now - last_edge_time) >= pdMS_TO_TICKS(DEBOUNCE_MS)) {
            last_edge_time = now;

            counter++;

            printf("FALLING EDGE #%d\n", counter);
        }
    }
}

void app_main(void)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << TEST_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .intr_type = GPIO_INTR_NEGEDGE,
    };

    ESP_ERROR_CHECK(gpio_config(&io_conf));

    xTaskCreate(
        gpio_task,
        "gpio_task",
        2048,
        NULL,
        10,
        &gpio_task_handle
    );

    ESP_ERROR_CHECK(gpio_install_isr_service(0));

    ESP_ERROR_CHECK(
        gpio_isr_handler_add(
            TEST_GPIO,
            gpio_isr_handler,
            NULL
        )
    );

    printf("GPIO4 falling-edge test ready.\n");
}