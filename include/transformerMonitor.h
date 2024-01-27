#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <FreeRTOSConfig.h>
#include <freertos/task.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>



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

StaticJsonDocument<256> dataStore[60];
time_t now;
unsigned short volts;

struct xformerMonitorData {
  unsigned short volts;
  double activeCurrent, passiveCurrent,
   ;
};

hw_timer_t *readEICTimer = NULL;

void IRAM_ATTR ReadData(){

  StaticJsonDocument<256> doc;
  JsonObject temp = doc.createNestedObject("temps");

    // Obtain DS18B20 sensor data
    monitorTempSensors.cabinet.requestTemperatures();
    monitorTempSensors.oil.requestTemperatures();
    
    // Get temp data in Celsius and Fahrenheit
    float cabinetTemperatureC = monitorTempSensors.cabinet.getTempCByIndex(0);
    float cabinetTemperatureF = monitorTempSensors.cabinet.getTempFByIndex(0);
    
    // Get the current time and store it in a variable
    time(&now);
    struct tm* timeinfo = gmtime(&now);

    char timeBuffer[32];
    strftime(timeBuffer, sizeof(timeBuffer), "%FT%TZ", timeinfo);
    // set {"time":"2021-05-04T13:13:04Z"}
    doc["time"] = timeBuffer;
    doc["meterStatus"] = volts;
    doc["voltage"] = eic.GetLineVoltage();
    temp["cabinet"] = monitorTempSensors.cabinet.getTempCByIndex(0);
    temp["oil"] = monitorTempSensors.oil.getTempCByIndex(0);

}

TaskHandle_t Task1;
void readEICData( void * pvParameters );