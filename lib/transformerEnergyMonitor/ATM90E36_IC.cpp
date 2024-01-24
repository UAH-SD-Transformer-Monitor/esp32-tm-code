#include "ATM90E36_IC.h"


  ATM90E36_IC::ATM90E36_IC(char ctLineLetter) {
    
    ATM90E36 ic;
    this->eic = &ic;

    switch (ctLineLetter)
    {
    case 'A':
    case 'B':
    case 'C':
      this->ctLine = ctLineLetter;
    break;
    }
  
    
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
        switch (ctLine)
    {
    case 'A':
      this->meterStatus = eic->GetLineVoltageA();
    case 'B':
      this->meterStatus = eic->GetLineVoltageB();
    case 'C':
      this->meterStatus = eic->GetLineVoltageC();
    break;
    
    default:
      status = 0;
      break;
    }
    this->meterStatus = eic->GetLineVoltageA();
    return meterStatus;
  }

  double ATM90E36_IC::GetActivePower()
  {
    this->meterStatus = eic->GetActivePowerA();
    return meterStatus;
  }
