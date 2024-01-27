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
  this->eic->begin(5, 0x003C, 0x100, 0x100, 0x100,0x100, 0x100, 0x100);
}
  double ATM90E36_IC::GetLineVoltage()
  {
    double lv;
    switch (ctLine)
    {
    case 'A':
      lv = eic->GetLineVoltageA();
    case 'B':
      lv = eic->GetLineVoltageB();
    case 'C':
      lv = eic->GetLineVoltageC();
    break;

    }

    return lv;

  }
  double ATM90E36_IC::GetSysStatus()
  {
    return eic->GetSysStatus0();
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
      ap = this->eic->GetActivePowerC();
    break;
    }
    return ap;
  }
