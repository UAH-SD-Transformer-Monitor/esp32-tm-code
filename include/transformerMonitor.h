#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <ArduinoJson.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#ifdef ATM90E26
  #include <energyic_UART.h>
#else // we are using the ATM90E36
  #include <ATM90E36.h>
#endif

// Turn build flags (Macros) into strings
#define ST(A) #A
#define STR(A) ST(A)

// extract the WiFi SSID from its macro
#ifdef TM_WIFI_SSID
  char const *ssid = STR(TM_WIFI_SSID);
#endif

// extract the WiFi Password from its macro
#ifdef TM_WIFI_PASSWD
  char const *wifiPassword = STR(TM_WIFI_PASSWD);
#endif

// extract the MQTT Port from its macro
#ifdef TM_MQTT_PORT
  uint16_t mqttPort = (uint16_t) strtoul(STR(TM_MQTT_PORT), NULL, 10);
#endif
// extract the MQTT server hostname from its macro
#ifdef TM_MQTT_SVR
  char const  *mqttServer = STR(TM_MQTT_SVR);
#endif
// extract the MQTT username from its macro
#ifdef TM_MQTT_USER
  char const  *mqttUser = STR(TM_MQTT_USER);
#endif
// extract the MQTT password from its macro
#ifdef TM_MQTT_PASSWD
  char const  *mqttPass = STR(TM_MQTT_PASSWD);
#endif

// include the cert if using SSL
#ifdef TM_MQTT_SSL
  #include <transformerMonitorServerCert.h>
#endif

// pins for UART 2
#define PIN_SerialATM_RX       16   //RX pin, CHANGE IT according to your board
#define PIN_SerialATM_TX       17   //TX pin, CHANGE IT according to your board

void messageReceived(String &topic, String &payload);


WiFiClientSecure wifiClient;
PubSubClient mqttClient;



HardwareSerial ATMSerial(1);        //1 = just hardware serial number. ESP32 supports 3 hardware serials. UART 0 usually for flashing.

#ifdef ATM90E26_EIC
  ATM90E26_UART eic(&ATMSerial);
#else
  ATM90E36 eic36;
#endif

// we are using the ESP32's MAC address to provide a unique ID
String client_id = "xformermon-";

// GPIO where the DS18B20 is connected to
const int oilTempBus = 4;
const int cabinetTempBus = 9;


// Setup a oneWire instance to communicate with any OneWire devices
OneWire cabinetTempBusOneWire(cabinetTempBus);
OneWire oilTempBusOneWire(oilTempBus);


// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature cabinetTemp(&cabinetTempBusOneWire);
DallasTemperature oilTemp(&oilTempBusOneWire);



struct tempSensors {             // Structure declaration
  DallasTemperature cabinet;   // Cabinet Temp (DallasTemperature variable)
  DallasTemperature oil;   // Oil Temp (DallasTemperature variable)
};

tempSensors monitorTempSensors{cabinetTemp, oilTemp};
