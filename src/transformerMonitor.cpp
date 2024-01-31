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
  // monitorTempSensors.cabinet.begin();
  // monitorTempSensors.oil.begin();

  setupMQTTClient();

  setupEnergyMonitor();

  xTaskCreatePinnedToCore(
      readEICData, /* Function to implement the task */
      "Task1",     /* Name of the task */
      10000,       /* Stack size in words */
      NULL,        /* Task input parameter */
      0,           /* Priority of the task */
      &taskReadEIC,      /* Task handle. */
      0);          /* Core where the task should run */

  xTaskCreatePinnedToCore(
      sendSensorDataOverMQTT, /* Function to implement the task */
      "Task1",     /* Name of the task */
      10000,       /* Stack size in words */
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


  // mqttClient.publish("xfmormermon", "buffer");

  // // publish a message roughly every second.
  Serial.println("Sleeping 10s");
  delay(10000);
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

  // Attach interrupt
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
  StaticJsonDocument<512> eicJsonData;
  JsonObject temp = eicJsonData.createNestedObject("temps");
  xformerMonitorData sensorData;
  for (;;)
  {
    
    char timeBuffer[32];
    strftime(timeBuffer, sizeof(timeBuffer), "%FT%TZ", sensorData.timeInfo);

    lastMillis = millis();

    mqttClient.loop();
    delay(10); // <- fixes some issues with WiFi stability

    if (!mqttClient.connected())
    {
      connect();
    }
  }
}