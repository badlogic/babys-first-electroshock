#include <Arduino.h>

void setup() {
  Serial.begin(112500);
  Serial.println("Buttons test");

  pinMode(A0, INPUT);
}

void loop() {
  int value = analogRead(A0);
  Serial.print(">pin2:");
  Serial.print(value);
  Serial.println();
  delay(10);

}
