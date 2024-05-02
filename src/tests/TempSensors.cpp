/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com  
*********/
#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// GPIO where the DS18B20 is connected to
const int oneWireBus = 4;     
const int oneWireBusOil = 5;     

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(oneWireBus);
OneWire oneWireOil(oneWireBusOil);

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);
DallasTemperature oilSensor(&oneWireOil);

void setup() {
  // Start the Serial Monitor
  Serial.begin(9600);
  // Start the DS18B20 sensor
  sensors.begin();
  oilSensor.begin();
}

void loop() {
    Serial.println("Cabinet: ");
  sensors.requestTemperatures(); 
  float temperatureC = sensors.getTempCByIndex(0);
  float temperatureF = sensors.getTempFByIndex(0);
  Serial.print(temperatureC);
  Serial.println("ºC");
  Serial.print(temperatureF);
  Serial.println("ºF");

  Serial.println("Oil: ");
  oilSensor.requestTemperatures(); 
  temperatureC = oilSensor.getTempCByIndex(0);
  temperatureF = oilSensor.getTempFByIndex(0);
  Serial.print(temperatureC);
  Serial.println("ºC");
  Serial.print(temperatureF);
  Serial.println("ºF");
  delay(5000);
}
