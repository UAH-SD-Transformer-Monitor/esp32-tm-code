#include <energyic_UART.h>

#define PIN_SerialATM_RX       19   //RX pin, CHANGE IT according to your board
#define PIN_SerialATM_TX       13   //TX pin, CHANGE IT according to your board

HardwareSerial ATMSerial(1);        //1 = just hardware serial number. ESP32 supports 3 hardware serials. UART 0 usually for flashing.


ATM90E26_UART eic(&ATMSerial);

void setup() {
  Serial.begin(115200);
  while (!Serial){}
  
  Serial.println("\nATM90E26 UART Test Started");
  

  #if defined(ESP32)
  //Must begin ATMSerial before IC init supplying baud rate, serial config, and RX TX pins
  ATMSerial.begin(9600, SERIAL_8N1, PIN_SerialATM_RX, PIN_SerialATM_TX);
  #else
  //Must begin ATMSerial before IC init
  ATMSerial.begin(9600);
  #endif
  
  eic.InitEnergyIC();
  delay(1000);
}

void loop() {
  /*Repeatedly fetch some values from the ATM90E26 */
  Serial.print("Sys Status:");
  unsigned short s_status = eic.GetSysStatus();
  if(s_status == 0xFFFF)
  {
	while (1);
  }
  
  Serial.println(eic.GetSysStatus(),HEX);
  delay(10);
  Serial.print("Meter Status:");
  Serial.println(eic.GetMeterStatus(),HEX);
  delay(10);
  Serial.print("Voltage:");
  #if defined(ESP32)
  //ESP32 or maybe another MCU support formatting float/double value
  Serial.printf("%.4f V\n", eic.GetLineVoltage());
  #else
  Serial.println(eic.GetLineVoltage());
  #endif
  delay(10);
  Serial.print("Current:");
  #if defined(ESP32)
  //ESP32 or maybe another MCU support formatting float/double value
  Serial.printf("%.4f A\n", eic.GetLineCurrent());
  #else
  Serial.println(eic.GetLineCurrent());
  #endif
  delay(10);
  Serial.print("Active power:");
  Serial.println(eic.GetActivePower());
  delay(10);
  Serial.print("p.f.:");
  Serial.println(eic.GetPowerFactor());
  delay(1000);
}