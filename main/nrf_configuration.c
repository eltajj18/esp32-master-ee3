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
void receiver(void *pvParameters)
{
    ESP_LOGI(pcTaskGetName(0), "Start");
    NRF24_t dev;
    Nrf24_init(&dev);
    uint8_t payload = 32;
    uint8_t channel = 115;
    Nrf24_config(&dev, channel, payload);

    // Set my own address using 5 characters
    esp_err_t ret = Nrf24_setRADDR(&dev, (uint8_t *)"FGHIJ");
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
    ESP_LOGI(pcTaskGetName(0), "Listening...");

    uint8_t buf[32];

    // Clear RX FiFo
    while (1)
    {
        if (Nrf24_dataReady(&dev) == false)
            break;
        Nrf24_getData(&dev, buf);
    }

    while (1)
    {
        // Wait for received data
        if (Nrf24_dataReady(&dev))
        {
            Nrf24_getData(&dev, buf);
            ESP_LOGI(pcTaskGetName(0), "Got data:%s", buf);
            // ESP_LOG_BUFFER_HEXDUMP(pcTaskGetName(0), buf, payload, ESP_LOG_INFO);
        }
        vTaskDelay(1);
    }
}
void sender(void *pvParameters)
{
    ESP_LOGI(pcTaskGetName(0), "Start");
    NRF24_t dev;
    Nrf24_init(&dev);
    uint8_t payload = 32;
    uint8_t channel = 115;
    Nrf24_config(&dev, channel, payload);

    // Set destination address using 5 characters
    esp_err_t ret = Nrf24_setTADDR(&dev, (uint8_t *)"FGHIJ");
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

    uint8_t buf[32];
    while (1)
    {
        TickType_t nowTick = xTaskGetTickCount();
        sprintf((char *)buf, "Hello World %" PRIu32, nowTick);
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
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void sender_best_move(NRF24_t *dev, int row_coordination, int column_coordination, bool isComputerMoveO)
{

    ESP_LOGI(pcTaskGetName(0), "Start Sending Best Move");
    esp_err_t ret = Nrf24_setTADDR(dev, (uint8_t *)"FGHIJ");
    if (ret != ESP_OK)
    {
        ESP_LOGE(pcTaskGetName(0), "NRF24L01 not installed");
        return;
    }
    // int move_type = isComputerMoveO ? 1 : 0; // Assuming '1' for 'O', '0' for 'X'
    int move_type = isComputerMoveO ? 0b01 : 0b00;
    int row_cord = row_coordination;
    int col_cord = column_coordination + 1;
    // Ensure coordinates are within the 0-3 range
    // row_coordination &= 0x03;    // Mask to keep only the last 2 bits
    // column_coordination &= 0x03; // Mask to keep only the last 2 bits
    // row_cord &= 0x03; // Mask to keep only the last 2 bits
    // col_cord &= 0x03; // Mask to keep only the last 2 bits
    int row_cord_binary = 0b00;
    int col_cord_binary = 0b00;

    if (row_cord == 0)
    {
        row_cord_binary = 0b00;
    }
    else if (row_cord == 1)
    {
        row_cord_binary = 0b01;
    }
    else if (row_cord == 2)
    {
        row_cord_binary = 0b10;
    }
    if (col_cord == 1)
    {
        col_cord_binary = 0b01;
    }
    else if (col_cord == 2)
    {
        col_cord_binary = 0b10;
    }
    else if (col_cord == 3)
    {
        col_cord_binary = 0b11;
    }
    // Encode row and column into the first and second pair of bits, respectively
    // uint8_t encoded = (row_cord << 6) | (col_cord << 4) | (move_type << 3);
    uint8_t buffer[32] = {0};
    uint8_t encoded = (row_cord_binary << 6) | (col_cord_binary << 4) | (move_type << 3);
    buffer[0] = encoded;
    // Now encoded contains the desired structure:
    // RRCCMT00 where RR = row, CC = column, and MT represents the move type (1 bit), 000 = unused.

    // Proceed with sending `encoded` as before...
    // Send encoded value
    int attempt = 0;
    
    ESP_LOGI(pcTaskGetName(0), "Attempt #%d to send best move...", attempt + 1);
    Nrf24_send(dev, buffer); // Ensure you're using the correct signature

    vTaskDelay(pdMS_TO_TICKS(10)); // Short delay to allow for transmission

    while (!Nrf24_isSend(dev, 1000))
    {

        ESP_LOGW(pcTaskGetName(0), "Send Fail. Encoded move: %02X", encoded);
        ESP_LOGI(pcTaskGetName(0), "Attempt #%d to send best move...", attempt + 1);

        // isSent = true;
        Nrf24_send(dev, buffer);
        attempt++;
        vTaskDelay(pdMS_TO_TICKS(100)); // Wait before retrying
    }

    ESP_LOGI(pcTaskGetName(0), "Send success. Encoded move: %02X", encoded);
    vTaskDelay(pdMS_TO_TICKS(20)); // Wait before retrying
}


void Nrf_bestMove_config(NRF24_t *dev)
{
    Nrf24_init(dev);
    uint8_t payload = 32;
    uint8_t channel = 115;
    Nrf24_config(dev, channel, payload);
    Nrf24_SetSpeedDataRates(dev, 0);
    Nrf24_printDetails(dev);
    vTaskDelay(pdMS_TO_TICKS(50)); // Wait after configuring
}
void Nrf_score_config(NRF24_t *dev)
{
    Nrf24_init_2(dev); // Initialize your NRF24 device
    uint8_t payload = 32;
    uint8_t channel = 115;
    Nrf24_config(dev, channel, payload);
    Nrf24_SetSpeedDataRates(dev, 0);
    Nrf24_printDetails(dev);       // Optional: Print NRF device details
    vTaskDelay(pdMS_TO_TICKS(50)); // Wait after configuring
}

void sender_score(NRF24_t *dev, uint8_t player_score, uint8_t computer_score)
{
    esp_err_t ret = Nrf24_setTADDR(dev, (uint8_t *)"ABCDE");
    if (ret != ESP_OK)
    {
        ESP_LOGE(pcTaskGetName(0), "NRF24L01 not installed");
        return;
    }
    player_score &= 0x0F; // Ensure scores are within range 0-15
    computer_score &= 0x0F;
    uint8_t buffer[32] = {0};
    uint8_t combined_score = (player_score << 4) | computer_score;
    buffer[0] = combined_score;
    ESP_LOGI(pcTaskGetName(0), "Start Sending Score");

    int attempt = 0;

    vTaskDelay(100 / portTICK_PERIOD_MS);
    ESP_LOGI(pcTaskGetName(0), "Attempt #%d to send score...", attempt + 1);
    Nrf24_send(dev, buffer); // Ensure correct send function signature
    vTaskDelay(pdMS_TO_TICKS(10));    // Short delay to wait for send completion

    while (!Nrf24_isSend(dev, 1000))
    { // Check if send was successful
        ESP_LOGI(pcTaskGetName(0), "Send failed. Retrying...");
        ESP_LOGI(pcTaskGetName(0), "Attempt #%d to send score...", attempt + 1);
        Nrf24_send(dev, buffer); // Ensure correct send function signature
        // isSent = true;
        attempt++;
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    ESP_LOGI(pcTaskGetName(0), "Send success. Combined score: %d", combined_score);
    vTaskDelay(pdMS_TO_TICKS(20)); // Wait before retrying

    // vTaskDelay(pdMS_TO_TICKS(1000)); // Wait before retrying
}

void receiver_score(NRF24_t dev)
{

    // Use the provided configuration function

    esp_err_t ret = Nrf24_setRADDR(&dev, (uint8_t *)"ABCDE");
    if (ret != ESP_OK)
    {
        ESP_LOGE(pcTaskGetName(0), "NRF24L01 not installed");
        return;
    }
    uint8_t buffer[1];
    ESP_LOGI(pcTaskGetName(0), "Start Receiving Score");

    // Clear RX FiFo
    while (1)
    {
        if (Nrf24_dataReady(&dev) == false)
            break;
        Nrf24_getData(&dev, buffer);
    }

    while (1)
    {
        if (Nrf24_dataReady(&dev))
        {
            Nrf24_getData(&dev, buffer);

            // Decode the received message
            uint8_t player_score = (buffer[0] >> 4) & 0x0F; // Extract player score from the first 4 bits
            uint8_t computer_score = buffer[0] & 0x0F;      // Extract computer score from the last 4 bits
            ESP_LOGI(pcTaskGetName(0), "Received scores: Player score %d, Computer score %d",
                     player_score, computer_score);
        }
        vTaskDelay(100 / portTICK_PERIOD_MS); // Delay to reduce CPU usage
    }
}
void receiver_best_move(NRF24_t dev)
{

    esp_err_t ret = Nrf24_setRADDR(&dev, (uint8_t *)"FGHIJ");
    if (ret != ESP_OK)
    {
        ESP_LOGE(pcTaskGetName(0), "NRF24L01 not installed");
        return;
    }
    uint8_t buffer[1];
    ESP_LOGI(pcTaskGetName(0), "Start Receiving best move");
    // Clear RX FiFo
    while (1)
    {
        if (Nrf24_dataReady(&dev) == false)
            break;
        Nrf24_getData(&dev, buffer);
    }
    while (1)
    {
        if (Nrf24_dataReady(&dev))
        {
            Nrf24_getData(&dev, buffer);

            // Decode the received message
            uint8_t row = (buffer[0] >> 6) & 0x03;          // Extract row from the first two bits
            uint8_t column = (buffer[0] >> 4) & 0x03;       // Extract column from the next two bits
            bool isComputerMoveO = (buffer[0] >> 3) & 0x01; // Extract move type

            // Log received move
            ESP_LOGI(pcTaskGetName(0), "Received move: Row %d, Column %d, Move type %s",
                     row, column, isComputerMoveO ? "O" : "X");
            vTaskDelay(100 / portTICK_PERIOD_MS); // Delay to reduce CPU usage
        }
    }
}
// void app_main(void)
// {

//     NRF24_t *dev = malloc(sizeof(NRF24_t));
//     Nrf_bestMove_config(dev);
//     // NRF24_t dev_score = Nrf_bestMove_config(dev_score);
//     // receiver_best_move(dev_score);
//     while (1)
//     {

//         printf("Starting the loop\n");
//         // vTaskDelay(15000 / portTICK_PERIOD_MS);
//         // //     printf("Sending first batch of scores\n");
//         sender_best_move(dev,0,0,true);
//         vTaskDelay(15000 / portTICK_PERIOD_MS);
//         sender_best_move(dev,1,1,true);
//         vTaskDelay(15000 / portTICK_PERIOD_MS);
//         sender_best_move(dev,2,2,true);
//         vTaskDelay(15000 / portTICK_PERIOD_MS);
//         // sender_score(dev, 0, 1);
//         // vTaskDelay(15000 / portTICK_PERIOD_MS);
//         // sender_best_move(dev,0,1,true);
//         // vTaskDelay(15000 / portTICK_PERIOD_MS);
//         // sender_best_move(dev,2,1,true);
//         // vTaskDelay(15000 / portTICK_PERIOD_MS);
//         // sender_best_move(dev,2,1,true);
//         // vTaskDelay(15000 / portTICK_PERIOD_MS);
//         // sender_best_move(dev,2,1,true);
//         // vTaskDelay(15000 / portTICK_PERIOD_MS);
//         // sender_best_move(dev,2,1,true);
//         // vTaskDelay(15000 / portTICK_PERIOD_MS);
//         // sender_best_move(dev,2,1,true);
//         // vTaskDelay(15000 / portTICK_PERIOD_MS);


//         // sender_score(dev, 1, 2);
//         // vTaskDelay(15000 / portTICK_PERIOD_MS);

//         // // //     // vTaskDelay(2000 / portTICK_PERIOD_MS);
//         //     printf("Sending second batch of moves\n");
//         // sender_score(dev, 8, 10);

//         // }

//         // sender_best_move(dev,2,1,true);
//     }
//     // // vTaskDelay(5000 / portTICK_PERIOD_MS);
//     // // printf("Sending second batch of moves\n");
//     // // sender_best_move(dev, 0, 0, false);
//     // vTaskDelay(5000 / portTICK_PERIOD_MS);

//     // // printf("Sending second batch of moves\n");
//     // // sender_best_move(dev, 1, 2, COMPUTER_MOVE);
//     // // printf("Sending third batch of moves\n");
//     // // sender_best_move(dev, 3, 2, COMPUTER_MOVE);
//     // }
//     // printf("Sending first batch of moves\n");
//     // NRF24_t dev = sender_best_move(1, 2);
//     // vTaskDelay(1000 / portTICK_PERIOD_MS);
//     // printf("Sending second batch of moves\n");
//     // sender_best_move_2(dev, 0, 0);
//     // printf("Sending third batch of moves\n");
//     // sender_best_move_2(dev, 0, 0);
// }