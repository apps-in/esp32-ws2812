# WS2812

ESP32 driver for WS2812 LED

## Installation

``
idf.py add-dependency "inapps/ws2812^1.0.0"
``

## How to use

### Initialization

```
WS2812_settings_st led_settings;
ws2812_config(&led_settings);
led_settings.led_spi_host = LED_SPI;            // SPI Host
led_settings.led_pin_di = LED_PIN;              // DI Pin
led_settings.led_count = LED_COUNT;             // Number of serially connected leds
led_settings.led_task_cpu = 0;                  // CPU to run LED task
led_settings.color_order = RGB;                 // Color order
led_settings.blink_off_duration_ms = 100;       // Blink OFF period duration (ms)
led_settings.blink_on_duration_ms = 500;        // Blink ON period duration (ms)
ws2812_init(&led_settings);
```

### Usage

**Override blink periods duration**
```
void ws2812_set_led_blink_time(uint16_t off_period_ms, uint16_t on_period_ms);
```

**Set static color**
```
void ws2812_set_led_color(uint8_t led_idx, uint8_t color_idx);
```
_color_idx - one of the indexed colors_
```
#define LED_COLOR_OFF (0)
#define LED_COLOR_WHITE (1)
#define LED_COLOR_GREEN (2)
#define LED_COLOR_BLUE (3)
#define LED_COLOR_PINK (4)
#define LED_COLOR_YELLOW (5)
#define LED_COLOR_ORANGE (6)
```

**Set blinking**
```
void ws2812_set_led_blinks(uint8_t led_idx, int8_t blinks);
```
_blinks - number of blinks (-1 for continious blinking, 0 for no blinking)_

**Immideatly change led state**
```
void ws2812_force_led_state(uint8_t led_idx, uint8_t color_idx, int8_t blinks);
```