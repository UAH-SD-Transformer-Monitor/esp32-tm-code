// /* MQTT over SSL Example

//    This example code is in the Public Domain (or CC0 licensed, at your option.)

//    Unless required by applicable law or agreed to in writing, this
//    software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
//    CONDITIONS OF ANY KIND, either express or implied.
// */

// #include <stdio.h>
// #include <stdint.h>
// #include <stddef.h>
// #include <string.h>
// #include <esp_system.h>
// #include <esp_partition.h>
// #include <nvs_flash.h>
// #include <esp_event.h>
// #include <esp_netif.h>

// #include <esp_log.h>
// #include <mqtt_client.h>
// #include <esp_tls.h>
// #include <esp_ota_ops.h>
// #include <sys/param.h>

// #include <transformerMonitorServerCert.h>

// static const char *TAG = "mqtts_example";

// #define MQTT_BROKER_URI "mqtt://"

// /*
//  * @brief Event handler registered to receive MQTT events
//  *
//  *  This function is called by the MQTT client event loop.
//  *
//  * @param handler_args user data registered to the event.
//  * @param base Event base for the handler(always MQTT Base in this example).
//  * @param event_id The id for the received event.
//  * @param event_data The data for the event, esp_mqtt_event_handle_t.
//  */
// static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
// {
//     ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32, base, event_id);
//     esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;
//     esp_mqtt_client_handle_t client = event->client;
//     int msg_id;
//     switch ((esp_mqtt_event_id_t)event_id) {
//     case MQTT_EVENT_CONNECTED:
//         ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
//         msg_id = esp_mqtt_client_subscribe(client, "/topic/qos0", 0);
//         ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

//         msg_id = esp_mqtt_client_subscribe(client, "/topic/qos1", 1);
//         ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

//         msg_id = esp_mqtt_client_unsubscribe(client, "/topic/qos1");
//         ESP_LOGI(TAG, "sent unsubscribe successful, msg_id=%d", msg_id);
//         break;
//     case MQTT_EVENT_DISCONNECTED:
//         ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
//         break;

//     case MQTT_EVENT_SUBSCRIBED:
//         ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
//         msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
//         ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
//         break;
//     case MQTT_EVENT_UNSUBSCRIBED:
//         ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
//         break;
//     case MQTT_EVENT_PUBLISHED:
//         ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
//         break;
//     case MQTT_EVENT_DATA:
//         ESP_LOGI(TAG, "MQTT_EVENT_DATA");
//         printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
//         printf("DATA=%.*s\r\n", event->data_len, event->data);

//         break;
//     case MQTT_EVENT_ERROR:
//         ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
//         if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
//             ESP_LOGI(TAG, "Last error code reported from esp-tls: 0x%x", event->error_handle->esp_tls_last_esp_err);
//             ESP_LOGI(TAG, "Last tls stack error number: 0x%x", event->error_handle->esp_tls_stack_err);
//             ESP_LOGI(TAG, "Last captured errno : %d (%s)",  event->error_handle->esp_transport_sock_errno,
//                      strerror(event->error_handle->esp_transport_sock_errno));
//         } else if (event->error_handle->error_type == MQTT_ERROR_TYPE_CONNECTION_REFUSED) {
//             ESP_LOGI(TAG, "Connection refused error: 0x%x", event->error_handle->connect_return_code);
//         } else {
//             ESP_LOGW(TAG, "Unknown error type: 0x%x", event->error_handle->error_type);
//         }
//         break;
//     default:
//         ESP_LOGI(TAG, "Other event id:%d", event->event_id);
//         break;
//     }
// }

// static void mqtt_app_start(void)
// {
//     esp_mqtt_client_config_t mqtt_cfg = {
//         .uri = MQTT_BROKER_URI,
//         .port = 1883,
//         .cert_pem = root_ca,
//     };

//     mqtt_cfg.protocol_ver = (esp_mqtt_protocol_ver_t)CONFIG_MQTT_PROTOCOL_311;

//     ESP_LOGI(TAG, "[APP] Free memory: %" PRIu32 " bytes", esp_get_free_heap_size());
//     esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
//     /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
//     esp_mqtt_client_register_event(client, MQTT_EVENT_ERROR, mqtt_event_handler, client);
// 	esp_mqtt_client_register_event(client, MQTT_EVENT_CONNECTED, mqtt_event_handler, client);
// 	esp_mqtt_client_register_event(client, MQTT_EVENT_DISCONNECTED, mqtt_event_handler, client);
// 	esp_mqtt_client_register_event(client, MQTT_EVENT_SUBSCRIBED, mqtt_event_handler, client);
// 	esp_mqtt_client_register_event(client, MQTT_EVENT_UNSUBSCRIBED, mqtt_event_handler, client);
// 	esp_mqtt_client_register_event(client, MQTT_EVENT_PUBLISHED, mqtt_event_handler, client);
// 	esp_mqtt_client_register_event(client, MQTT_EVENT_DATA, mqtt_event_handler, client);
// 	esp_mqtt_client_register_event(client, MQTT_EVENT_BEFORE_CONNECT, mqtt_event_handler, client);
//     esp_mqtt_client_start(client);
// }

// void app_main(void)
// {
//     ESP_LOGI(TAG, "[APP] Startup..");
//     ESP_LOGI(TAG, "[APP] Free memory: %" PRIu32 " bytes", esp_get_free_heap_size());
//     ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

//     esp_log_level_set("*", ESP_LOG_INFO);
//     esp_log_level_set("esp-tls", ESP_LOG_VERBOSE);
//     esp_log_level_set("mqtt_client", ESP_LOG_VERBOSE);
//     esp_log_level_set("mqtt_example", ESP_LOG_VERBOSE);
//     esp_log_level_set("transport_base", ESP_LOG_VERBOSE);
//     esp_log_level_set("transport", ESP_LOG_VERBOSE);
//     esp_log_level_set("outbox", ESP_LOG_VERBOSE);

//     ESP_ERROR_CHECK(nvs_flash_init());
//     ESP_ERROR_CHECK(esp_netif_init());
//     ESP_ERROR_CHECK(esp_event_loop_create_default());


//     mqtt_app_start();
// }

