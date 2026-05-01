# Patch: I80 LCD Support for ESP Board Manager

The `espressif__esp_board_manager` component does not natively support `sub_type: i80` for parallel LCD displays on ESP32-S3. The following files must be patched to enable Intel 8080 (I80) bus support.

## Files Modified

### 1. `managed_components/espressif__esp_board_manager/devices/dev_display_lcd/dev_display_lcd.h`

Add the include, define, struct and union member for I80:

```c
/* after the PARLIO include block */
#if CONFIG_ESP_BOARD_DEV_DISPLAY_LCD_SUB_I80_SUPPORT
#include "esp_lcd_io_i80.h"
#endif /* CONFIG_ESP_BOARD_DEV_DISPLAY_LCD_SUB_I80_SUPPORT */
```

```c
/* after ESP_BOARD_DEVICE_LCD_SUB_TYPE_PARLIO */
#define ESP_BOARD_DEVICE_LCD_SUB_TYPE_I80     "i80"     /*!< LCD display over I80 (esp_lcd_io_i80) */
```

```c
/* after dev_display_lcd_parlio_sub_config_t */
#if CONFIG_ESP_BOARD_DEV_DISPLAY_LCD_SUB_I80_SUPPORT
typedef struct {
    esp_lcd_i80_bus_config_t      bus_i80;       /*!< I80 bus configuration */
    esp_lcd_panel_io_i80_config_t io_i80;        /*!< I80 panel IO configuration */
    esp_lcd_panel_dev_config_t    panel_config;  /*!< LCD panel device configuration */
} dev_display_lcd_i80_sub_config_t;
#endif /* CONFIG_ESP_BOARD_DEV_DISPLAY_LCD_SUB_I80_SUPPORT */
```

```c
/* inside the union, after parlio */
#if CONFIG_ESP_BOARD_DEV_DISPLAY_LCD_SUB_I80_SUPPORT
        dev_display_lcd_i80_sub_config_t     i80;
#endif /* CONFIG_ESP_BOARD_DEV_DISPLAY_LCD_SUB_I80_SUPPORT */
```

### 2. `managed_components/espressif__esp_board_manager/devices/dev_display_lcd/dev_display_lcd.py`

Add the parser function **before** `parse_parlio_sub_config`:

```python
def parse_i80_sub_config(full_config: dict = None, peripherals_dict=None) -> dict:
    """Parse I80 (esp_lcd_io_i80) sub configuration."""
    sub_config = full_config.get('config', {})
    x_max = int(sub_config.get('x_max', 320))
    bus = sub_config.get('bus_i80_config', {})
    io = sub_config.get('io_i80_config', {})

    data_nums = bus.get('data_gpio_nums')
    if data_nums is not None:
        data_nums = [int(x) for x in data_nums]
    else:
        data_nums = [int(bus.get(f'data_gpio_{i}', -1)) for i in range(16)]

    while len(data_nums) < 16:
        data_nums.append(-1)
    data_nums = data_nums[:16]

    default_max_transfer = x_max * 20 * 2
    max_transfer = int(bus.get('max_transfer_bytes', default_max_transfer))
    if max_transfer <= 0:
        max_transfer = default_max_transfer

    bus_i80_parsed = {
        'dc_gpio_num': int(bus.get('dc_gpio_num', -1)),
        'wr_gpio_num': int(bus.get('wr_gpio_num', -1)),
        'data_gpio_nums': data_nums,
        'bus_width': int(bus.get('bus_width', 8)),
        'max_transfer_bytes': max_transfer,
        'clk_src': bus.get('clk_src', 'LCD_CLK_SRC_DEFAULT'),
        'dma_burst_size': int(bus.get('dma_burst_size', 0)),
        'flags': {
            'swap_color_bytes': bus.get('flags', {}).get('swap_color_bytes', False),
            'pclk_active_neg': bus.get('flags', {}).get('pclk_active_neg', False),
            'reverse_color_bits': bus.get('flags', {}).get('reverse_color_bits', False),
        },
    }

    io_i80_parsed = {
        'cs_gpio_num': int(io.get('cs_gpio_num', -1)),
        'pclk_hz': int(io.get('pclk_hz', 16000000)),
        'trans_queue_depth': int(io.get('trans_queue_depth', 10)),
        'dc_levels': {
            'dc_cmd_level': int(io.get('dc_levels', {}).get('dc_cmd_level', 0)),
            'dc_data_level': int(io.get('dc_levels', {}).get('dc_data_level', 1)),
            'dc_idle_level': int(io.get('dc_levels', {}).get('dc_idle_level', 0)),
            'dc_dummy_level': int(io.get('dc_levels', {}).get('dc_dummy_level', 0)),
        },
        'lcd_cmd_bits': int(io.get('lcd_cmd_bits', 8)),
        'lcd_param_bits': int(io.get('lcd_param_bits', 8)),
        'flags': {
            'cs_active_high': io.get('flags', {}).get('cs_active_high', False),
            'reverse_color_bits': io.get('flags', {}).get('reverse_color_bits', False),
            'swap_color_bytes': io.get('flags', {}).get('swap_color_bytes', False),
            'pclk_active_neg': io.get('flags', {}).get('pclk_active_neg', False),
        },
    }

    return {
        'bus_i80': bus_i80_parsed,
        'io_i80': io_i80_parsed,
        'panel_config': _parse_lcd_panel_dev_config_dict(sub_config),
    }
```

