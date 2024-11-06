#include "ws2812.h"

#define DEFAULT_BLINK_OFF_TIME_MS (100)
#define DEFAULT_BLINK_ON_TIME_MS (500)

typedef struct
{
    uint8_t color;
    int8_t blinks;
    uint8_t pending_color;
} led_state_st;

static const char *TAG = "WS2812";

static spi_device_handle_t spi;

static led_state_st *led_state;
static uint8_t led_count;
static bool is_blink_off_period;
static WS2812_color_order_t color_order;
static bool need_to_update_leds;
static uint16_t blink_off_ms;
static uint16_t blink_on_ms;

static void get_color(uint8_t color_idx, float *first_byte, float *second_byte, float *third_byte);
static void update_leds();
static void ws2818_task(void *args);

static void get_color(uint8_t color_idx, float *first_byte, float *second_byte, float *third_byte)
{
    float r, g, b;
    switch (color_idx)
    {
    case LED_COLOR_GREEN:
        r = 0;
        g = 1;
        b = 0;
        break;
    case LED_COLOR_BLUE:
        r = 0;
        g = 0;
        b = 1;
        break;
    case LED_COLOR_PINK:
        r = 1;
        g = 0.1;
        b = 0.6;
        break;
    case LED_COLOR_YELLOW:
        r = 1;
        g = 1;
        b = 0;
        break;
    case LED_COLOR_ORANGE:
        r = 1;
        g = 0.5;
        b = 0;
        break;
    case LED_COLOR_WHITE:
        r = 1;
        g = 1;
        b = 1;
        break;
    case LED_COLOR_OFF:
    default:
        r = 0;
        g = 0;
        b = 0;
        break;
    }
    switch (color_order)
    {
    case RBG:
        *first_byte = r;
        *second_byte = b;
        *third_byte = g;
        break;
    case GRB:
        *first_byte = g;
        *second_byte = r;
        *third_byte = b;
        break;
    case GBR:
        *first_byte = g;
        *second_byte = b;
        *third_byte = r;
        break;
    case BGR:
        *first_byte = b;
        *second_byte = g;
        *third_byte = r;
        break;
    case BRG:
        *first_byte = b;
        *second_byte = r;
        *third_byte = g;
        break;
    case RGB:
    default:
        *first_byte = r;
        *second_byte = g;
        *third_byte = b;
        break;
    }
}

static void update_leds()
{
    uint8_t index = 0;
    uint8_t data_to_send[12 * led_count];
    for (size_t j = 0; j < led_count; j++)
    {
        float first_byte, second_byte, third_byte;
        uint8_t color = led_state[j].color;
        if (is_blink_off_period && led_state[j].blinks != LED_BLINK_OFF)
        {
            color = LED_COLOR_OFF;
        }
        get_color(color, &first_byte, &second_byte, &third_byte);
        uint8_t bytes[] = {(uint8_t)(first_byte * 255), (uint8_t)(second_byte * 255), (uint8_t)(third_byte * 255)};
        for (uint8_t i = 0; i < 3; i++)
        {
            for (uint8_t bit = 0; bit < 8; bit += 2)
            {
                if ((bytes[i] << bit) & 0x80)
                {
                    data_to_send[index] = 0xE0;
                }
                else
                {
                    data_to_send[index] = 0x80;
                }
                if ((bytes[i] << (bit + 1)) & 0x80)
                {
                    data_to_send[index] = data_to_send[index] | 0x0E;
                }
                else
                {
                    data_to_send[index] = data_to_send[index] | 0x08;
                }
                index++;
            }
        }
    }

    spi_transaction_t trans = {
        .length = 12 * led_count * 8,
        .tx_buffer = data_to_send,
        .rx_buffer = NULL,
        .user = (void *)0};

    spi_device_transmit(spi, &trans);
}

