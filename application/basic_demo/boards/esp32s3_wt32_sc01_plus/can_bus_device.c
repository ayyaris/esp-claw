/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "esp_log.h"
#include "esp_check.h"
#include "driver/twai.h"
#include "esp_board_entry.h"

static const char *TAG = "CAN_BUS_DEVICE";

#include "gen_board_device_custom.h"

static bool s_twai_installed = false;

int can_bus_init(void *cfg, int cfg_size, void **device_handle)
{
    dev_custom_can_bus_config_t *can_cfg = (dev_custom_can_bus_config_t *)cfg;
    if (can_cfg == NULL) {
        ESP_LOGE(TAG, "CAN bus config is NULL");
        return -1;
    }

    ESP_LOGI(TAG, "Initializing CAN bus: tx=%d rx=%d bitrate=%d mode=%s",
             can_cfg->tx_gpio, can_cfg->rx_gpio, can_cfg->bitrate,
             can_cfg->mode ? can_cfg->mode : "normal");

    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(
        can_cfg->tx_gpio, can_cfg->rx_gpio, TWAI_MODE_NORMAL);

    /* Adjust mode if requested */
    if (can_cfg->mode && strcmp(can_cfg->mode, "listen_only") == 0) {
        g_config.mode = TWAI_MODE_LISTEN_ONLY;
    }

    twai_timing_config_t t_config;
    if (can_cfg->bitrate == 125000) {
        twai_timing_config_t temp = TWAI_TIMING_CONFIG_125KBITS();
        t_config = temp;
    } else if (can_cfg->bitrate == 250000) {
        twai_timing_config_t temp = TWAI_TIMING_CONFIG_250KBITS();
        t_config = temp;
    } else if (can_cfg->bitrate == 500000) {
        twai_timing_config_t temp = TWAI_TIMING_CONFIG_500KBITS();
        t_config = temp;
    } else if (can_cfg->bitrate == 1000000) {
        twai_timing_config_t temp = TWAI_TIMING_CONFIG_1MBITS();
        t_config = temp;
    } else {
        ESP_LOGW(TAG, "Unsupported bitrate %d, falling back to 500Kbps", can_cfg->bitrate);
        twai_timing_config_t temp = TWAI_TIMING_CONFIG_500KBITS();
        t_config = temp;
    }

    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    esp_err_t ret = twai_driver_install(&g_config, &t_config, &f_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to install TWAI driver: %s", esp_err_to_name(ret));
        return -1;
    }

    ret = twai_start();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start TWAI: %s", esp_err_to_name(ret));
        twai_driver_uninstall();
        return -1;
    }

    s_twai_installed = true;
    ESP_LOGI(TAG, "CAN bus initialized successfully");

    /* No dynamic handle needed; TWAI is a singleton */
    if (device_handle) {
        *device_handle = (void *)1;
    }
    return 0;
}

int can_bus_deinit(void *device_handle)
{
    (void)device_handle;

    if (s_twai_installed) {
        twai_stop();
        twai_driver_uninstall();
        s_twai_installed = false;
        ESP_LOGI(TAG, "CAN bus deinitialized");
    }
    return 0;
}

ESP_BOARD_ENTRY_IMPLEMENT(can_bus, can_bus_init, can_bus_deinit);
