#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"

#include "driver/spi_master.h"
#include "driver/gpio.h"

#define LED_BLINK_CONTINIOUS (-1)
#define LED_BLINK_OFF (0)
#define LED_BLINK_ONE_TIME (1)
#define LED_BLINK_TEN_TIMES (10)

#define LED_COLOR_OFF (0)
#define LED_COLOR_WHITE (1)
#define LED_COLOR_GREEN (2)
#define LED_COLOR_BLUE (3)
#define LED_COLOR_PINK (4)
#define LED_COLOR_YELLOW (5)
#define LED_COLOR_ORANGE (6)

typedef enum
{
    RGB,
    RBG,
    GRB,
    GBR,
    BGR,
    BRG,
} WS2812_color_order_t;

typedef struct
{
    int led_spi_host;
    gpio_num_t led_pin_di;
    uint8_t led_count;
    WS2812_color_order_t color_order;
    uint8_t led_task_cpu;
    uint16_t blink_off_duration_ms;
    uint16_t blink_on_duration_ms;
} WS2812_settings_st;

void ws2812_config(WS2812_settings_st *settings);
void ws2812_init(WS2812_settings_st *settings);
void ws2812_set_led_blink_time(uint16_t off_period_ms, uint16_t on_period_ms);
void ws2812_set_led_color(uint8_t led_idx, uint8_t color_idx);
void ws2812_set_led_blinks(uint8_t led_idx, int8_t blinks);
void ws2812_force_led_state(uint8_t led_idx, uint8_t color_idx, int8_t blinks);
