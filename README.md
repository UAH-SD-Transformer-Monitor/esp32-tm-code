# ESP 32 Transformer Monitor

This is the code that will control the ESP32.

## Requirements

We will be using PlatformIO for this project. This will allow us to possibly use its remote test and dev features.

You will need to change your upload port in `platformio.ini`.

## Env variables

Add the env variables to a `.env` file before building it:

```sh
TRANSFORMER_MON_WIFI_SSID=Your-WiFi-SSID
TRANSFORMER_MON_WIFI_PASSWD=Your-WiFi-Password

# Your MQTT server
TRANSFORMER_MON_MQTT_SERVER=xformer.utilitymonitor.io

TRANSFORMER_MON_MQTT_USER=test

TRANSFORMER_MON_MQTT_PASS=secure-mqtt-pass

TRANSFORMER_MON_MQTT_ID=unique-id

TRANSFORMER_MON_MQTT_PORT="8883"

# Must be "enabled" if using SSL!
TRANSFORMER_MON_USE_SSL="enabled"
```

## Development

Put .cpp files in `src`, .h files in `include`, and any libraries in `lib`.

## Tests

Put tests in `src/tests`. These are not PlatformIO tests.

Run and upload tests using PlatformIO's VSCode extension. Or upload them using the command `pio run -t upload -e Test[Name]`. Name is the name of the test to run.

In general, when adding tests make sure to add Test to the beginning of the environment name. Make sure to only include the files for your test. See below for guidelines.

If you add a test, add an env section to `platformio.ini` with a syntax like the following:

```ini
[env:Test[NameOfTest]]
lib_deps = 
    [any libraries you need]
# filter everything else except the files needed for the test
build_src_filter = 
    -<*> 
    -<.git/> 
    -<.svn/> 
    +<tests/[NameOfTest].cpp>
```
