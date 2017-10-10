#include <SoftwareSerial.h>

SoftwareSerial ss(12,13);

void setup() {
  // put your setup code here, to run once:
  ss.begin(9600);
  Serial.begin(9600);
  Serial.println("setup done");
  ss.println("setup done");
}

void loop() {
  // put your main code here, to run repeatedly:

}
