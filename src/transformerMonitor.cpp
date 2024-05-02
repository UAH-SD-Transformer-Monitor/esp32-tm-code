/*
  Transformer monitor project
*/
#include <transformerMonitor.h>

unsigned long lastMillis = 0;

// You can use x.509 client certificates if you want
// const char* test_client_key = "";   //to verify the client
// const char* test_client_cert = "";  //to verify the client
void setup()
{
  Serial.begin(9600);
  delay(1000);
  setupEnergyMonitor();
  // set LED pins
  pinMode(PIN_RED, OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);
  pinMode(PIN_BLUE, OUTPUT);

  // set LED to Red - FF0000
  setLEDColor(255, 0, 0);
  delay(1000);

  // create data queue
  eicDataQueue = xQueueCreate(60, sizeof(xformerMonitorData));
  if (eicDataQueue == 0)
  {
    printf("Failed to create queue= %p\n", eicDataQueue);
  }
  // configure time
  // TODO: make dst and timezone configurable
  int timezone = 3;
  int dst = 0;
  configTime(timezone * 3600, dst * 0, "pool.ntp.org", "time.nist.gov");

  delay(1000);

#ifdef TM_MQTT_SSL
  wifiClient.setCACert(root_ca);
#endif

  // Start the DS18B20 sensors
  monitorTempSensors.cabinet.begin();
  monitorTempSensors.oil.begin();

  // Get each DS18B20 sensors' address
  monitorTempSensors.oil.getAddress(oilTempSensorAddr, 0);
  monitorTempSensors.cabinet.getAddress(cabinetTempSensorAddr, 0);

  setupMQTTClient();


  // set LED color
  setLEDColor(0, 0, 255);

  connect();

  delay(500);
}

// connect connects to the WiFi and restarts it
// TODO: set LED red when not connected, green when connected
void connect()
{
  // connect to the WiFi network
  Serial.print("Attempting to connect to SSID: ");
  Serial.println(ssid);
  WiFi.begin(ssid, wifiPassword);

  // attempt to connect to Wifi network:
  while (WiFi.status() != WL_CONNECTED)
  {
    setLEDColor(255, 0, 0);
    Serial.print(".");
    // wait 1 second for re-trying
    delay(1000);
  }
  Serial.print("Connected to ");
  Serial.println(ssid);

  Serial.print("Username: ");
  Serial.println(mqttUser);

  Serial.print("\nconnecting...");

  // we are using the ESP32's MAC address to provide a unique ID
  Serial.printf("The client %s connects to the public mqtt broker\n", client_id);
  if (mqttClient.connect(client_id, mqttUser, mqttPass))
  {
    setLEDColor(0, 255, 0);
    Serial.println("\nconnected!");
  }
  else
  {
    Serial.println("\nnot connected to MQTT!");
    // set LED color to red
    setLEDColor(255, 0, 0);
    delay(2000);
  }

}

void messageReceived(String &topic, String &payload)
{
  Serial.println("incoming: " + topic + " - " + payload);

  // Note: Do not use the client in the callback to publish, subscribe or
  // unsubscribe as it may cause deadlocks when other things arrive while
  // sending and receiving acknowledgments. Instead, change a global variable,
  // or push to a queue and handle it in the loop after calling `client.loop()`.
}

