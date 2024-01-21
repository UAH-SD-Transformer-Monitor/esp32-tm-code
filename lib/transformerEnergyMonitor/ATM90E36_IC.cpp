#include "ATM90E36_IC.h"


  ATM90E36_IC::ATM90E36_IC() {
    
    ATM90E36 ic;
    this->eic = &ic;
  
    
    /*
  The ATM90E36 has to be setup via SPI.
   SPI for the ESP32:
    - MOSI: 23
    - MISO: 19
    - CLK: 18
    - CS: 5
  */
  // use the ATM90E36 over SPI 
    eic->begin(10, 0x60,0x060,0x060,0x060, 0x060,0x60, 0x60 );

  }

  double ATM90E36_IC::GetLineVoltage()
  {
    this->meterStatus = eic->GetLineVoltageA();
    return meterStatus;
  }
