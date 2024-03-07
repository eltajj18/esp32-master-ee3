// // HTTP Client - FreeRTOS ESP IDF - GET
// #include <string.h>
// #include <sys/param.h>
// #include <stdlib.h>
// #include <ctype.h>
// #include "esp_log.h"
// #include "nvs_flash.h"
// #include "esp_event.h"
// #include "esp_netif.h"
// #include "esp_tls.h"
// #include "esp_wifi.h"
// #if CONFIG_MBEDTLS_CERTIFICATE_BUNDLE
// #include "esp_crt_bundle.h"
// #endif
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "esp_system.h"
// #include "esp_http_client.h"
// #include "include/wifi_connection.h"
// #include <stdio.h>
// #include "freertos/timers.h"
// #include "freertos/event_groups.h"
// #include "esp_err.h"

// #define MAX_HTTP_RECV_BUFFER 512
// #define MAX_HTTP_OUTPUT_BUFFER 2048
// static const char *TAG = "HTTP_CLIENT";

// static void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
// {
//     switch (event_id)
//     {
//     case WIFI_EVENT_STA_START:
//         printf("WiFi connecting ... \n");
//         break;
//     case WIFI_EVENT_STA_CONNECTED:
//         printf("WiFi connected ... \n");
//         break;
//     case WIFI_EVENT_STA_DISCONNECTED:
//         printf("WiFi lost connection ... \n");
//         break;
//     case IP_EVENT_STA_GOT_IP:
//         printf("WiFi got IP ... \n\n");
//         break;
//     default:
//         break;
//     }
// }
// void wifi_connection()
// {
//     // 1 - Wi-Fi/LwIP Init Phase
//     esp_netif_init();                    // TCP/IP initiation 					s1.1
//     esp_event_loop_create_default();     // event loop 			                s1.2
//     esp_netif_create_default_wifi_sta(); // WiFi station 	                    s1.3
//     wifi_init_config_t wifi_initiation = WIFI_INIT_CONFIG_DEFAULT();
//     esp_wifi_init(&wifi_initiation); // 					                    s1.4
//     // 2 - Wi-Fi Configuration Phase
//     esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL);
//     esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL);
//     wifi_config_t wifi_configuration = {
//         .sta = {
//             .ssid = SSID,
//             .password = PASSWORD}};
//     esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_configuration);
//     // 3 - Wi-Fi Start Phase
//     esp_wifi_start();
//     // 4- Wi-Fi Connect Phase
//     esp_wifi_connect();
// }
// esp_err_t _http_event_handler(esp_http_client_event_t *evt)
// {
//     static char *output_buffer; // Buffer to store response of http request from event handler
//     static int output_len;      // Stores number of bytes read
//     switch (evt->event_id)
//     {
//     case HTTP_EVENT_ERROR:
//         ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
//         break;
//     case HTTP_EVENT_ON_CONNECTED:
//         ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
//         break;
//     case HTTP_EVENT_HEADER_SENT:
//         ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
//         break;
//     case HTTP_EVENT_ON_HEADER:
//         ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
//         break;
//     case HTTP_EVENT_ON_DATA:
//         ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
//         // Clean the buffer in case of a new request
//         if (output_len == 0 && evt->user_data)
//         {
//             // we are just starting to copy the output data into the use
//             memset(evt->user_data, 0, MAX_HTTP_OUTPUT_BUFFER);
//         }
//         /*
//          *  Check for chunked encoding is added as the URL for chunked encoding used in this example returns binary data.
//          *  However, event handler can also be used in case chunked encoding is used.
//          */
//         if (!esp_http_client_is_chunked_response(evt->client))
//         {
//             // If user_data buffer is configured, copy the response into the buffer
//             int copy_len = 0;
//             if (evt->user_data)
//             {
//                 // The last byte in evt->user_data is kept for the NULL character in case of out-of-bound access.
//                 copy_len = MIN(evt->data_len, (MAX_HTTP_OUTPUT_BUFFER - output_len));
//                 if (copy_len)
//                 {
//                     memcpy(evt->user_data + output_len, evt->data, copy_len);
//                 }
//             }
//             else
//             {
//                 int content_len = esp_http_client_get_content_length(evt->client);
//                 if (output_buffer == NULL)
//                 {
//                     // We initialize output_buffer with 0 because it is used by strlen() and similar functions therefore should be null terminated.
//                     output_buffer = (char *)calloc(content_len + 1, sizeof(char));
//                     output_len = 0;
//                     if (output_buffer == NULL)
//                     {
//                         ESP_LOGE(TAG, "Failed to allocate memory for output buffer");
//                         return ESP_FAIL;
//                     }
//                 }
//                 copy_len = MIN(evt->data_len, (content_len - output_len));
//                 if (copy_len)
//                 {
//                     memcpy(output_buffer + output_len, evt->data, copy_len);
//                 }
//             }
//             output_len += copy_len;
//         }

