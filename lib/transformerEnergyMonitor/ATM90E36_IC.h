#include "transformerEnergyMonitor.h"
#include <ATM90E36.h>
#include <SPI.h>

class ATM90E36_IC : public transformerEnergyMonitor {

private:
    char ctLine;
    int status;
public:
  ATM90E36 *eic;
  ATM90E36_IC(const char ctLineLetter, ATM90E36 ic);

  void begin();

  double GetLineVoltage();
  double GetActivePower();
  double GetSysStatus();

  double GetLineCurrent();
  double GetLineCurrentN();
  
  double GetApparentPower();
  
  double GetPowerFactor();
  
  double GetVHarm();
  double GetCHarm();


};
