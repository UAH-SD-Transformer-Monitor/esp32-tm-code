Import("env")


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

wifiConfig = monitorConfigFile.get("wifi")
mqttConfig = monitorConfigFile.get("mqtt")

mqttServer = mqttConfig.get("server")
mqttPort = mqttConfig.get("port")
sensorConfig = monitorConfigFile.get("sensor")

transformerLine = sensorConfig['config']['line'] 
sslEnabled = monitorConfigFile["ssl"]['enabled']
if not sslEnabled:
  mqttConfig["port"] = 1883
else:
  mqttConfig["port"] = 8883



if wifiConfig.get("ssid") == None:
    print(f"wifi object variable SSID not defined. Define it in the config.yml file at the root of the project.")
    os._exit(1)

if wifiConfig.get("password") == None:
    print(f"wifi object variable password not defined. Define it in the config.yml file at the root of the project.")
    os._exit(1)
if mqttConfig.get("server") == None:
    print(f"mqtt object variable server not defined. Define it in the config.yml file at the root of the project.")
    os._exit(1)


env.Append(CPPDEFINES=[
  ("TM_WIFI_SSID", wifiConfig.get('ssid')),
  ("TM_WIFI_PASSWD", wifiConfig.get('password') ),
  ("TM_MQTT_PORT", mqttConfig.get('port')),
  ("TM_MQTT_SVR", mqttConfig.get("server")),
  ("TM_MQTT_USER", mqttConfig.get("user")),
  ("TM_MQTT_PASSWD", mqttConfig.get("password")),
  ("TM_CT", transformerLine)
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