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
#include "cJSON.h"
#include "include/http_request.h"


char game_state[9];

int retry_num = 0;

static void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_id == WIFI_EVENT_STA_START)
    {
        printf("WIFI CONNECTING....\n");
    }
    else if (event_id == WIFI_EVENT_STA_CONNECTED)
    {
        printf("WiFi CONNECTED\n");
    }
    else if (event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        printf("WiFi lost connection\n");
        if (retry_num < 5)
        {
            esp_wifi_connect();
            retry_num++;
            printf("Retrying to Connect...\n");
        }
    }
    else if (event_id == IP_EVENT_STA_GOT_IP)
    {
        printf("Wifi got IP...\n\n");
    }
}
void wifi_connection()
{
    //                          s1.4
    // 2 - Wi-Fi Configuration Phase
    esp_netif_init();
    esp_event_loop_create_default();     // event loop                    s1.2
    esp_netif_create_default_wifi_sta(); // WiFi station                      s1.3
    wifi_init_config_t wifi_initiation = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&wifi_initiation); //
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL);
    wifi_config_t wifi_configuration = {
        .sta = {
            .ssid = SSID,
            .password = PASSWORD,

        }

    };
    strcpy((char *)wifi_configuration.sta.ssid, SSID);
    strcpy((char *)wifi_configuration.sta.password, PASSWORD);
    // esp_log_write(ESP_LOG_INFO, "Kconfig", "SSID=%s, PASS=%s", ssid, pass);
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_configuration);
    // 3 - Wi-Fi Start Phase
    esp_wifi_start();
    esp_wifi_set_mode(WIFI_MODE_STA);
    // 4- Wi-Fi Connect Phase
    esp_wifi_connect();
    printf("wifi_init_softap finished. SSID:%s  password:%s", SSID, PASSWORD);
}
esp_err_t client_event_get_handler(esp_http_client_event_handle_t evt)
{
    switch (evt->event_id)
    {
    case HTTP_EVENT_ON_DATA:
        // printf("HTTP_EVENT_ON_DATA: %.*s\n", evt->data_len, (char *)evt->data);
        if (!esp_http_client_is_chunked_response(evt->client))
        {
            // Assuming the complete data is received in one go, which might not be the case
            cJSON *json = cJSON_Parse(evt->data);
            if (!json)
            {
                printf("Error before: [%s]\n", cJSON_GetErrorPtr());
            }
            else
            {
                if (cJSON_IsArray(json))
                {
                    for (int i = 0; i < cJSON_GetArraySize(json); i++)
                    {
                        cJSON *item = cJSON_GetArrayItem(json, i);
                        if (cJSON_IsString(item) && (item->valuestring != NULL))
                        {
                            // Assuming single characters for simplicity
                            game_state[i] = item->valuestring[0];
                        }
                    }
                }
                cJSON_Delete(json);
            }
        }
        break;

    default:
        break;
    }
    return ESP_OK;
}
void get_rest_array()
{
    esp_http_client_config_t config_get = {
        .url = SERVER_URL_GET,
        .method = HTTP_METHOD_GET,
        .cert_pem = NULL,
        .event_handler = client_event_get_handler};

    esp_http_client_handle_t client = esp_http_client_init(&config_get);
    esp_http_client_perform(client);
    esp_http_client_cleanup(client);
}

char* get_game_state()
{
    return game_state;
}

esp_err_t client_event_post_handler(esp_http_client_event_handle_t evt)
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

void post_rest_button()
{
    esp_http_client_config_t config_post = {
        .url = SERVER_URL_POST,
        .method = HTTP_METHOD_POST,
        .cert_pem = NULL,
        .event_handler = client_event_post_handler};

    esp_http_client_handle_t client = esp_http_client_init(&config_post);

    const char *post_data = "{\"action\": \"capture\"}";
    esp_http_client_set_post_field(client, post_data, strlen(post_data));
    esp_http_client_set_header(client, "Content-Type", "application/json");

    esp_http_client_perform(client);
    esp_http_client_cleanup(client);
}

static bool is_array_ready = false;

esp_err_t http_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id)
    {
    case HTTP_EVENT_ON_DATA:
        if (!esp_http_client_is_chunked_response(evt->client))
        {
            // Parse JSON response directly within the event handler
            cJSON *json = cJSON_Parse(evt->data);
            if (json != NULL)
            {
                printf("HTTP_EVENT_ON_DATA: %.*s\n", evt->data_len, (char *)evt->data);

                cJSON *is_ready_json = cJSON_GetObjectItemCaseSensitive(json, "is_ready");
                is_array_ready = cJSON_IsTrue(is_ready_json);
                printf(is_array_ready ? "Array is ready\n" : "Array is not ready\n");
                cJSON_Delete(json);
            }
        }
        break;
    default:
        break;
    }
    return ESP_OK;
}
bool poll_if_ready() {
    esp_http_client_config_t config = {
        .url = SERVER_URL_ARRAY_READY,
        .event_handler = http_event_handler,
        // Add additional configuration as necessary
    };

    int retry = 0;
    const int retry_limit = 10; // Adjust as necessary
    bool result = false;

    while (!is_array_ready) {
        printf("Retry %d\n", retry);
        esp_http_client_handle_t client = esp_http_client_init(&config);
        
        // Reset the readiness state before each request
        is_array_ready = false;

        esp_err_t err = esp_http_client_perform(client);
        if (err == ESP_OK) {
            // The event handler will process the response and update is_array_ready
            // Wait a short time for the response to be processed
            vTaskDelay(pdMS_TO_TICKS(2000)); // 2 seconds delay between retries
        } else {
            ESP_LOGE("HTTP_POLL", "HTTP Request failed: %s", esp_err_to_name(err));
        }

        esp_http_client_cleanup(client);

        // Check if the array became ready during this iteration
        if (is_array_ready) {
            result = true;
            break; // Exit the loop if the array is ready
        }

        retry++;
    }

    return result; // Returns true if the array is ready, false otherwise (including on max retries)
}


// void app_main(void)
// {
//     nvs_flash_init();
//     wifi_connection();

//     vTaskDelay(2000 / portTICK_PERIOD_MS);
//     printf("WIFI was initiated ...........\n\n");

  
// }