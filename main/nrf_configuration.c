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
#include "include/game_config.h"

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
    // Print settings
    Nrf24_printDetails(&dev);

    uint8_t buf[2];
    while (1)
    {

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
        vTaskDelay(3000 / portTICK_PERIOD_MS);
    }
}

void sender_best_move(NRF24_t dev, int row_coordination, int column_coordination,bool isComputerMoveO)
{

    ESP_LOGI(pcTaskGetName(0), "Start Sending Best Move");
    esp_err_t ret = Nrf24_setTADDR(&dev, (uint8_t *)"FGHIJ");
    if (ret != ESP_OK)
    {
        ESP_LOGE(pcTaskGetName(0), "NRF24L01 not installed");
        return;
    }
    uint8_t move_type = isComputerMoveO ? 1 : 0;  // Assuming '1' for 'O', '0' for 'X'

    // Ensure coordinates are within the 0-3 range
    row_coordination &= 0x03;    // Mask to keep only the last 2 bits
    column_coordination &= 0x03; // Mask to keep only the last 2 bits

    // Encode row and column into the first and second pair of bits, respectively
    uint8_t encoded = (row_coordination << 6) | (column_coordination << 4) | (move_type << 3);

    // Now encoded contains the desired structure:
    // RRCCMT00 where RR = row, CC = column, and MT represents the move type (1 bit), 000 = unused.

    // Proceed with sending `encoded` as before...
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
            vTaskDelay(pdMS_TO_TICKS(2000)); // Wait before retrying
        }
        attempt++;
    }

    if (!isSent)
    {
        ESP_LOGE(pcTaskGetName(0), "Failed to send best move after %d attempts.", MAX_RETRY_ATTEMPTS);
    }
}

NRF24_t Nrf_bestMove_config(NRF24_t dev)
{
    Nrf24_init(&dev);
    uint8_t payload = 1;
    uint8_t channel = 115;
    Nrf24_config(&dev, channel, payload);
    Nrf24_SetSpeedDataRates(&dev, 1);
    Nrf24_printDetails(&dev);
    vTaskDelay(pdMS_TO_TICKS(50)); // Wait after configuring
    return dev;
}
NRF24_t Nrf_score_config(NRF24_t dev)
{
    Nrf24_init_2(&dev); // Initialize your NRF24 device
    uint8_t payload = 1;
    uint8_t channel = 115;
    Nrf24_config(&dev, channel, payload);
    Nrf24_SetSpeedDataRates(&dev, 0);
    Nrf24_printDetails(&dev);      // Optional: Print NRF device details
    vTaskDelay(pdMS_TO_TICKS(50)); // Wait after configuring
    return dev;
}

void sender_score(NRF24_t dev, uint8_t player_score, uint8_t computer_score)
{
    esp_err_t ret = Nrf24_setTADDR(&dev, (uint8_t *)"FGHIJ");
    if (ret != ESP_OK)
    {
        ESP_LOGE(pcTaskGetName(0), "NRF24L01 not installed");
        return;
    }
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
//     NRF24_t dev = Nrf_bestMove_config(dev);
//     vTaskDelay(1000 / portTICK_PERIOD_MS);
//     printf("Sending first batch of moves\n");
//     sender_best_move(dev, 0, 0);
//     vTaskDelay(5000 / portTICK_PERIOD_MS);

//     printf("Sending second batch of moves\n");
//     sender_best_move(dev, 0, 0);
//     // printf("Sending first batch of moves\n");
//     // NRF24_t dev = sender_best_move(1, 2);
//     // vTaskDelay(1000 / portTICK_PERIOD_MS);
//     // printf("Sending second batch of moves\n");
//     // sender_best_move_2(dev, 0, 0);
//     // printf("Sending third batch of moves\n");
//     // sender_best_move_2(dev, 0, 0);
// }