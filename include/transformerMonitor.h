#include <WiFiClientSecure.h>
#include <MQTT.h>
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

#define MQTT_BROKER_URI "mqtt://"

void messageReceived(String &topic, String &payload);


WiFiClientSecure wifiClient;
MQTTClient client;