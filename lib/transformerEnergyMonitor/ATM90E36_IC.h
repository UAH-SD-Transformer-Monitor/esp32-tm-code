#include "transformerEnergyMonitor.h"
#include <ATM90E36.h>
#include <SPI.h>

class ATM90E36_IC : public transformerEnergyMonitor {

private:
    char ctLine;
    int status;
public:
  ATM90E36 *eic;
  ATM90E36_IC(char ctLineLetter);

  double GetLineVoltage();
  double GetActivePower();

};
