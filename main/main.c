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
#include "freertos/portmacro.h"
#include "include/nrf_configuration.h"
#include "include/mirf.h"
#include "include/game_config.h"

volatile bool is_processing = false;
// #define BLINK_GPIO_1 2
#define BUTTON_GPIO 18

NRF24_t dev_score;
NRF24_t dev_medium_move;
// int count = 0;
uint8_t player_score = 0;
uint8_t computer_score = 0;

void evaluate_game(int evaluation)
{
    if (evaluation == 10)
    {
        computer_score++;
        printf("Computer wins\n");
    }
    else if (evaluation == -10)
    {
        player_score++;
        printf("Player wins\n");
    }
    else
    {
        printf("It's a draw\n");
        // no scores added;
    }
}

void button_press_task(void *pvParameter)
{
    if (is_processing)
    {
        goto exit;
    }

    is_processing = true;

    printf("Button pressed\n");
    post_rest_button(); // Sends a signal to the server to capture an image

    if (!poll_if_ready())
    {
        printf("Failed to go further\n");
        goto cleanup; // Jump to cleanup if polling is not ready
    }
    get_rest_array();

    // Only proceed if polling is successful
    char *game_state = get_game_state();
    char board[3][3] = {};
    transformArrayTo3x3(game_state, board);
    // drawsa();
    draw(board);

    char firstMoveSymbol;
    configureMoves(board, &firstMoveSymbol);
    printf("Computer plays as: %c\n", COMPUTER_MOVE);

    vTaskDelay(200 / portTICK_PERIOD_MS);
    if (!isMovesLeft(board))
    {
        int evaluation = evaluate(board);
        evaluate_game(evaluation);
        printf("The score is: %d\n", evaluation);
        // sender_score(dev_score, player_score, computer_score);
    }
    else
    {
        if (evaluate(board) == 0)
        {
            /*Check if there is no winnner*/
            Move mediumMove = medium(board);
            printf("The best move is: %d %d\n", mediumMove.row, mediumMove.col);
            sender_best_move(dev_medium_move, mediumMove.row, mediumMove.col, isComputerMoveO());
            // board[mediumMove.row][mediumMove.col] = COMPUTER_MOVE;
            // if (evaluate(board) != 0)
            // {
            //     vTaskDelay(3000 / portTICK_PERIOD_MS); // wait for the machine to draw the board then send the score for dramatic effect
            //     evaluate_game(evaluate(board));
            //     printf("The score is: %d\n", evaluate(board));
            //     // sender_score(dev_score, player_score, computer_score);
            // }

            // else
            // {
            //     draw(board); // otherwise just draw the board and move to show the output
            // }
        }
        else
        {
            /*if there is then give the score*/
            int evaluation = evaluate(board);
            evaluate_game(evaluation);
            printf("The score is: %d\n", evaluation);
            // sender_score(dev_score, player_score, computer_score);
        }
    }
    vTaskDelay(3000 / portTICK_PERIOD_MS); // wait for the machine to draw the board
cleanup:
    // Clean up and prepare for the next button press
    printf("Getting ready for a new button press\n\n");
    is_processing = false;

exit:
    // Explicitly delete this task to free up resources
    vTaskDelete(NULL);
}

void app_main(void)
{
    printf("Entered to app_main\n");
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

    /*Configure Nrf24 that is responsible for sending best move coordinations*/
    dev_medium_move = Nrf_bestMove_config(dev_medium_move);

    /*Configure Nrf24 that is responsible for sending score*/
    dev_score = Nrf_score_config(dev_score);

    while (1)
    {
        vTaskDelay(20 / portTICK_PERIOD_MS);
        if (gpio_get_level(BUTTON_GPIO) == 0 && !is_processing)
        {

            vTaskDelay(1000 / portTICK_PERIOD_MS); // to prevent multiple presses
            xTaskCreate(button_press_task, "button_press_task", 10000, NULL, 10, NULL);
            // button_press_task();
        }
    }
}
