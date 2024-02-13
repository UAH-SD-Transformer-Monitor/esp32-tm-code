#include <Preferences.h>

struct xformerMonConfigData {
  char *wifiSsid;
  char *wifiPass;
  char *mqttName;
  char *mqttServerHost;
  char *mqttUserName;
  char *mqttPassword;
  uint16_t mqttServerPort;
};


// Turn build flags (Macros) into strings
#define ST(A) #A
#define STR(A) ST(A)

// extract the WiFi SSID from its macro
#ifdef TM_WIFI_SSID
  char *wifiSsid = STR(TM_WIFI_SSID);
#endif

// extract the WiFi Password from its macro
#ifdef TM_WIFI_PASSWD
  char *wifiPassword = STR(TM_WIFI_PASSWD);
#endif

// extract the MQTT Port from its macro
#ifdef TM_MQTT_PORT
  uint16_t mqttPort = (uint16_t) strtoul(STR(TM_MQTT_PORT), NULL, 10);
#endif
// extract the MQTT server hostname from its macro
#ifdef TM_MQTT_SVR
  char *mqttServer = STR(TM_MQTT_SVR);
#endif
// extract the MQTT username from its macro
#ifdef TM_MQTT_USER
  char *mqttUser = STR(TM_MQTT_USER);
#endif
// extract the MQTT password from its macro
#ifdef TM_MQTT_PASSWD
  char *mqttPass = STR(TM_MQTT_PASSWD);
#endif

Preferences preferences;
Preferences mqttPreferences;

xformerMonConfigData monitorConfig;



void setup() {
  Serial.begin(115200);
  Serial.println();

  monitorConfig.wifiSsid = wifiSsid;

  preferences.begin("wifi", false);
  mqttPreferences.begin("mqtt", false);
  preferences.putBytes("config", &monitorConfig, sizeof(monitorConfig));
  preferences.putString("password", wifiPassword);

  Serial.println("Network Credentials Saved using Preferences");

  preferences.end();
}

void loop() {

}