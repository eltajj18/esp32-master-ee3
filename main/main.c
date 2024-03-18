#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "include/http_request.h"
#include "include/minimax.h"
#include <stdbool.h>
#include "nvs_flash.h"

volatile bool is_processing = false;
// #define BLINK_GPIO_1 2
#define BUTTON_GPIO 18
// int count = 0;
int player_score = 0;
int computer_score = 0;

void evaluate_game(int evaluation)
{
    if (evaluation == 10)
    {
        computer_score++;
    }
    else if (evaluation == -10)
    {
        player_score++;
    }
    else
    {
        printf("It's a draw\n");
        // no scores added;
    }
}

void button_press_task(void *pvParameter)
{
    // Ensure we don't react to any button presses until ready
    if (is_processing)
    {
        vTaskDelete(NULL); // Safely exit this task if it's somehow started while processing
        return;
    }

    is_processing = true;

    char *game_state;
    post_rest_button(); // Assume this sends a signal to the server to capture an image

    // Start polling for the new array after the initial post request
    // This function should contain logic that waits for the new game state to be ready
    // and retrieves it. Once done, is_processing can be set to false to allow new button presses.
    if (poll_if_ready())
    {
        get_rest_array();
        game_state = get_game_state();

        char board[3][3] = {};
        transformArrayTo3x3(game_state, board);
        draw(board);

        int evaluation;
        Move bestMove;

        if (!isMovesLeft(board))
        {
            evaluation = evaluate(board);
            evaluate_game(evaluation);
            // NRF send the score
        }
        else
        {
            bestMove = findBestMove(board);
            // NRF send the best move
        }
    }
    else
    {
        printf("Game state not ready yet\n");
    }
    is_processing = false; // Ready for a new button press now

    vTaskDelete(NULL); // Task completed, clean up
}

void app_main(void)
{
    nvs_flash_init();
    wifi_connection();
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    printf("WIFI was initiated ...........\n\n");
    /* Reset the pin */
    gpio_reset_pin(BUTTON_GPIO);
    /* Set the GPIOs to Output mode */
    gpio_set_direction(BUTTON_GPIO, GPIO_MODE_INPUT);
    /* Enable Pullup for Input Pin */
    gpio_pullup_en(BUTTON_GPIO);

    while (1)
    {
        if (gpio_get_level(BUTTON_GPIO) == 0 && !is_processing)
        {

            vTaskDelay(1000 / portTICK_PERIOD_MS); // to prevent multiple presses
            xTaskCreate(button_press_task, "button_press_task", 2048, NULL, 10, NULL);
        }
    }
}