static void ws2818_task(void *args)
{
    while (true)
    {
        while (!need_to_update_leds)
        {
            vTaskDelay(pdMS_TO_TICKS(100));
        }
        bool need_to_blink = false;
        do
        {
            for (size_t i = 0; i < led_count; i++)
            {
                if (led_state[i].blinks != LED_BLINK_OFF)
                {
                    need_to_blink = true;
                    break;
                }
            }
            if (need_to_blink)
            {
                update_leds();
                if (is_blink_off_period)
                {
                    for (size_t i = 0; i < led_count; i++)
                    {
                        if (led_state[i].blinks == LED_BLINK_CONTINIOUS)
                        {
                            led_state[i].color = led_state[i].pending_color;
                        }
                        else if (led_state[i].blinks > LED_BLINK_OFF)
                        {
                            led_state[i].blinks = led_state[i].blinks - 1;
                            if (led_state[i].blinks == LED_BLINK_OFF)
                            {
                                led_state[i].color = led_state[i].pending_color;
                            }
                        }
                    }
                    vTaskDelay(pdMS_TO_TICKS(blink_off_ms));
                }
                else
                {
                    vTaskDelay(pdMS_TO_TICKS(blink_on_ms));
                }
                is_blink_off_period = !is_blink_off_period;
            }
        } while (need_to_blink);
        update_leds();
        need_to_update_leds = false;
    }
}

void ws2812_config(WS2812_settings_st *settings)
{
    settings->led_spi_host = SPI_HOST_MAX - 1;
    settings->led_pin_di = GPIO_NUM_0;
    settings->led_count = 1;
    settings->color_order = RGB;
    settings->led_task_cpu = 0;
    settings->blink_off_duration_ms = DEFAULT_BLINK_OFF_TIME_MS;
    settings->blink_on_duration_ms = DEFAULT_BLINK_ON_TIME_MS;
}

void ws2812_init(WS2812_settings_st *settings)
{
    need_to_update_leds = false;
    is_blink_off_period = false;
    color_order = settings->color_order;
    blink_off_ms = settings->blink_off_duration_ms;
    blink_on_ms = settings->blink_on_duration_ms;
    led_count = settings->led_count;
    led_state = (led_state_st *)malloc(sizeof(led_state_st) * led_count);
    for (size_t i = 0; i < led_count; i++)
    {
        led_state[i].color = LED_COLOR_OFF;
        led_state[i].blinks = LED_BLINK_OFF;
        led_state[i].pending_color = LED_COLOR_OFF;
    }

    spi_bus_config_t bus_cfg = {
        .mosi_io_num = settings->led_pin_di,
        .miso_io_num = -1,
        .sclk_io_num = -1,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1};

    spi_device_interface_config_t dev_cfg = {
        .clock_speed_hz = 3100000, // 3.1 MHz SPI for WS2812D
        .mode = 0,
        .spics_io_num = -1,
        .queue_size = 1};

    spi_bus_initialize(settings->led_spi_host, &bus_cfg, 0);
    spi_bus_add_device(settings->led_spi_host, &dev_cfg, &spi);

    xTaskCreatePinnedToCore(ws2818_task, "WS2812_TASK", 4096, NULL, 0, NULL, settings->led_task_cpu);
    ESP_LOGI(TAG, "WS2812 initialized");
}

void ws2812_set_led_blink_time(uint16_t off_period_ms, uint16_t on_period_ms)
{
    blink_off_ms = off_period_ms;
    blink_on_ms = on_period_ms;
}

void ws2812_set_led_color(uint8_t led_idx, uint8_t color_idx)
{
    if (led_state[led_idx].blinks <= LED_BLINK_OFF)
    {
        led_state[led_idx].color = color_idx;
    }
    led_state[led_idx].pending_color = color_idx;
    need_to_update_leds = true;
}

void ws2812_set_led_blinks(uint8_t led_idx, int8_t blinks)
{
    if (blinks == LED_BLINK_CONTINIOUS || led_state[led_idx].blinks <= LED_BLINK_OFF)
    {
        led_state[led_idx].blinks = blinks;
    }
    need_to_update_leds = true;
}

void ws2812_force_led_state(uint8_t led_idx, uint8_t color_idx, int8_t blinks)
{
    led_state[led_idx].pending_color = led_state[led_idx].color;
    led_state[led_idx].color = color_idx;
    led_state[led_idx].blinks = blinks;
    need_to_update_leds = true;
}