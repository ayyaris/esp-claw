/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <string.h>
#include "esp_log.h"
#include "driver/gpio.h"
#include "esp_rom_sys.h"
#include "esp_lcd_st7796.h"
#include "esp_lcd_touch_ft5x06.h"

static const char *TAG = "WT32_SC01_PLUS_SETUP_DEVICE";

/* Display and touch share the same reset line (GPIO4).
 * We perform the reset once here, before either device is initialized,
 * and disable software reset in both device configs. */
static bool s_shared_reset_done = false;

static void do_shared_reset(void)
{
    if (!s_shared_reset_done) {
        gpio_config_t cfg = {
            .mode = GPIO_MODE_OUTPUT,
            .pin_bit_mask = BIT64(4),
        };
        gpio_config(&cfg);
        gpio_set_level(4, 0);
        esp_rom_delay_us(20000);   // 20 ms low
        gpio_set_level(4, 1);
        esp_rom_delay_us(20000);   // 20 ms high
        s_shared_reset_done = true;
        ESP_LOGI(TAG, "Shared reset on GPIO4 done");
    }
}

esp_err_t lcd_panel_factory_entry_t(esp_lcd_panel_io_handle_t io, const esp_lcd_panel_dev_config_t *panel_dev_config, esp_lcd_panel_handle_t *ret_panel)
{
    do_shared_reset();
    esp_lcd_panel_dev_config_t panel_dev_cfg = {0};
    memcpy(&panel_dev_cfg, panel_dev_config, sizeof(esp_lcd_panel_dev_config_t));
    panel_dev_cfg.vendor_config = NULL;  // force I80/SPI path, not MIPI-DSI
    esp_err_t ret = esp_lcd_new_panel_st7796(io, &panel_dev_cfg, ret_panel);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "New st7796 panel failed");
    }
    return ret;
}

esp_err_t lcd_touch_factory_entry_t(esp_lcd_panel_io_handle_t io, const esp_lcd_touch_config_t *touch_dev_config, esp_lcd_touch_handle_t *ret_touch)
{
    do_shared_reset();
    esp_err_t ret = esp_lcd_touch_new_i2c_ft5x06(io, touch_dev_config, ret_touch);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create ft5x06 touch driver: %s", esp_err_to_name(ret));
    }
    return ret;
}
