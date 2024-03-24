#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include "esp_http_client.h"

// Defines the size of the game state array
// #define GAME_SIZE 9

// Global variable declaration for game state
// extern char game_state[GAME_SIZE];

/**
 * @brief Initializes the Wi-Fi connection using predefined SSID and password.
 */
void wifi_connection(void);

/**
 * @brief HTTP event handler for processing GET requests' responses.
 *
 * @param evt The event handle.
 * @return esp_err_t The status result of event handling.
 */
esp_err_t client_event_get_handler(esp_http_client_event_handle_t evt);

/**
 * @brief Performs an HTTP GET request to retrieve the game state array.
 */
void get_rest_array(void);

/**
 * @brief Retrieves the current game state.
 *
 * @return char* Pointer to the game state array.
 */
char *get_game_state(void);

/**
 * @brief HTTP event handler for processing POST requests' responses.
 *
 * @param evt The event handle.
 * @return esp_err_t The status result of event handling.
 */
esp_err_t client_event_post_handler(esp_http_client_event_handle_t evt);

/**
 * @brief Performs an HTTP POST request to trigger a button action.
 */
void post_rest_button(void);

/**
 * @brief HTTP event handler for processing GET is_ready requests' responses.
 *
 * @param evt The event handle.
 * @return esp_err_t The status result of event handling.
 */
esp_err_t http_event_handler(esp_http_client_event_t *evt);
/**
 * @brief Performs an HTTP GET request to see if the array_data is updated.
 */
bool poll_if_ready();

// void drawsa();
#endif // HTTP_REQUEST_H
