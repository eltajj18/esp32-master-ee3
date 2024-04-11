/*	Mirf Example

    This example code is in the Public Domain (or CC0 licensed, at your option.)

    Unless required by applicable law or agreed to in writing, this
    software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
    CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <stdint.h>
#include "include/mirf.h"
#include "include/nrf_configuration.h"

// #if CONFIG_ADVANCED
// void AdvancedSettings(NRF24_t * dev)
// {
// #if CONFIG_RF_RATIO_2M
// 	ESP_LOGW(pcTaskGetName(0), "Set RF Data Ratio to 2MBps");
// 	Nrf24_SetSpeedDataRates(dev, 1);
// #endif // CONFIG_RF_RATIO_2M

// #if CONFIG_RF_RATIO_1M
// 	ESP_LOGW(pcTaskGetName(0), "Set RF Data Ratio to 1MBps");
// 	Nrf24_SetSpeedDataRates(dev, 0);
// #endif // CONFIG_RF_RATIO_2M

// #if CONFIG_RF_RATIO_250K
// 	ESP_LOGW(pcTaskGetName(0), "Set RF Data Ratio to 250KBps");
// 	Nrf24_SetSpeedDataRates(dev, 2);
// #endif // CONFIG_RF_RATIO_2M

// 	ESP_LOGW(pcTaskGetName(0), "CONFIG_RETRANSMIT_DELAY=%d", CONFIG_RETRANSMIT_DELAY);
// 	Nrf24_setRetransmitDelay(dev, CONFIG_RETRANSMIT_DELAY);
// }
// #endif // CONFIG_ADVANCED

// #if CONFIG_RECEIVER
// void receiver(void *pvParameters)
// {
//     ESP_LOGI(pcTaskGetName(0), "Start");
//     NRF24_t dev;
//     Nrf24_init(&dev);
//     uint8_t payload = 32;
//     uint8_t channel = (uint8_t)115;
//     Nrf24_config(&dev, channel, payload);

//     // Set own address using 5 characters
//     esp_err_t ret = Nrf24_setRADDR(&dev, (uint8_t *)"FGHIJ");
//     if (ret != ESP_OK)
//     {
//         ESP_LOGE(pcTaskGetName(0), "nrf24l01 not installed");
//         while (1)
//         {
//             vTaskDelay(1);
//         }
//     }

// #if CONFIG_ADVANCED
//     AdvancedSettings(&dev);
// #endif // CONFIG_ADVANCED

//     // Print settings
//     Nrf24_printDetails(&dev);
//     ESP_LOGI(pcTaskGetName(0), "Listening...");

//     uint8_t buf[32];

//     // Clear RX FiFo
//     while (1)
//     {
//         if (Nrf24_dataReady(&dev) == false)
//             break;
//         Nrf24_getData(&dev, buf);
//     }

//     while (1)
//     {
//         // When the program is received, the received data is output from the serial port
//         if (Nrf24_dataReady(&dev))
//         {
//             Nrf24_getData(&dev, buf);
//             ESP_LOGI(pcTaskGetName(0), "Got data:%s", buf);
//             // ESP_LOG_BUFFER_HEXDUMP(pcTaskGetName(0), buf, payload, ESP_LOG_INFO);
//         }
//         vTaskDelay(1);
//     }
// }
// // #endif // CONFIG_RECEIVER

// #if CONFIG_SENDER
void sender(void *pvParameters)
{
    ESP_LOGI(pcTaskGetName(0), "Start");
    NRF24_t dev;
    Nrf24_init(&dev);
    uint8_t payload = 2;
    uint8_t channel = (uint8_t)110;
    Nrf24_config(&dev, channel, payload);
    Nrf24_SetSpeedDataRates(&dev, 0);
    // Set the receiver address using 5 characters
    // uint8_t * pipe= {0xAA, 0xBB, 0xCC, 0xDD, 0xEE};

    esp_err_t ret = Nrf24_setTADDR(&dev, (uint8_t *)"FGHIJ");
    // esp_err_t ret = Nrf24_setTADDR(&dev, pipe);

    if (ret != ESP_OK)
    {
        ESP_LOGE(pcTaskGetName(0), "nrf24l01 not installed");
        while (1)
        {
            vTaskDelay(1);
        }
    }

#if CONFIG_ADVANCED
    AdvancedSettings(&dev);
#endif // CONFIG_ADVANCED

    // Print settings
    Nrf24_printDetails(&dev);

    uint8_t buf[2];
    while (1)
    {
        TickType_t nowTick = xTaskGetTickCount();
        sprintf((char *)buf, "A");
        Nrf24_send(&dev, buf);
        vTaskDelay(1);
        ESP_LOGI(pcTaskGetName(0), "Wait for sending.....");
        if (Nrf24_isSend(&dev, 1000))
        {
            ESP_LOGI(pcTaskGetName(0), "Send success:%s", buf);
        }
        else
        {
            ESP_LOGW(pcTaskGetName(0), "Send fail:");
        }
        vTaskDelay(4000 / portTICK_PERIOD_MS);
    }
}

void sender_best_move(NRF24_t *dev, int row_coordination, int column_coordination)
{
    // Ensure coordinates are within the 0-3 range
    row_coordination &= 0x03;    // Mask to keep only the last 2 bits
    column_coordination &= 0x03; // Mask to keep only the last 2 bits

    // Encode row and column into the first and second pair of bits, respectively
    uint8_t encoded = (row_coordination << 6) | (column_coordination << 4);

    // Now encoded contains the desired structure:
    // RRCC0000 where RR = row, CC = column, and 0000 = unused.

    // Proceed with sending `encoded` as before...
    ESP_LOGI(pcTaskGetName(0), "Start Sending Best Move");

    // Send encoded value
    int attempt = 0;
    bool isSent = false;
    while (attempt < MAX_RETRY_ATTEMPTS && !isSent)
    {
        vTaskDelay(20 / portTICK_PERIOD_MS);

        ESP_LOGI(pcTaskGetName(0), "Attempt #%d to send best move...", attempt + 1);
        Nrf24_send(&dev, &encoded); // Ensure you're using the correct signature

        // vTaskDelay(pdMS_TO_TICKS(10)); // Short delay to allow for transmission

        if (Nrf24_isSend(&dev, 1000))
        {
            ESP_LOGI(pcTaskGetName(0), "Send success. Encoded move: %02X", encoded);
            isSent = true;
        }
        else
        {
            ESP_LOGW(pcTaskGetName(0), "Send failed. Retrying...");
            vTaskDelay(pdMS_TO_TICKS(1000)); // Wait before retrying
        }
        attempt++;
    }

    if (!isSent)
    {
        ESP_LOGE(pcTaskGetName(0), "Failed to send best move after %d attempts.", MAX_RETRY_ATTEMPTS);
    }
}
void Nrf_bestMove_config(NRF24_t *dev)
{
    Nrf24_init(&dev);
    uint8_t payload = 32;
    uint8_t channel = 115;
    Nrf24_config(&dev, channel, payload);

    esp_err_t ret = Nrf24_setTADDR(&dev, (uint8_t *)"FGIJ");
    if (ret != ESP_OK)
    {
        ESP_LOGE(pcTaskGetName(0), "NRF24L01 not installed");
        return;
    }

    Nrf24_printDetails(&dev);
}
void Nrf_score_config(NRF24_t *dev)
{
    Nrf24_init_2(&dev); // Initialize your NRF24 device
    uint8_t payload = 32;
    uint8_t channel = 115;
    Nrf24_config(&dev, channel, payload);

    esp_err_t ret = Nrf24_setTADDR(&dev, (uint8_t *)"ABCD");
    if (ret != ESP_OK)
    {
        ESP_LOGE(pcTaskGetName(0), "NRF24L01 not installed");
        return; // Exit if NRF device is not properly installed
    }

    Nrf24_printDetails(&dev); // Optional: Print NRF device details
}
void sender_score(NRF24_t *dev, uint8_t player_score, uint8_t computer_score)
{
    player_score &= 0x0F; // Ensure scores are within range 0-15
    computer_score &= 0x0F;
    uint8_t combined_score = (player_score << 4) | computer_score;

    ESP_LOGI(pcTaskGetName(0), "Start Sending Score");

    int attempt = 0;
    bool isSent = false;
    while (attempt < MAX_RETRY_ATTEMPTS && !isSent)
    {
        vTaskDelay(20 / portTICK_PERIOD_MS);

        ESP_LOGI(pcTaskGetName(0), "Attempt #%d to send score...", attempt + 1);
        Nrf24_send(&dev, &combined_score); // Ensure correct send function signature
        // vTaskDelay(pdMS_TO_TICKS(10));     // Short delay to wait for send completion

        if (Nrf24_isSend(&dev, 1000))
        { // Check if send was successful
            ESP_LOGI(pcTaskGetName(0), "Send success. Combined score: %d", combined_score);
            isSent = true;
        }
        else
        {
            ESP_LOGW(pcTaskGetName(0), "Send failed. Retrying...");
            vTaskDelay(pdMS_TO_TICKS(1000)); // Wait before retrying
        }
        attempt++;
    }

    if (!isSent)
    {
        ESP_LOGE(pcTaskGetName(0), "Failed to send score after %d attempts.", MAX_RETRY_ATTEMPTS);
    }
}

// #endif // CONFIG_SENDER

// void app_main(void)
// {
//     // #if CONFIG_RECEIVER
//     // xTaskCreate(&receiver, "RECEIVER", 1024*3, NULL, 2, NULL);
//     // #endif

//     // #if CONFIG_SENDER
//     xTaskCreate(&sender, "SENDER", 1024 * 3, NULL, 2, NULL);
//     // #endif
// }