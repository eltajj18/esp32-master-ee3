#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "include/http_request.h"
#include "include/minimax.h"
#include <stdbool.h>

volatile bool is_processing = false;
// #define BLINK_GPIO_1 2
#define BUTTON_GPIO 18

// int count = 0;


void button_press_task(void *pvParameter) {
    // Ensure we don't react to any button presses until ready
    if(is_processing) {
        vTaskDelete(NULL); // Safely exit this task if it's somehow started while processing
        return;
    }

    is_processing = true;
    
    post_rest_button(); // Assume this sends a signal to the server to capture an image
    
    // Start polling for the new array after the initial post request
    // This function should contain logic that waits for the new game state to be ready
    // and retrieves it. Once done, is_processing can be set to false to allow new button presses.
    if(poll_if_ready()){
       get_rest_array();
       char* board = get_game_state();
       
    }
    //check if game is over , if yes send the score through the nrf
    //else calculate the next move and sent it through the nrf and that is it






    is_processing = false; // Ready for a new button press now

    vTaskDelete(NULL); // Task completed, clean up
}



// void app_main(void) 
// {
//     nvs_flash_init();
//     wifi_connection();
//         /* Reset the pin */
//     gpio_reset_pin(BUTTON_GPIO);
//     /* Set the GPIOs to Output mode */
//     gpio_set_direction(BUTTON_GPIO, GPIO_MODE_INPUT);
//     /* Enable Pullup for Input Pin */
//     gpio_pullup_en(BUTTON_GPIO);
    
//     while (1) 
//     {
//         if( gpio_get_level(BUTTON_GPIO) == 0 && !is_processing )
//         {

//         vTaskDelay(1000 / portTICK_PERIOD_MS); //to prevent multiple presses
//         xTaskCreate(button_press_task, "button_press_task", 2048, NULL, 10, NULL);


//         // count++;
//         // post_rest_button(); //trigger the server to capture the image
//         // //check if new array is available
//         // //retrieve the new array
//         // //put it inside the minimax function
//         // //send the value via nrfs

//         }
//     }
// }