//         break;
//     case HTTP_EVENT_ON_FINISH:
//         ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
//         if (output_buffer != NULL)
//         {
//             // Response is accumulated in output_buffer. Uncomment the below line to print the accumulated response
//             // ESP_LOG_BUFFER_HEX(TAG, output_buffer, output_len);
//             free(output_buffer);
//             output_buffer = NULL;
//         }
//         output_len = 0;
//         break;
//     case HTTP_EVENT_DISCONNECTED:
//         ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
//         int mbedtls_err = 0;
//         esp_err_t err = esp_tls_get_and_clear_last_error((esp_tls_error_handle_t)evt->data, &mbedtls_err, NULL);
//         if (err != 0)
//         {
//             ESP_LOGI(TAG, "Last esp error code: 0x%x", err);
//             ESP_LOGI(TAG, "Last mbedtls failure: 0x%x", mbedtls_err);
//         }
//         if (output_buffer != NULL)
//         {
//             free(output_buffer);
//             output_buffer = NULL;
//         }
//         output_len = 0;
//         break;
//     case HTTP_EVENT_REDIRECT:
//         ESP_LOGD(TAG, "HTTP_EVENT_REDIRECT");
//         esp_http_client_set_header(evt->client, "From", "user@example.com");
//         esp_http_client_set_header(evt->client, "Accept", "text/html");
//         esp_http_client_set_redirection(evt->client);
//         break;
//     }
//     return ESP_OK;
// }

// static void http_rest_with_url(void)
// {
//     // Declare local_response_buffer with size (MAX_HTTP_OUTPUT_BUFFER + 1) to prevent out of bound access when
//     // it is used by functions like strlen(). The buffer should only be used upto size MAX_HTTP_OUTPUT_BUFFER
//     char local_response_buffer[MAX_HTTP_OUTPUT_BUFFER + 1] = {0};
//     /**
//      * NOTE: All the configuration parameters for http_client must be spefied either in URL or as host and path parameters.
//      * If host and path parameters are not set, query parameter will be ignored. In such cases,
//      * query parameter should be specified in URL.
//      *
//      * If URL as well as host and path parameters are specified, values of host and path will be considered.
//      */
//     esp_http_client_config_t config = {
//         .url = SERVER_URL,
//         .method = HTTP_METHOD_GET,
//         .event_handler = _http_event_handler,
//         .user_data = local_response_buffer, // Pass address of local buffer to get response
//         .disable_auto_redirect = true,
//     };
//     esp_http_client_handle_t client = esp_http_client_init(&config);

//     // GET
//     esp_err_t err = esp_http_client_perform(client);
//     if (err == ESP_OK)
//     {
//         ESP_LOGI(TAG, "HTTP GET Status = %d, content_length = %" PRId64,
//                  esp_http_client_get_status_code(client),
//                  esp_http_client_get_content_length(client));
//     }
//     else
//     {
//         ESP_LOGE(TAG, "HTTP GET request failed: %s", esp_err_to_name(err));
//     }
//     ESP_LOG_BUFFER_HEX(TAG, local_response_buffer, strlen(local_response_buffer));
// }

// static void http_test_task(void *pvParameters)
// {
//     http_rest_with_url();
// }
// void app_main(void)
// {
//     esp_err_t ret = nvs_flash_init();
//     if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
//     {
//         ESP_ERROR_CHECK(nvs_flash_erase());
//         ret = nvs_flash_init();
//     }
//     ESP_ERROR_CHECK(ret);

//     ESP_ERROR_CHECK(esp_netif_init());
//     ESP_ERROR_CHECK(esp_event_loop_create_default());

//     /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
//      * Read "Establishing Wi-Fi or Ethernet Connection" section in
//      * examples/protocols/README.md for more information about this function.
//      */
//     // ESP_ERROR_CHECK(example_connect());
//     ESP_LOGI(TAG, "Connected to AP, begin http example");

//     http_rest_with_url();
// }
// HTTP Client - FreeRTOS ESP IDF - GET

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_http_client.h"
#include "include/wifi_connection.h"

static void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    switch (event_id)
    {
    case WIFI_EVENT_STA_START:
        printf("WiFi connecting ... \n");
        break;
    case WIFI_EVENT_STA_CONNECTED:
        printf("WiFi connected ... \n");
        break;
    case WIFI_EVENT_STA_DISCONNECTED:
        printf("WiFi lost connection ... \n");
        break;
    case IP_EVENT_STA_GOT_IP:
        printf("WiFi got IP ... \n\n");
        break;
    default:
        break;
    }
}

void wifi_connection()
{
    // 1 - Wi-Fi/LwIP Init Phase
    esp_netif_init();                    // TCP/IP initiation 					s1.1
    esp_event_loop_create_default();     // event loop 			                s1.2
    esp_netif_create_default_wifi_sta(); // WiFi station 	                    s1.3
    wifi_init_config_t wifi_initiation = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&wifi_initiation); // 					                    s1.4
    // 2 - Wi-Fi Configuration Phase
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL);
    wifi_config_t wifi_configuration = {
        .sta = {
            .ssid = SSID,
            .password = PASSWORD}};
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_configuration);
    // 3 - Wi-Fi Start Phase
    esp_wifi_start();
    // 4- Wi-Fi Connect Phase
    esp_wifi_connect();
}

esp_err_t client_event_get_handler(esp_http_client_event_handle_t evt)
{
    switch (evt->event_id)
    {
    case HTTP_EVENT_ON_DATA:
        printf("HTTP_EVENT_ON_DATA: %.*s\n", evt->data_len, (char *)evt->data);
        break;

    default:
        break;
    }
    return ESP_OK;
}

static void rest_get()
{
    esp_http_client_config_t config_get = {
        .url = "http://martin-upward-lately.ngrok-free.app/array",
        .method = HTTP_METHOD_GET,
        .cert_pem = NULL,
        .event_handler = client_event_get_handler};

    esp_http_client_handle_t client = esp_http_client_init(&config_get);
    esp_http_client_perform(client);
    esp_http_client_cleanup(client);
}

void app_main(void)
{
    nvs_flash_init();
    wifi_connection();

    vTaskDelay(2000 / portTICK_PERIOD_MS);
    printf("WIFI was initiated ...........\n\n");

    rest_get();
}