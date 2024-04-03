#include "ATM90E36_IC.h"


  ATM90E36_IC::ATM90E36_IC(const char ctLineLetter, ATM90E36 ic) {
    
    this->eic = &ic;

    switch (ctLineLetter)
    {
    case 'A':
    case 'B':
    case 'C':
      this->ctLine = ctLineLetter;
    break;
    }

  }

void ATM90E36_IC::begin(){
  /*
    CS pin - 5 for ESP32
    Line Frequency - 60 Hz for NA - 4485
    PGA Gain - 
    Current gain - 
    Note: values are adjusted from https://github.com/DitroniX/IPEM-IoT-Power-Energy-Monitor/blob/main/Code/IPEM_1_Test_Code_ATM90E32_ATM90E36/include/IPEM_Hardware.h
  */
 unsigned short PgaGain = 0b0101010101111111;
 unsigned short VoltageGain = 19700; 
  this->eic->begin(5, 4485, PgaGain, VoltageGain, 0x1000,0x1000, 0x1000, 0x1000);
}
  double ATM90E36_IC::GetLineVoltage()
  {
    double lv;
    switch (ctLine)
    {
    case 'A':
      lv = eic->GetLineVoltageA();
    break;
    case 'B':
      lv = eic->GetLineVoltageB();
    break;
    case 'C':
      lv = eic->GetLineVoltageC();
    break;

    }

    return lv;

  }

  double ATM90E36_IC::GetActivePower()
  {
    double ap;
    switch (ctLine)
    {
    case 'A':
      ap = eic->GetActivePowerA();
    break;
    case 'B':
      ap = eic->GetActivePowerB();
    break;
    case 'C':
      ap = eic->GetActivePowerC();
    break;
    }
    return ap;
  }

  double ATM90E36_IC::GetVHarm()
  {
    double vh;
    switch (ctLine)
    {
    case 'A':
      vh = eic->GetVHarmA();
    break;
    case 'B':
      vh = eic->GetVHarmB();
    break;
    case 'C':
      vh = eic->GetVHarmC();
    break;
    }
    return vh;
  }

  double ATM90E36_IC::GetCHarm()
  {
    double ch;
    switch (ctLine)
    {
    case 'A':
      ch = eic->GetVHarmA();
    break;
    case 'B':
      ch = eic->GetVHarmB();
    break;
    case 'C':
      ch = eic->GetVHarmC();
    break;
    }
    return ch;
  }
  
  double ATM90E36_IC::GetPowerFactor()
  {
    double pf;
    switch (ctLine)
    {
    case 'A':
      pf = eic->GetPowerFactorA();
    break;
    case 'B':
      pf = eic->GetPowerFactorB();
    break;
    case 'C':
      pf = eic->GetPowerFactorC();
    break;
    }
    return pf;
  }

  double ATM90E36_IC::GetApparentPower()
  {
    return eic->GetTotalApparentPower();
  }

  double ATM90E36_IC::GetPhase()
  {
    double p;
    switch (ctLine)
    {
    case 'A':
      p = eic->GetPhaseA();
    break;
    case 'B':
      p = eic->GetPhaseB();
    break;
    case 'C':
      p = eic->GetPhaseC();
    break;
    }
    return p;
  }
  double ATM90E36_IC::GetLineCurrent()
  {
    double i;
    switch (ctLine)
    {
    case 'A':
      i = eic->GetLineCurrentA();
    break;
    case 'B':
      i = eic->GetLineCurrentB();
    break;
    case 'C':
      i = eic->GetLineCurrentC();
    break;
    }
    return i;
  }

  double ATM90E36_IC::GetLineCurrentN()
  {
    return eic->GetLineCurrentN();
  }

  double ATM90E36_IC::GetTemperature()
  {
    return eic->GetTemperature();
  }

  double ATM90E36_IC::GetMeterStatus0()
  {
    return eic->GetMeterStatus0();
  }
  
  double ATM90E36_IC::GetMeterStatus1()
  {
    return eic->GetMeterStatus1();
  }

  double ATM90E36_IC::GetSysStatus0()
  {
    return eic->GetSysStatus0();
  }

  double ATM90E36_IC::GetSysStatus1()
  {
    return eic->GetSysStatus1();
  }

  double ATM90E36_IC::GetImportEnergy()
  {
    return eic->GetImportEnergy();
  }
  
  double ATM90E36_IC::GetExportEnergy()
  {
    return eic->GetExportEnergy();
  }
