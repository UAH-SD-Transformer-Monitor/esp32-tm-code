#include "transformerEnergyMonitor.h"
#include <energyic_UART.h>

class ATM90E26_IC : public transformerEnergyMonitor {

private:
  ATM90E26_UART *eic;

public:
  ATM90E26_IC();
  double GetLineVoltage();
  double GetLineCurrent();
  double GetActivePower();
  double GetFrequency();
  double GetPowerFactor();
  double GetImportEnergy();
  double GetExportEnergy();
  void InitEnergyIC();
  unsigned short GetSysStatus();
  unsigned short GetMeterStatus();

};