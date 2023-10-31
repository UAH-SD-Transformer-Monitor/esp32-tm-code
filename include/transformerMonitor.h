#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <ArduinoJson.h>
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
   uint16_t mqttPort = (uint16_t) strtoul(STR(TM_MQTT_PORT), NULL, 10);
#endif
#ifdef TM_MQTT_SVR
   char const  *mqttServer = STR(TM_MQTT_SVR);
#endif



void messageReceived(String &topic, String &payload);


WiFiClientSecure wifiClient;
PubSubClient mqttClient;