Import("env")

# # Install custom packages from the PyPi registry
# env.Execute("$PYTHONEXE -m pip install python-dotenv")

try:
    import dotenv
except ImportError:
    env.Execute("$PYTHONEXE -m pip install python-dotenv")

dotenv.load_dotenv()

import os

env.Append(CPPDEFINES=[
  ("TM_WIFI_SSID", os.getenv("TRANSFORMER_MON_WIFI_SSID") ),
  ("TM_WIFI_PASSWD", os.getenv("TRANSFORMER_MON_WIFI_PASSWD") )
])