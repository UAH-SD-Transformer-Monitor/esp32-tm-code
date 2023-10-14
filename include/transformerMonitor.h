#include <WiFiClientSecure.h>
#include <SPI.h>
// Turn build flags (Macros) into strings
#define ST(A) #A
#define STR(A) ST(A)

static const uint8_t SS    = 7;
static const uint8_t MOSI  = 6;
static const uint8_t MISO  = 5;
static const uint8_t SCK   = 4;

#ifdef TM_WIFI_SSID
   char const *ssid = STR(TM_WIFI_SSID);
#endif
#ifdef TM_WIFI_PASSWD
   char const *wifiPassword = STR(TM_WIFI_PASSWD);
#endif

