# ESP 32 Transformer Monitor

This is the code that will control the ESP32.

## Requirements

We will be using PlatformIO for this project. This allows us to have greater control and allows for swapping boards if needed.

***You will need to change your upload port in `platformio.ini`.***

You will need to download the Silcon Labs Driver here - https://community.silabs.com/s/article/legacy-os-software-and-driver-packages?language=en_US

## Setup

*The Excel sheet has the necessary formulas to calculate the sensor config values. The important ones go in a `include/sensorConfigValues.h`.*

Add the necessary variables to a `config.yml` file before building the project.

*If you have the server certificate at `cert.crt` at build time, this cert will be used instead of fetching it from the server.*

```yaml
wifi:
  # Your WiFi SSID
  ssid: "Your-WiFi-SSID"
  # Your WiFi Password
  password: "Your-Secret-WiFi-Password"
mqtt:
  # Your MQTT server
  server: your.mqtt-broker.tld
  # The port the MQTT server is running on
  port: 8883
  # Your MQTT User
  user: test
  # Your MQTT Password
  password: secure-mqtt-pass
  # A unique ID, usually the name of the transformer
  id: unique-id
```

## Development

Put .cpp files in `src`, .h files in `include`, and any libraries in `lib`.

## Tests

Put tests in `src/tests`. **These are not PlatformIO tests.**

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

## Setup

The PlatformIO environment `prod` is the same as `dev`, but `dev` has serial monitor code.
