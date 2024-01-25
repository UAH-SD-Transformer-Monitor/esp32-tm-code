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

  // configure time
  // TODO: make dst and timezone configurable
  int timezone = 3;
  int dst = 0;
  configTime(timezone * 3600, dst * 0, "pool.ntp.org", "time.nist.gov");

  // Initialize serial and wait for port to open:
  Serial.begin(115200);
  while (!Serial)
  {
  }

  delay(1000);

#ifdef TM_MQTT_SSL
  wifiClient.setCACert(root_ca);
#endif

  // Start the DS18B20 sensors
  monitorTempSensors.cabinet.begin();
  monitorTempSensors.oil.begin();

  setupMQTTClient();
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

  // mqttClient.subscribe("/transformer-mon");
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
  mqttClient.loop();
  delay(10); // <- fixes some issues with WiFi stability

  if (!mqttClient.connected())
  {
    connect();
  }

  mqttClient.publish("xfmormermon", "buffer");

  // publish a message roughly every second.
  if (millis() - lastMillis > 1000)
  {

    lastMillis = millis();
    monitorTempSensors.cabinet.requestTemperatures();
    monitorTempSensors.oil.requestTemperatures();
    // unsigned short volts = getEICSysStatus();
    unsigned short volts = 121.2;
    // float cabinetTemperatureC = monitorTempSensors.cabinet.getTempCByIndex(0);
    // float cabinetTemperatureF = monitorTempSensors.cabinet.getTempFByIndex(0);
    StaticJsonDocument<256> doc;
    // Get the current time and store it in a variable
    time_t now;
    time(&now);
    struct tm* timeinfo = gmtime(&now);

    char timeBuffer[32];
    strftime(timeBuffer, sizeof(timeBuffer), "%FT%TZ", timeinfo);
    // set {"time":"2021-05-04T13:13:04Z"}
    doc["time"] = timeBuffer;
    doc["meterStatus"] = eic.GetLineVoltage();
    doc["voltage"] = volts;
    JsonObject temp = doc.createNestedObject("temps");
    temp["cabinet"] = monitorTempSensors.cabinet.getTempCByIndex(0);
    temp["oil"] = monitorTempSensors.oil.getTempCByIndex(0);

    dataStore->add(doc);

    char mqttBuffer[256];

    serializeJson(doc, mqttBuffer);
    mqttClient.publish("xfomermon/", mqttBuffer);
  }
  Serial.println("Sleeping 10s");
  delay(10000);
}

void setupMQTTClient()
{

  Serial.print("Setting MQTT server: ");
  Serial.println(mqttServer);

  mqttClient.setClient(wifiClient);
  mqttClient.setServer(mqttServer, mqttPort);
}

// this function initializes the energy monitor
// depending on which monitor is used, the function will have call different functions
void setupEnergyMonitor()
{
}