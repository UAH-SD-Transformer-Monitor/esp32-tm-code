// Turn build flags (Macros) into strings
#define ST(A) #A
#define STR(A) ST(A)


#ifdef TM_WIFI_SSID
   char const *ssid = STR(TM_WIFI_SSID);
#endif
#ifdef TM_WIFI_PASSWD
   char const *wifiPassword = STR(TM_WIFI_PASSWD);
#endif

