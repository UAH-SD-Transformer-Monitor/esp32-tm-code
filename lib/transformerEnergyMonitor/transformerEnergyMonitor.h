/*

*/

#include <Arduino.h>

class transformerEnergyMonitor {

private:



public:

  float volts;
  float monitorStatus;
  double meterStatus;
  unsigned short s_status;

  transformerEnergyMonitor() {};
  ~transformerEnergyMonitor() {};

  double GetLineVoltage() { return volts; }
};

