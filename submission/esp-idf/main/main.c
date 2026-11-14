#include <stdbool.h>
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_err.h"
#include "esp_log.h"

// ESP32 ADC1_CHANNEL_6 คือ GPIO34 (บนบอร์ดเขียน GPI34)
#define LDR_ADC_UNIT       ADC_UNIT_1
#define LDR_ADC_CHANNEL    ADC_CHANNEL_6
#define LDR_ADC_ATTEN      ADC_ATTEN_DB_12
#define SAMPLE_COUNT       16
#define SAMPLE_INTERVAL_MS 500

// เปลี่ยนเป็น 1 หลังคัดลอกสัมประสิทธิ์จาก calibration.xlsx
#define CALIBRATION_READY  0
#define USE_QUADRATIC_MODEL 0

static const float LINEAR_M = 0.0f;
static const float LINEAR_B = 0.0f;
static const float POLY_A = 0.0f;
static const float POLY_B = 0.0f;
static const float POLY_C = 0.0f;

static const char *TAG = "LDR";

static float adc_to_lux(int raw)
{
#if USE_QUADRATIC_MODEL
    return POLY_A * raw * raw + POLY_B * raw + POLY_C;
#else
    return LINEAR_M * raw + LINEAR_B;
#endif
}

static bool init_adc_calibration(adc_cali_handle_t *handle)
{
    esp_err_t result = ESP_FAIL;

#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    adc_cali_curve_fitting_config_t config = {
        .unit_id = LDR_ADC_UNIT,
        .chan = LDR_ADC_CHANNEL,
        .atten = LDR_ADC_ATTEN,
        .bitwidth = ADC_BITWIDTH_12,
    };
    result = adc_cali_create_scheme_curve_fitting(&config, handle);
#elif ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    adc_cali_line_fitting_config_t config = {
        .unit_id = LDR_ADC_UNIT,
        .atten = LDR_ADC_ATTEN,
        .bitwidth = ADC_BITWIDTH_12,
        .default_vref = 1100,
    };
    result = adc_cali_create_scheme_line_fitting(&config, handle);
#endif

    return result == ESP_OK;
}

void app_main(void)
{
    adc_oneshot_unit_handle_t adc_handle;
    const adc_oneshot_unit_init_cfg_t unit_config = {
        .unit_id = LDR_ADC_UNIT,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&unit_config, &adc_handle));

    const adc_oneshot_chan_cfg_t channel_config = {
        .atten = LDR_ADC_ATTEN,
        .bitwidth = ADC_BITWIDTH_12,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(
        adc_handle, LDR_ADC_CHANNEL, &channel_config));

    adc_cali_handle_t calibration_handle = NULL;
    const bool calibrated = init_adc_calibration(&calibration_handle);

    ESP_LOGI(TAG, "Connect LDR signal to GPI34");
    ESP_LOGI(TAG, "time_ms,adc,voltage_mv,lux");

    while (true) {
        int total = 0;
        for (int i = 0; i < SAMPLE_COUNT; i++) {
            int raw = 0;
            ESP_ERROR_CHECK(adc_oneshot_read(
                adc_handle, LDR_ADC_CHANNEL, &raw));
            total += raw;
            vTaskDelay(pdMS_TO_TICKS(2));
        }

        const int raw = total / SAMPLE_COUNT;
        int voltage_mv = -1;
        if (calibrated) {
            ESP_ERROR_CHECK(adc_cali_raw_to_voltage(
                calibration_handle, raw, &voltage_mv));
        }

#if CALIBRATION_READY
        float lux = adc_to_lux(raw);
        if (lux < 0) {
            lux = 0;
        }
        printf("%lu,%d,%d,%.2f\n",
               (unsigned long)(xTaskGetTickCount() * portTICK_PERIOD_MS),
               raw, voltage_mv, lux);
#else
        printf("%lu,%d,%d,NA\n",
               (unsigned long)(xTaskGetTickCount() * portTICK_PERIOD_MS),
               raw, voltage_mv);
#endif

        vTaskDelay(pdMS_TO_TICKS(SAMPLE_INTERVAL_MS));
    }
}
