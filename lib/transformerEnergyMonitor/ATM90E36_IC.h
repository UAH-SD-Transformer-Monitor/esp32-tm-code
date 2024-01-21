#include "transformerEnergyMonitor.h"
#include <ATM90E36.h>
#include <SPI.h>

class ATM90E36_IC : public transformerEnergyMonitor {

private:

public:
  ATM90E36 *eic;
  ATM90E36_IC();

  double GetLineVoltage();

};
