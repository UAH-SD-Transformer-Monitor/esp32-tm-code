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
  // set LED pins
  pinMode(PIN_RED, OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);
  pinMode(PIN_BLUE, OUTPUT);

  // set LED to Red - FF0000
  setLEDColor(255, 0, 0);
  delay(1000);
  Serial.begin(9600);

  // create data queue
  eicDataQueue = xQueueCreate(50, sizeof(xformerMonitorData));
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
  // monitorTempSensors.cabinet.begin();
  // monitorTempSensors.oil.begin();

  // Get each DS18B20 sensors' address
  // monitorTempSensors.oil.getAddress(oilTempSensorAddr, 0);
  // monitorTempSensors.cabinet.getAddress(cabinetTempSensorAddr, 0);

  setupMQTTClient();

  setupEnergyMonitor();

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
    delay(100);
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
  }
  else
  {
    // set LED color
    setLEDColor(255, 0, 0);
    Serial.print("failed with state ");
    Serial.print(mqttClient.state());
    delay(2000);
  }

  Serial.println("\nconnected!");
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
  for (;;)
  {
    if (messagesWaiting > 2)
    {
      xformerMonitorData mqttSensorData;
      xQueueReceive(eicDataQueue, &mqttSensorData, portMAX_DELAY);
      char timeBuffer[32];
      delay(100);
      strftime(timeBuffer, sizeof(timeBuffer), "%FT%TZ", mqttSensorData.timeInfo);
      delay(100);
      if (mqttSensorData.sysStatus == 0xFFFF)
      {
        // Sensor is not working - set LED red
        setLEDColor(255, 0, 0);
      }
      else
      {
        // Sensor is working - set LED green
        setLEDColor(0, 255, 0);
      }

      JsonObject tempObj = mqttJsonData.createNestedObject("temps");
      delay(10);
      JsonObject powerObj = mqttJsonData.createNestedObject("power");
      delay(10);
      JsonObject energyObj = mqttJsonData.createNestedObject("energy");
      delay(10);
      mqttJsonData["deviceId"] = client_id;
      mqttJsonData["time"] = timeBuffer;
      mqttJsonData["meterStatus"] = mqttSensorData.meterStatus;
      mqttJsonData["sysStatus"] = mqttSensorData.sysStatus;
      

      mqttJsonData["current"] = mqttSensorData.lineCurrent;
      mqttJsonData["voltage"] = mqttSensorData.lineVoltage;
      powerObj["active"] = mqttSensorData.power.active;
      powerObj["apparent"] = mqttSensorData.power.apparent;
      powerObj["factor"] = mqttSensorData.power.factor;
      powerObj["reactive"] = mqttSensorData.power.reactive;

      tempObj["oil"] = mqttSensorData.temps.oil;
      tempObj["cabinet"] = mqttSensorData.temps.cabinet;

      energyObj["export"] = mqttSensorData.energy.exp;
      energyObj["import"] = mqttSensorData.energy.import;

      char mqttDataBuffer[512];
      size_t n = serializeJson(mqttJsonData, mqttDataBuffer);
      Serial.print(mqttDataBuffer);
      delay(50);
      mqttClient.publish("xfmormermon/", mqttDataBuffer, n);
      mqttJsonData.clear();
    }
    delay(10); // <- fixes some issues with WiFi stability
    mqttClient.loop();

    if (!mqttClient.connected())
    {
      connect();
    }
    messagesWaiting = uxQueueMessagesWaiting(eicDataQueue);
    emptySpaces = uxQueueMessagesWaiting(eicDataQueue);
  }
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
  // Attach interrupt for reading data every one second
  // readEICTimer = timerBegin(0, 80, true);
  // timerAttachInterrupt(readEICTimer, &ReadData, true);
  // timerAlarmWrite(readEICTimer, 1000000, true);
  // timerAlarmEnable(readEICTimer); // Just Enable
  // Serial.println("App");

  for (;;)
  {
    xformerMonitorData sensorData;
    delay(1000);
    // vTaskDelay(3000);
    static int timesEnteredISR = 1;
    timesEnteredISR++;

  
    // global time variable
    time_t now;
    
    // Read temperature data every 60 seconds
    // Obtain DS18B20 sensor data
    if (timesEnteredISR == 60)
    {
      timesEnteredISR = 0;
      monitorTempSensors.cabinet.requestTemperatures();
      monitorTempSensors.oil.requestTemperatures();
      // get cabinet temp sensor data
      sensorData.temps.cabinet = monitorTempSensors.cabinet.getTempC(cabinetTempSensorAddr);
      // get oil temp sensor data
      sensorData.temps.oil = monitorTempSensors.oil.getTempC(oilTempSensorAddr);
    }

    // Get the current time and store it in a variable
    time(&now);
    // set {"time":"2021-05-04T13:13:04Z"}
    delay(10);
    sensorData.timeInfo = gmtime(&now);
    if (!sensorData.timeInfo)
    {
      Serial.println("Time is null");
    }
      Serial.println(sensorData.timeInfo);
    
    delay(10);
    // in hex
    sensorData.meterStatus = eic.GetMeterStatus();

    delay(10);
    sensorData.sysStatus = eic.GetSysStatus();
    delay(10);

    sensorData.lineVoltage = eic.GetLineVoltage();

    delay(10);
    sensorData.lineCurrent = eic.GetLineCurrent();

    // convert lineVoltage and lineCurrent to floats
    delay(10);
    sensorData.power.factor = eic.GetPowerFactor();

    xQueueSend(eicDataQueue, &sensorData, portMAX_DELAY);
    // Serial.println("hello from ISR");
  }
}

void IRAM_ATTR ReadData()
{
  // // Count the number of times the ISR has been entered
  // static int timesEnteredISR = 1;
  // timesEnteredISR++;

  // // Read temperature data every 60 seconds
  // // Obtain DS18B20 sensor data
  // if (timesEnteredISR == 60)
  // {
  //   // timesEnteredISR = 0;
  //   // monitorTempSensors.cabinet.requestTemperatures();
  //   // monitorTempSensors.oil.requestTemperatures();
  //   // // get cabinet temp sensor data
  //   // sensorData.temps.cabinet = monitorTempSensors.cabinet.getTempC(cabinetTempSensorAddr);
  //   // // get oil temp sensor data
  //   // sensorData.temps.oil =  monitorTempSensors.oil.getTempC(oilTempSensorAddr);
  // }

  // // Get the current time and store it in a variable
  // time(&now);
  // // set {"time":"2021-05-04T13:13:04Z"}
  // sensorData.timeInfo = gmtime(&now);
  // // in hex
  // sensorData.meterStatus = eic.GetMeterStatus();

  // sensorData.sysStatus = eic.GetSysStatus();

  // sensorData.lineVoltage = eic.GetLineVoltage();

  // sensorData.lineCurrent = eic.GetLineCurrent();
  // // convert lineVoltage and lineCurrent to floats

  // sensorData.power.factor = eic.GetPowerFactor();

  // xQueueSend(eicDataQueue, &sensorData, portMAX_DELAY);
  // // Serial.println("hello from ISR");
}

void setLEDColor(int R, int G, int B)
{
  // analogWrite(PIN_RED,   R);
  // analogWrite(PIN_GREEN, G);
  // analogWrite(PIN_BLUE,  B);
}