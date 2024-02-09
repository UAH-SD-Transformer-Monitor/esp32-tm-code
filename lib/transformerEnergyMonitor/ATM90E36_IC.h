#include "transformerEnergyMonitor.h"
#include <ATM90E36.h>
#include <SPI.h>

class ATM90E36_IC : public transformerEnergyMonitor {

private:
    char ctLine;
public:
  ATM90E36 *eic;
  ATM90E36_IC(const char ctLineLetter, ATM90E36 ic);

  void begin();

  double GetLineVoltage();
  double GetActivePower();
  
  double GetMeterStatus0();
  double GetMeterStatus1();

  double GetSysStatus0();
  double GetSysStatus1();
	
  double GetTemperature();

  double GetPhase();

  double GetLineCurrent();
  double GetLineCurrentN();
  
  double GetApparentPower();
  
  double GetPowerFactor();
  
  double GetVHarm();
  double GetCHarm();

  double GetImportEnergy();
	double GetExportEnergy();
};
