Import("env")

# dictionary of environment variable names with the names as the values
# the keys may be the same as the values
# the keys must be defined in an .env file or the OS environment
# key (can be anything): value (must be in the .env file or OS environment)
envVars = {
"WIFI_SSID": "TRANSFORMER_MON_WIFI_SSID",
"WIFI_PASSWD": "TRANSFORMER_MON_WIFI_PASSWD",
}

try:
    import dotenv
except ImportError:
    env.Execute("$PYTHONEXE -m pip install python-dotenv")
    import dotenv


dotenv.load_dotenv()

import os

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