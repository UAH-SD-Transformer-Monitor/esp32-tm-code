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
  pinMode(PIN_RED,   OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);
  pinMode(PIN_BLUE,  OUTPUT);

  // set LED to Red - FF0000
  setLEDColor(255, 0, 0);
  delay(1000);
  Serial.begin(9600);

  // create data queue
  eicDataQueue = xQueueCreate( 50, sizeof( xformerMonitorData ) );
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

  // // Get each DS18B20 sensors' address
  // monitorTempSensors.oil.getAddress(oilTempSensorAddr, 0);
  // monitorTempSensors.cabinet.getAddress(cabinetTempSensorAddr, 0);



  setupMQTTClient();

  setupEnergyMonitor();
  

  // set LED color
  setLEDColor(0, 0, 255);
  // BaseType_t test = xTaskCreatePinnedToCore(
  //     sendSensorDataOverMQTT, /* Function to implement the task */
  //     "Send sensor data over MQTT",     /* Name of the task */
  //     50000,       /* Stack size in words */
  //     NULL,        /* Task input parameter */
  //     0,           /* Priority of the task */
  //     &taskSendData,      /* Task handle. */
  //     0);          /* Core where the task should run */
  // Serial.println(test);
  delay(500);
 BaseType_t eicTask = xTaskCreatePinnedToCore(
      readEICData, /* Function to implement the task */
      "Read EIC data",     /* Name of the task */
      20000,       /* Stack size in words */
      NULL,        /* Task input parameter */
      0,           /* Priority of the task */
      &taskReadEIC,      /* Task handle. */
      0);          /* Core where the task should run */
  Serial.println(eicTask);
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
    delay(3000);
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
  Serial.print("Task1 running on core ");
  Serial.println(xPortGetCoreID());
  delay(3000);
  StaticJsonDocument<512> mqttJsonData;
  JsonObject tempObj = mqttJsonData.createNestedObject("temps");
  JsonObject powerObj = mqttJsonData.createNestedObject("power");
  JsonObject energyObj = mqttJsonData.createNestedObject("energy");
  xformerMonitorData mqttSensorData;
  int messagesWaiting = uxQueueMessagesWaiting(eicDataQueue);
  int emptySpaces = uxQueueSpacesAvailable(eicDataQueue);
  for (;;)
  {
    // Serial.println("hello from Sensor data");
    if (messagesWaiting > 2)
    {
      xQueueReceive(eicDataQueue, &mqttSensorData, portMAX_DELAY);
      char timeBuffer[32];
      strftime(timeBuffer, sizeof(timeBuffer), "%FT%TZ", mqttSensorData.timeInfo);

      mqttJsonData["deviceId"] = client_id;
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

      char mqttDataBuffer[512];
      size_t n = serializeJson(mqttJsonData, mqttDataBuffer);
      delay(50);
      mqttClient.publish("xfmormermon/", mqttDataBuffer, n);
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
  Serial.print("Task0 running on core ");
  Serial.println(xPortGetCoreID());
  vTaskDelay(2000);

  // Attach interrupt for reading data every one second
  readEICTimer = timerBegin(0, 80, true);
  timerAttachInterrupt(readEICTimer, &ReadData, true);
  timerAlarmWrite(readEICTimer, 1000000, true);
  timerAlarmEnable(readEICTimer); // Just Enable

  for (;;)
  {
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
  sensorData.lineCurrent = eic.GetLineCurrent();
  sensorData.neutralCurrent = eic.GetLineCurrentN();
  sensorData.power.factor = eic.GetPowerFactor();

  xQueueSend(eicDataQueue, &sensorData, portMAX_DELAY);
  Serial.println("hello from ISR");
}

void setLEDColor(int R, int G, int B) {
  analogWrite(PIN_RED,   R);
  analogWrite(PIN_GREEN, G);
  analogWrite(PIN_BLUE,  B);
}