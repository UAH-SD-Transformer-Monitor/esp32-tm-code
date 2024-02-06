#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <FreeRTOSConfig.h>
#include <freertos/task.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <idf_additions.h>



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

// HardwareSerial ATMSerial(1);        //1 = just hardware serial number. ESP32 supports 3 hardware serials. UART 0 usually for flashing.

#ifdef ATM90E26_EIC
#include <ATM90E26_IC.h>
  
ATM90E26_IC eic;

#else
// extract the ATM90E36 CT LINE from its macro
#ifdef TM_ATM90E36_CT_LINE
  char const ctLine = (char) STR(TM_ATM90E36_CT_LINE);
#else
  char const ctLine = 'A';
#endif
#include <ATM90E36_IC.h>
ATM90E36 ic;
ATM90E36_IC eic(ctLine, ic);

ATM90E36_IC SetupEic(ctLine, ic);

#endif

// we are using the ESP32's MAC address to provide a unique ID
String client_id = "xformermon-";

// GPIO where the DS18B20 is connected to
const int tempBus = 4;

// Setup a oneWire instance to communicate with any OneWire devices
OneWire tempBusOneWire(tempBus);

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature tempSensors(&tempBusOneWire);


StaticJsonDocument<256> dataStore[60];
time_t now;

// Data structs for queue
struct tempData {
  float cabinetTemp, oilTemp;
};

struct powerData {
  float activePower, passivePower;
};

struct xformerMonitorData {
  unsigned short SysStatus, meterStatus;
  double activeCurrent, passiveCurrent, lineCurrent, lineVoltage,
  activePower, passivePower, importEnergy, exportEnergy;
  tm *timeInfo;
  tempData temps;
  powerData power;
};
// End data structs for queue

// Global to be used in ISR
xformerMonitorData sensorData; 


hw_timer_t *readEICTimer = NULL;

// Variables for tasks
TaskHandle_t taskReadEIC;
TaskHandle_t taskSendData;

QueueHandle_t eicDataQueue;

void readEICData( void * pvParameters );
void sendSensorDataOverMQTT( void * pvParameters );

void IRAM_ATTR ReadData();

struct xformerMonConfigData {
  char *wifiSsid;
  char *wifiPass;
  char *mqttName;
  char *mqttServerHost;
  char *mqttUserName;
  char *mqttPassword;
  uint16_t mqttServerPort;
} monitorConfig;