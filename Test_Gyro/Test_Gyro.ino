#include <Wire.h>
#define ACCEL_INTERVAL_MS 200
#include <Accel.h>

Accel accel(ACCEL_INTERVAL_MS);

void setup() {
    Serial.begin(115200);
  
    pinMode(10, INPUT_PULLUP);
    pinMode(9, INPUT_PULLUP);
      
    accel.begin();
}

void loop() {
    bool didUpdate = accel.Update();
}
