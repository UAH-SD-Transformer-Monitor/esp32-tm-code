#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <energyic_UART.h>
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
#ifdef TM_MQTT_PASSWD
   char const  *mqttPass = STR(TM_MQTT_PASSWD);
#endif

#define PIN_SerialATM_RX       16   //RX pin, CHANGE IT according to your board
#define PIN_SerialATM_TX       17   //TX pin, CHANGE IT according to your board

void messageReceived(String &topic, String &payload);


WiFiClientSecure wifiClient;
PubSubClient mqttClient;


HardwareSerial ATMSerial(1);        //1 = just hardware serial number. ESP32 supports 3 hardware serials. UART 0 usually for flashing.


ATM90E26_UART eic(&ATMSerial);