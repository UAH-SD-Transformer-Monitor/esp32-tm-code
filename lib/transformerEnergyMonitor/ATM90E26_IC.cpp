#include "ATM90E26_IC.h"

ATM90E26_IC::ATM90E26_IC()
{
  
  int PIN_SerialATM_RX = 16; // RX pin, CHANGE IT according to your board
  int PIN_SerialATM_TX = 17; // TX pin, CHANGE IT according to your board

  HardwareSerial ATMSerial(1); // 1 = just hardware serial number. ESP32 supports 3 hardware serials. UART 0 usually for flashing.
  ATM90E26_UART ic(&ATMSerial);
  // Must begin ATMSerial before IC init supplying baud rate, serial config, and RX TX pins
  ATMSerial.begin(9600, SERIAL_8N1, PIN_SerialATM_RX, PIN_SerialATM_TX);

  ic.InitEnergyIC();
  delay(1000);
  s_status = ic.GetSysStatus();

  this->eic = &ic;
}

double ATM90E26_IC::GetLineVoltage()
{
  this->meterStatus = eic->GetLineVoltage();
  return meterStatus;
}

  double  ATM90E26_IC::GetLineCurrent() {
    return eic->GetLineCurrent();
  }

unsigned short ATM90E26_IC::GetMeterStatus() {
  meterStatus =  this->eic->GetMeterStatus();
  return this->eic->GetMeterStatus();
}