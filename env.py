#!/bin/python3
import os
import subprocess
import platform
# for dist in pkg_resources.working_set:
# Install missed package
try:
    import dotenv
except ImportError:

    PLATFORMIO_CORE_DIR = os.getenv("PLATFORMIO_CORE_DIR")
    if platform.system() == "Windows":
        # Install missed packages from the PyPi registry
        windowsPath = os.path.join(PLATFORMIO_CORE_DIR, "penv", "Scripts", "pip3.exe")
        subprocess.call(windowsPath + " -m pip3 install --upgrade python-dotenv", shell=True)
    else:
        unixPath = os.path.join(PLATFORMIO_CORE_DIR, "penv", "bin", "python")
        subprocess.call("pip3 install --upgrade python-dotenv", shell=True)
    import dotenv


# TODO: Platform-independent install methods

PLATFORMIO_CORE_DIR = os.getenv("PLATFORMIO_CORE_DIR", "~/.platformio")
unixPath = os.path.join(PLATFORMIO_CORE_DIR, "penv", "bin", "python3")
subprocess.call(unixPath + " -m pip install --upgrade python-dotenv", shell=True)

dotenv.load_dotenv()


ssid = os.getenv("TRANSFORMER_MON_WIFI_SSID")
print("'-D TM_WIFI_SSID=\"%s\"'" % ssid)

wifiPasswd = os.getenv("TRANSFORMER_MON_WIFI_PASSWD")
print("'-D TM_WIFI_PASSWD=\"%s\"'" % wifiPasswd)