void loop()
{
  BaseType_t eicTask = xTaskCreatePinnedToCore(
      readEICData,     /* Function to implement the task */
      "Read EIC data", /* Name of the task */
      60000,           /* Stack size in words */
      NULL,            /* Task input parameter */
      0,               /* Priority of the task */
      &taskReadEIC,    /* Task handle. */
      0);              /* Core where the task should run */

  delay(3000);
  StaticJsonDocument<512> mqttJsonData;
  int messagesWaiting = uxQueueMessagesWaiting(eicDataQueue);
  int emptySpaces = uxQueueSpacesAvailable(eicDataQueue);

  // ! Anything after in the for loop will be run indefinitely
  for (;;)
  {
    if (messagesWaiting > 1)
    {
      xformerMonitorData mqttSensorData;
      xQueueReceive(eicDataQueue, &mqttSensorData, portMAX_DELAY);
      char timeBuffer[32];
      delay(100);
      strftime(timeBuffer, sizeof(timeBuffer), "%FT%TZ", mqttSensorData.timeInfo);
      delay(100);
      // if sysStatus is not reporting, or if there is no current
      if (mqttSensorData.sysStatus == 0xFFFF || mqttSensorData.lineCurrent <= 0.05)
      {
        // Sensor is not working - set LED red
        setLEDColor(255, 0, 0);
      }
      else
      {
        // Sensor is working - set LED green
        setLEDColor(0, 255, 0);
      }

      // ******** Parse struct into JSON ************** //

      JsonObject tempObj = mqttJsonData.createNestedObject("temps");
      delay(10);
      JsonObject powerObj = mqttJsonData.createNestedObject("power");
      delay(10);
      
      mqttJsonData["deviceId"] = client_id;
      
      mqttJsonData["time"] = timeBuffer;

      mqttJsonData["meterStatus"] = mqttSensorData.meterStatus;
      mqttJsonData["sysStatus"] = mqttSensorData.sysStatus;
      
      // * Voltage and current

      mqttJsonData["voltage"] = mqttSensorData.lineVoltage;
      
      mqttJsonData["current"] = mqttSensorData.lineCurrent;
      
      // * Power
      powerObj["active"] = mqttSensorData.power.active;
      
      powerObj["factor"] = mqttSensorData.power.factor;

      // * Temperatures
      tempObj["oil"] = mqttSensorData.temps.oil;
      
      tempObj["cabinet"] = mqttSensorData.temps.cabinet;

      char mqttDataBuffer[512];
      size_t n = serializeJson(mqttJsonData, mqttDataBuffer);
      delay(50);
      mqttClient.publish("xformermon/", mqttDataBuffer, n);
    }
    delay(10); // <- fixes some issues with WiFi stability
    mqttClient.loop();
    mqttJsonData.clear();

    if (!mqttClient.connected())
    {
      connect();
    }
    messagesWaiting = uxQueueMessagesWaiting(eicDataQueue);
    emptySpaces = uxQueueMessagesWaiting(eicDataQueue);
    delay(250);
  }
  
  // ! End loop

}

void setupMQTTClient()
{

  Serial.print("Setting MQTT server: ");
  Serial.println(mqttServer);

  mqttClient.setClient(wifiClient);
  mqttClient.setServer(mqttServer, mqttPort);
  mqttClient.setBufferSize(512);
}

// this function initializes the energy monitor
// depending on which monitor is used, the function will have call different functions
void setupEnergyMonitor()
{
  // Buad rate for UART serial
  // Serial config
  // Serial RX pin
  // Serial TX pin
  ATMSerial.begin(9600, SERIAL_8N1, PIN_SerialATM_RX, PIN_SerialATM_TX);
  eic.InitEnergyIC();
  delay(1000);
}

// readEICData: reads the EIC and inserts data into queue
void readEICData(void *pvParameters)
{
  xformerMonitorData sensorData;

  // * get temps on startup
  monitorTempSensors.cabinet.requestTemperatures();
  monitorTempSensors.oil.requestTemperatures();
  // get cabinet temp sensor data
  sensorData.temps.cabinet = monitorTempSensors.cabinet.getTempC(cabinetTempSensorAddr);
  // get oil temp sensor data
  sensorData.temps.oil = monitorTempSensors.oil.getTempC(oilTempSensorAddr);

  for (;;)
  {
    static int secondsPassed = 0;
    // global time variable
    time_t now;
    // Get the current time and store it in a variable
    time(&now);
    // set {"time":"2021-05-04T13:13:04Z"}
    sensorData.timeInfo = gmtime(&now);
    
    // Read temperature data every 60 seconds
    // Obtain DS18B20 sensor data
    if (secondsPassed == 60)
    {
      secondsPassed = 0;
      monitorTempSensors.cabinet.requestTemperatures();
      monitorTempSensors.oil.requestTemperatures();
      // get cabinet temp sensor data
      sensorData.temps.cabinet = monitorTempSensors.cabinet.getTempC(cabinetTempSensorAddr);
      // get oil temp sensor data
      sensorData.temps.oil = monitorTempSensors.oil.getTempC(oilTempSensorAddr);
    }

    
    vTaskDelay(10);
    // in hex
    sensorData.meterStatus = eic.GetMeterStatus();

    vTaskDelay(10);
    sensorData.sysStatus = eic.GetSysStatus();
    
    vTaskDelay(10);

    sensorData.lineVoltage = eic.GetLineVoltage();

    vTaskDelay(10);
    sensorData.lineCurrent = eic.GetLineCurrent();

    vTaskDelay(10);
    sensorData.power.factor = eic.GetPowerFactor();
    vTaskDelay(10);
    sensorData.power.active = eic.GetActivePower();
    vTaskDelay(10);

    // send data to queue
    xQueueSend(eicDataQueue, &sensorData, portMAX_DELAY);
    vTaskDelay(1000);
    secondsPassed++;
  }
}

void setLEDColor(int R, int G, int B)
{
  analogWrite(PIN_RED,   R);
  analogWrite(PIN_GREEN, G);
  analogWrite(PIN_BLUE,  B);
}