Change the `sub_type` validation list:
```python
if sub_type not in ['dsi', 'spi', 'parlio', 'i80']:
```

Add the `i80` branch in `parse()` **before** the final `else:`:

```python
    elif sub_type == 'i80':
        sub_cfg = parse_i80_sub_config(full_config, peripherals_dict)
        sub_cfg_union = {'i80': sub_cfg}
        lcd_width = full_config.get('config').get('x_max', 320)
        lcd_height = full_config.get('config').get('y_max', 240)
        swap_xy = full_config.get('config').get('swap_xy', False)
        mirror_x = full_config.get('config').get('mirror_x', False)
        mirror_y = full_config.get('config').get('mirror_y', False)
        need_reset = full_config.get('config').get('need_reset', True)
        invert_color = full_config.get('config').get('invert_color', False)
        rgb_ele_order = sub_cfg.get('panel_config', {}).get('rgb_ele_order', 'LCD_RGB_ELEMENT_ORDER_RGB')
        data_endian = sub_cfg.get('panel_config', {}).get('data_endian', 'LCD_RGB_DATA_ENDIAN_BIG')
        bits_per_pixel = sub_cfg.get('panel_config', {}).get('bits_per_pixel', 16)
```

### 3. `managed_components/espressif__esp_board_manager/devices/CMakeLists.txt`

Add inside the `CONFIG_ESP_BOARD_DEV_DISPLAY_LCD_SUPPORT` block:

```cmake
        if(CONFIG_ESP_BOARD_DEV_DISPLAY_LCD_SUB_I80_SUPPORT)
            list(APPEND srcs "${CMAKE_CURRENT_LIST_DIR}/devices/dev_display_lcd/dev_display_lcd_sub_i80.c")
        endif()
```

### 4. `managed_components/espressif__esp_board_manager/gen_codes/Kconfig.in`

Add inside the `if ESP_BOARD_DEV_DISPLAY_LCD_SUPPORT` block:

```kconfig
    config ESP_BOARD_DEV_DISPLAY_LCD_SUB_I80_SUPPORT
        bool "Device 'display_lcd' i80 sub-type support"
        help
            Enable display_lcd i80 sub-type support.
            This option enables the display_lcd i80 sub-type driver.
```

## New File

Create `managed_components/espressif__esp_board_manager/devices/dev_display_lcd/dev_display_lcd_sub_i80.c` with the content from:
`boards/esp32s3_wt32_sc01_plus/setup_device.c` references `esp_lcd_new_panel_st7796` via `lcd_panel_factory_entry_t`.

The full content of the new C file is available in the working tree at:
`managed_components/espressif__esp_board_manager/devices/dev_display_lcd/dev_display_lcd_sub_i80.c`

## Application

After patching, run:

```bash
idf.py fullclean
idf.py build
```
