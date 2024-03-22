/*
  Transformer monitor project
*/

#include <transformerMonitor.h>

unsigned long lastMillis = 0;

// function that sets up the MQTT client
void setupMQTTClient();

// function to set up and initialize the energy monitor
void setupEnergyMonitor();

// You can use x.509 client certificates if you want
// const char* test_client_key = "";   //to verify the client
// const char* test_client_cert = "";  //to verify the client
void setup()
{
  pinMode(PIN_RED,   OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);
  pinMode(PIN_BLUE,  OUTPUT);

  // set LED to Red - FF0000
  setColor(255, 0, 0);
  delay(10000);

  eicDataQueue = xQueueCreate( 50, sizeof( xformerMonitorData ) );
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

  setupEnergyMonitor();

  // set LED color
  setColor(0, 0, 255);
  xTaskCreatePinnedToCore(
      readEICData, /* Function to implement the task */
      "Read EIC data",     /* Name of the task */
      100000,       /* Stack size in words */
      NULL,        /* Task input parameter */
      0,           /* Priority of the task */
      &taskReadEIC,      /* Task handle. */
      0);          /* Core where the task should run */

  xTaskCreatePinnedToCore(
      sendSensorDataOverMQTT, /* Function to implement the task */
      "Send sensor data over MQTT",     /* Name of the task */
      100000,       /* Stack size in words */
      NULL,        /* Task input parameter */
      0,           /* Priority of the task */
      &taskSendData,      /* Task handle. */
      1);          /* Core where the task should run */
}

void connect()
{

  // connect to the WiFi network
  Serial.print("Attempting to connect to SSID: ");
  Serial.println(ssid);
  WiFi.begin(ssid, wifiPassword);

  // attempt to connect to Wifi network:
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    // wait 1 second for re-trying
    delay(3000);
  }
  Serial.print("Connected to ");
  Serial.println(ssid);

  Serial.print("Username: ");
  Serial.println(mqttUser);

  Serial.print("\nconnecting...");

  // we are using the ESP32's MAC address to provide a unique ID
  client_id += String(WiFi.macAddress());
  Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
  if (mqttClient.connect(client_id.c_str(), mqttUser, mqttPass))
  {
  }
  else
  {
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

  // publish a message roughly every second.
  // Serial.println("Sleeping 10s");
  // delay(10000);
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
#ifdef ATM90E26_EIC

  ATM90E26_IC eic;

#else
  /* Initialize the serial port to host */
  /*
  The ATM90E36 has to be setup via SPI.
   SPI for the ESP32:
    - CLK: 18
    - MISO: 19
    - MOSI: 23
    - CS: 5
  */
  SPI.begin(SCK, MISO, MOSI, SS);
  delay(1000);
  eic.begin();

#endif
}

// readEICData: reads the EIC and inserts data into queue
void readEICData(void *pvParameters)
{
  Serial.print("Task1 running on core ");
  Serial.println(xPortGetCoreID());

  // Attach interrupt for reading data every one second
  readEICTimer = timerBegin(0, 80, true);
  timerAttachInterrupt(readEICTimer, &ReadData, true);
  timerAlarmWrite(readEICTimer, 1000000, true);
  timerAlarmEnable(readEICTimer); // Just Enable

  for (;;)
  {
  }
}

// sendSensorDataOverMQTT: reads the EIC and inserts data into queue
void sendSensorDataOverMQTT(void *pvParameters)
{
  Serial.print("Task1 running on core ");
  Serial.println(xPortGetCoreID());
  StaticJsonDocument<512> mqttJsonData;
  JsonObject tempObj = mqttJsonData.createNestedObject("temps");
  JsonObject powerObj = mqttJsonData.createNestedObject("power");
  JsonObject energyObj = mqttJsonData.createNestedObject("energy");
  xformerMonitorData mqttSensorData;
  int messagesWaiting = uxQueueMessagesWaiting(eicDataQueue);
  int emptySpaces = uxQueueSpacesAvailable(eicDataQueue);
  for (;;)
  {
    if (messagesWaiting > 2)
    {
      xQueueReceive(eicDataQueue, &mqttSensorData, portMAX_DELAY);
      char timeBuffer[32];
      strftime(timeBuffer, sizeof(timeBuffer), "%FT%TZ", mqttSensorData.timeInfo);

      mqttJsonData["deviceId"] = "esp32-random-id";
      mqttJsonData["time"] = timeBuffer;
      mqttJsonData["meterStatus"] = mqttSensorData.meterStatus;
      mqttJsonData["sysStatus"] = mqttSensorData.sysStatus;
      mqttJsonData["current"] = mqttSensorData.lineCurrent;
      mqttJsonData["neutralCurrent"] = mqttSensorData.neutralCurrent;
      mqttJsonData["voltage"] = mqttSensorData.lineCurrent;
      powerObj["active"] = mqttSensorData.power.active;
      powerObj["apparent"] = mqttSensorData.power.apparent;
      powerObj["factor"] = mqttSensorData.power.factor;
      powerObj["reactive"] = mqttSensorData.power.reactive;

      tempObj["oil"] = mqttSensorData.temps.oil;
      tempObj["cabinet"] = mqttSensorData.temps.cabinet;
      
      energyObj["export"] = mqttSensorData.energy.exp;
      energyObj["import"] = mqttSensorData.energy.import;

      char buffer[512];
      size_t n = serializeJson(mqttJsonData, buffer);
      mqttClient.publish("xfmormermon/", buffer, n);
    }

    mqttClient.loop();
    delay(10); // <- fixes some issues with WiFi stability

    if (!mqttClient.connected())
    {
      connect();
    }
    messagesWaiting = uxQueueMessagesWaiting(eicDataQueue);
    emptySpaces = uxQueueMessagesWaiting(eicDataQueue);
  }

}

void IRAM_ATTR ReadData(){
  // Count the number of times the ISR has been entered
  static int timesEnteredISR = 1;
  timesEnteredISR++;

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
    sensorData.temps.oil =  monitorTempSensors.oil.getTempC(oilTempSensorAddr);
  }
  

  // Get the current time and store it in a variable
  time(&now);
  // set {"time":"2021-05-04T13:13:04Z"}
  sensorData.timeInfo = gmtime(&now);

  sensorData.lineVoltage = eic.GetLineVoltage();
  sensorData.neutralCurrent = eic.GetLineCurrentN();
  // sensorData.energy.exp =

  xQueueSend(eicDataQueue, &sensorData, portMAX_DELAY);

}

void setColor(int R, int G, int B) {
  analogWrite(PIN_RED,   R);
  analogWrite(PIN_GREEN, G);
  analogWrite(PIN_BLUE,  B);
}