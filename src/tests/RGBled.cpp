#include <Arduino.h>

#define PIN_RED 32   // GPIO32
#define PIN_BLUE 27  // GPIO27
#define PIN_GREEN 33 // GPIO33

void setup()
{
    pinMode(PIN_RED, OUTPUT);
    pinMode(PIN_GREEN, OUTPUT);
    pinMode(PIN_BLUE, OUTPUT);

    analogWrite(PIN_RED, 0);
    analogWrite(PIN_GREEN, 255);
    analogWrite(PIN_BLUE, 0);
}

void loop(){

}