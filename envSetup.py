Import("env")

# dictionary of environment variable names with the names as the values
# the keys may be the same as the values
# the values must be defined in an .env file or the OS environment
# key (can be anything) : value (must be in the .env file or OS environment)
envVars = {
"WIFI_SSID": "XFORMER_MON_WIFI_SSID",
"WIFI_PASSWD": "XFORMER_MON_WIFI_PASSWD",
"SSL_ENABLED": "XFORMER_MON_USE_SSL",
"MQTT_PORT": "XFORMER_MON_MQTT_PORT",
"MQTT_SERVER": "XFORMER_MON_MQTT_SERVER",
"MQTT_USER": "XFORMER_MON_MQTT_USER",
"MQTT_PASS": "XFORMER_MON_MQTT_PASS",
"MQTT_ID": "XFORMER_MON_MQTT_ID",
"TM_CT": "XFORMER_MON_LINE", # if not set, will default to 'A' in code
}


import socket
import sys
try:
    import yaml
    import OpenSSL
    from OpenSSL import crypto
except ImportError:
    env.Execute("\"$PYTHONEXE\" -m pip install python-dotenv pyOpenSSL crypto pyyaml")
    import yaml
    import OpenSSL
    from OpenSSL import crypto


with open('config.yml', 'r') as file:
  monitorConfigFile = yaml.safe_load(file)
import os
import ssl

wifiConfig = monitorConfigFile["wifi"]
mqttServer = os.getenv(envVars["MQTT_SERVER"])
mqttUser = os.getenv(envVars["MQTT_USER"])
mqttPass = os.getenv(envVars["MQTT_PASS"])
transformerCT = os.getenv(envVars["TM_CT"])
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
  ("TM_MQTT_PORT", mqttPort),
  ("TM_MQTT_SVR", mqttServer),
  ("TM_MQTT_USER", mqttUser),
  ("TM_MQTT_PASSWD", mqttPass),
  ("TM_MQTT_CT", transformerCT)
])

if sslEnabled:
  print("SSL enabled")
  env.Append(CPPDEFINES=[
    ("TM_MQTT_SSL", sslEnabled)
  ])
  certStr = "const char* root_ca= \\\n"
  # get the SSL cert and write it to a file
  cert = ssl.get_server_certificate((mqttServer, mqttPort))
  ctx = OpenSSL.SSL.Context(OpenSSL.SSL.SSLv23_METHOD)
  s = socket.socket()
  connection = OpenSSL.SSL.Connection(ctx, s)
  connection.connect((mqttServer, mqttPort))
  connection.setblocking(1)
  connection.do_handshake()
  chain = connection.get_peer_cert_chain()
  
  for index, cert in enumerate(chain):
    cert_components = dict(cert.get_subject().get_components())
    if(sys.version_info[0] >= 3):
      cn = (cert_components.get(b'CN')).decode('utf-8')
    else:
      cn = cert_components.get('CN')
    print('Centificate {0} - CN: {1}'.format(index, cn))

    try:
      temp_certname = '{0}_{1}.crt'.format("transformerMonitorServerCert", index)
      with open(temp_certname, 'w+') as output_file:
        if(sys.version_info[0] >= 3):
          output_file.write((crypto.dump_certificate
          (crypto.FILETYPE_PEM, cert).decode('utf-8')))
        else:
          output_file.write((crypto.dump_certificate(crypto.FILETYPE_PEM, cert)))
    except IOError:
      print('Exception:  {0}'.format(IOError.strerror))
  # transform the certificate to a multi-line C-string variable
  with open("transformerMonitorServerCert_2.crt", "r") as file:
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