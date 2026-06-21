#include <Arduino.h>

const int pins[] = {
  0,1,2,3,4,5,6,7,8,9,
  10,11,12,13,14,15,16,17,18,21,
  33,34,35,36,37,38,39,40,41,42,
  43,44,45,46,47,48
};

bool lastState[sizeof(pins)/sizeof(pins[0])];

void setup() {
  Serial.begin(115200);

  int count = sizeof(pins)/sizeof(pins[0]);

  for (int i = 0; i < count; i++) {
    pinMode(pins[i], INPUT_PULLUP);
    lastState[i] = digitalRead(pins[i]);
  }

  Serial.println("Ready");
}

void loop() {
  int count = sizeof(pins)/sizeof(pins[0]);

  for (int i = 0; i < count; i++) {
    bool current = digitalRead(pins[i]);

    if (current != lastState[i]) {
      Serial.printf("GPIO %d -> %s\n",
                    pins[i],
                    current ? "HIGH" : "LOW");

      lastState[i] = current;
    }
  }

  delay(10);
}