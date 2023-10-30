#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <SPI.h>
// Turn build flags (Macros) into strings
#define ST(A) #A
#define STR(A) ST(A)

#ifdef TM_WIFI_SSID
   char const *ssid = STR(TM_WIFI_SSID);
#endif
#ifdef TM_WIFI_PASSWD
   char const *wifiPassword = STR(TM_WIFI_PASSWD);
#endif
#ifdef TM_MQTT_PORT
   int mqttPort = atoi(STR(TM_MQTT_PORT));
#endif
#ifdef TM_MQTT_SERVER
   uint16_t mqttPort = (uint16_t) strtoul(STR(TM_MQTT_SERVER), NULL, 10);
#endif



void messageReceived(String &topic, String &payload);


WiFiClientSecure wifiClient;
PubSubClient mqttClient;