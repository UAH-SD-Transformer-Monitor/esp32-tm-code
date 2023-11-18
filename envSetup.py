Import("env")

# dictionary of environment variable names with the names as the values
# the keys may be the same as the values
# the keys must be defined in an .env file or the OS environment
# key (can be anything): value (must be in the .env file or OS environment)
envVars = {
"WIFI_SSID": "TRANSFORMER_MON_WIFI_SSID",
"WIFI_PASSWD": "TRANSFORMER_MON_WIFI_PASSWD",
"SSL_ENABLED": "TRANSFORMER_MON_USE_SSL",
"MQTT_PORT": "TRANSFORMER_MON_MQTT_PORT",
"MQTT_SERVER": "TRANSFORMER_MON_MQTT_SERVER",
"MQTT_USER": "TRANSFORMER_MON_MQTT_USER",
"MQTT_ID": "TRANSFORMER_MON_MQTT_ID",
"MQTT_PASS": "TRANSFORMER_MON_MQTT_PASS",
}

try:
    import dotenv
except ImportError:
    env.Execute("$PYTHONEXE -m pip install python-dotenv")
    import dotenv


dotenv.load_dotenv()

import os
import ssl

mqttServer = os.getenv(envVars["MQTT_SERVER"])
mqttPass = os.getenv(envVars["MQTT_PASS"])
sslEnabled = os.getenv(envVars["SSL_ENABLED"]) == "enabled"
mqttPort = 1883
if not sslEnabled:
  os.environ[envVars["MQTT_PORT"]] = "1883"
else:
  os.environ[envVars["MQTT_PORT"]] = "8883"
  mqttPort = 8883
   
   

wifiSSID = os.getenv(envVars["WIFI_SSID"])
wifiPasswd = os.getenv(envVars["WIFI_PASSWD"])



if wifiSSID == None:
    print(f"Environment variable {envVars['WIFI_SSID']} variable not defined. Define it in a .env file at the root of the project.")
    os._exit(1)

if wifiPasswd == None:
    print(f"Environment variable {envVars['WIFI_PASSWD']} variable not defined. Define it in a .env file at the root of the project.")
    os._exit(1)
if mqttServer == None:
    print(f"Environment variable {envVars['MQTT_SERVER']} variable not defined. Define it in a .env file at the root of the project.")
    os._exit(1)


env.Append(CPPDEFINES=[
  ("TM_WIFI_SSID", wifiSSID ),
  ("TM_WIFI_PASSWD", wifiPasswd ),
  ("TM_MQTT_SERVER", mqttServer),
  ("TM_MQTT_PORT", mqttPort),
  ("TM_MQTT_SVR", mqttServer),
  ("TM_MQTT_SLL", sslEnabled),
  ("TM_MQTT_PASSWD", mqttPass)
])

if sslEnabled:
  env.Append(CPPDEFINES=[
    ("TM_MQTT_SLL", sslEnabled)
  ])
  certStr = "const char* root_ca= \\\n"
  # get the SSL cert and write it to a file
  cert = ssl.get_server_certificate((mqttServer, mqttPort))
  w = open("transformer_monitor_cert.pem", "w")
  w.writelines(cert.splitlines(True))
  w.close()

  # transform the certificate to a multi-line C-string variable
  with open("transformer_monitor_cert.pem", "r") as file:
    for item in file:
      for i in item.splitlines():
        if i.startswith("-----END CERTIFICATE-----"):
          certStr += " \"" + str(i) + "\\n\""
        else:
          certStr += " \"" + str(i) + "\\n\"\\\n"

  certHeaderFile = open("include/transformerMonitorServerCert.h", "w")
  certHeaderFile.write(certStr)
  certHeaderFile.close()
  certHeaderFile = open("include/transformerMonitorServerCert.h", "a")
  certHeaderFile.write(";")
  certHeaderFile.close()
else:
  if os.path.exists("include/transformerMonitorServerCert.h"):
    os.remove("include/transformerMonitorServerCert.h")