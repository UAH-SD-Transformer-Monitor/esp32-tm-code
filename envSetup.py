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
}

try:
    import dotenv
except ImportError:
    env.Execute("$PYTHONEXE -m pip install python-dotenv")
    import dotenv


dotenv.load_dotenv()

import os
import ssl

sslEnabled = os.getenv(envVars["SSL_ENABLED"])
mqttPort = 1083
if sslEnabled != "enabled" or sslEnabled == None:
  os.environ[envVars["MQTT_PORT"]] = "1083"
   

wifiSSID = os.getenv(envVars["WIFI_SSID"])
wifiPasswd = os.getenv(envVars["WIFI_PASSWD"])

if wifiSSID == None:
    print(f"Environment variable {envVars['WIFI_SSID']} variable not defined. Define it in a .env file at the root of the project.")
    os._exit(1)

if wifiPasswd == None:
    print(f"Environment variable {envVars['WIFI_PASSWD']} variable not defined. Define it in a .env file at the root of the project.")
    os._exit(1)


env.Append(CPPDEFINES=[
  ("TM_WIFI_SSID", wifiSSID ),
  ("TM_WIFI_PASSWD", wifiPasswd )
])


certStr = "const char* root_ca= \\\n"
# get the SSL cert and write it to a file
cert = ssl.get_server_certificate(('public.cloud.shiftr.io', 8883))
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