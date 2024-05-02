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
#include <energyic_UART.h>

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

//********* GPIO PINS ************* //

// pins for UART
#define PIN_SerialATM_RX       19   //RX pin for AdaFruit Huzzah32
#define PIN_SerialATM_TX       13   //TX pin for AdaFruit Huzzah32

// GPIO pins where the DS18B20 sensors are connected
const int oilTempBus = 5; // one in the epoxy resin
const int cabinetTempBus = 4;

#define PIN_RED 16   // GPIO16
#define PIN_BLUE 17  // GPIO17
#define PIN_GREEN 21 // GPIO21

//******************************* //

void messageReceived(String &topic, String &payload);

WiFiClientSecure wifiClient;
PubSubClient mqttClient;

HardwareSerial ATMSerial(1);        //1 = just hardware serial number. ESP32 supports 3 hardware serials. UART 0 usually for flashing.
ATM90E26_UART eic(&ATMSerial);


// * we are using the transformer's name to provide a unique ID
char* client_id = "al-xformer-592";

struct tempSensors {
  DallasTemperature oil, cabinet;
};

DeviceAddress oilTempSensorAddr;
DeviceAddress cabinetTempSensorAddr;

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oilTempBusOneWire(oilTempBus);
OneWire cabinetTempBusOneWire(cabinetTempBus);

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature oilTempSensor(&oilTempBusOneWire);
DallasTemperature cabinetTempSensor(&cabinetTempBusOneWire);

// temp sensor objects
tempSensors monitorTempSensors{oilTempSensor, cabinetTempSensor};


// Data structs for queue
struct tempData {
  float cabinet, oil;
};

struct powerData {
  double active, factor;
};

struct xformerMonitorData {
  unsigned short sysStatus, meterStatus;
  double lineCurrent, lineVoltage;
  tm *timeInfo;
  tempData temps;
  powerData power;
};
// End data structs for queue

// Global to be used in ISR

// Variables for tasks and queue
TaskHandle_t taskReadEIC;
TaskHandle_t taskSendData;

QueueHandle_t eicDataQueue;
// End variables for tasks

// Functions for tasks
void readEICData( void * pvParameters );
// End functions for tasks

// Timer variable and function
hw_timer_t *readEICTimer = NULL;
void IRAM_ATTR ReadData();
// End timer variable and function

// LED color Function
// Red   - 0 to 255
// Green - 0 to 255
// Blue  - 0 to 255
void setLEDColor(int R, int G, int B);

// connect to WiFI and MQTT
void connect();

// function that sets up the MQTT client
void setupMQTTClient();

// function to set up and initialize the energy monitor
void setupEnergyMonitor();

struct xformerMonConfigData {
  char *wifiSsid;
  char *wifiPass;
  char *mqttName;
  char *mqttServerHost;
  char *mqttUserName;
  char *mqttPassword;
  uint16_t mqttServerPort;
} monitorConfig;