# ESP 32 Transformer Monitor

This is the code that will control the ESP32.

## Requirements

We will be using PlatformIO for this project. This will allow us to possibly use its remote test and dev features.

You will need to change your upload port in `platformio.ini`.

## Env variables

Add the env variables to a `.env` file before building it:

```sh
TRANSFORMER_MON_SSID=Your-WiFi-SSID
TRANSFORMER_MON_SSID_PASS=Your-WiFi-Password
# currently has no effect
TRANSFORMER_MON_SERVER=Your-Monitor-server
```

## Development

Put .cpp files in `src`, .h files in `include`, and any libraries in `lib`.
