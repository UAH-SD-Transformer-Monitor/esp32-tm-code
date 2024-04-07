/* ATM90E36 Energy Monitor Demo Application

   The MIT License (MIT)

  Copyright (c) 2016 whatnick and Ryzee

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#include <SPI.h>
#define DEBUG_SERIAL
#include <ATM90E36.h>

ATM90E36 eic;

void setup() {
  /* Initialize the serial port to host */
  /*
  The ATM90E36 has to be setup via SPI.
   SPI for the ESP32:
    - CLK: 33
    - MISO: 19
    - MOSI: 23
    - CS: 5
  */
  delay(2000);
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB
  }

  Serial.println("Start ATM90E36");
  /*Initialise the ATM90E36 + SPI port */
  // ss pin is the first parameter
  SPI.begin(SCK, MISO, MOSI, SS);
  delay(1000);
  /*
    CS pin - 33 for ESP32
    Line Frequency - 60 Hz for NA - 5509 - see MMode0 section (4.2.3) in data sheet for ATM90E36
    PGA Gain - 
    Current gain - 
    Note: values are adjusted from https://github.com/DitroniX/IPEM-IoT-Power-Energy-Monitor/blob/main/Code/IPEM_1_Test_Code_ATM90E32_ATM90E36/include/IPEM_Hardware.h
  */
  unsigned short PgaGain = 0x5555; 
  unsigned short frequency = 0b0001010110000101;
  unsigned short VoltageGain = 19800;
  eic.begin(SS, frequency, PgaGain, VoltageGain, 0x1000,0x1000, 0x1000, 0x1000);
  delay(1000);
}

void loop() {
  
  /*Repeatedly fetch some values from the ATM90E36 */
  double voltageA,freq,voltageB,voltageC,currentA,currentB,currentC,power,pf,new_current,new_power;
  int sys0=eic.GetSysStatus0();
  int sys1=eic.GetSysStatus1();
  int en0=eic.GetMeterStatus0();
  int en1=eic.GetMeterStatus1();
  Serial.println("S0:0x"+String(sys0,HEX));
  delay(10);
  Serial.println("S1:0x"+String(sys1,HEX));
  delay(10);
  Serial.println("E0:0x"+String(en0,HEX));
  delay(10);
  Serial.println("E1:0x"+String(en1,HEX));
  voltageA=eic.GetLineVoltageA();
  Serial.println("VA:"+String(voltageA)+"V");
  voltageB=eic.GetLineVoltageB();
  Serial.println("VB:"+String(voltageB)+"V");
  voltageC=eic.GetLineVoltageC();
  Serial.println("VC:"+String(voltageC)+"V");
  delay(10);
  currentA = eic.GetLineCurrentA();
  Serial.println("IA:"+String(currentA)+"A");
  currentB = eic.GetLineCurrentB();
  Serial.println("IB:"+String(currentB)+"A");
  currentC = eic.GetLineCurrentC();
  Serial.println("IC:"+String(currentC)+"A");
  delay(10);
  freq=eic.GetFrequency();
  delay(10);
  Serial.println("f"+String(freq)+"Hz");
  Serial.println("Waiting 1s");
  delay(1000);
}