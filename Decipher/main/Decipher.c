/*
 * SPDX-FileCopyrightText: 2022-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "soc/soc_caps.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include <rom/ets_sys.h>

const static char *TAG = "ADC";

/*---------------------------------------------------------------
        ADC General Macros
---------------------------------------------------------------*/
//ADC1 Channels
#define EXAMPLE_ADC1_CHAN0          ADC_CHANNEL_0
#define EXAMPLE_ADC_ATTEN           ADC_ATTEN_DB_0

static int adc_raw[2][10];

// Morse code table: electronics-notes.com/articles/ham_radio/morse_code/characters-table-chart.php
const char letters[43] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ',', '?', '-', '(', '.', ')', '/'};
const char *code[43] = {".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..", ".---", "-.-", ".-..", "--", "-.", "---", ".--.", "--.-", ".-.", "...", "-", "..-", "...-", ".--", "-..-", "-.--", "--..", "-----", ".----", "..---", "...--", "....-", ".....", "-....", "--...", "---..", "----.", "--..--", "..--..", "-....-", "-.--.", ".-.-.-","-.--.-", "-..-." };


void decipher(char *arr) 
{
    for (int i = 0; i < 43; i++) {
        if (strcmp(code[i], arr) == 0) {
            putchar(letters[i]);
            break;
        }
    }
}


void app_main(void)
{
    //-------------ADC1 Init---------------//
    adc_oneshot_unit_handle_t adc1_handle;
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

    //-------------ADC1 Config---------------//
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = EXAMPLE_ADC_ATTEN,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, EXAMPLE_ADC1_CHAN0, &config));

    char morseBuffer[256] = {'\0'};
    int morseIndex = 0;
    bool signalDetected = false;
    bool needSpace = false;
    TickType_t signalStartTime = 0;
    TickType_t lastSignalTime = xTaskGetTickCount();
    float lets = 0;
    TickType_t charTime = 0;
    

    // TIME_UNIT = 0.01

    while (1) {
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, EXAMPLE_ADC1_CHAN0, &adc_raw[0][0]));
        int adc_value = adc_raw[0][0];
        TickType_t currentTime = xTaskGetTickCount();


        if (adc_value == 4095) {
            
            if (!signalDetected) {
                signalDetected = true;
                signalStartTime = currentTime;
                if (charTime == 0) {
                    charTime = currentTime;
                }
            }
        
        } else {
            if (signalDetected) {
                // Signal ended, determine if it was a dot or a dash
                TickType_t duration = currentTime - signalStartTime;

                if (duration <= pdMS_TO_TICKS(10) && duration > 0) {  
                    morseBuffer[morseIndex++] = '.';
                } else if (duration <= pdMS_TO_TICKS(30) && duration > 0) {  
                    morseBuffer[morseIndex++] = '-';         
                } 
                signalDetected = false;
                lastSignalTime = currentTime;
            } else {
                TickType_t gapDuration = currentTime - lastSignalTime;
                if (gapDuration >= pdMS_TO_TICKS(30) && morseIndex > 0) {
                    morseBuffer[morseIndex++] = '\0';
                    decipher(morseBuffer);
                    lets++;
                    memset(morseBuffer, 0x0, sizeof(morseBuffer));
                    morseIndex = 0;
                    lastSignalTime = currentTime;
                    needSpace = true;
                } 
                if (gapDuration == pdMS_TO_TICKS(70) && needSpace) {
                    putchar(' ');
                    needSpace = false;
                }

                if (gapDuration == pdMS_TO_TICKS(100)) {
                    putchar('\n');
                    lastSignalTime = currentTime;
 
                    if (charTime != 0) { 
                        TickType_t currCharTime = currentTime - charTime;
                        float currCharTimeInSeconds = (float)pdTICKS_TO_MS(currCharTime) / 1000.0;
                        printf("Ch/S: %.2f\n", lets / currCharTimeInSeconds);
                    }
                    charTime = 0;
                    lets = 0;
                }
            }
        
        }
        // vTaskDelay(pdMS_TO_TICKS(25));  
        // ets_delay_us(10);
    }
    
}
