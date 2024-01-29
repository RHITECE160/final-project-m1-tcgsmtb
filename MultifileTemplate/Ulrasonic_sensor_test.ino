#include <Ultrasonic.h>

const int triggerPin = 26;
const int echoPin = 26;

Ultrasonic mySonar(triggerPin, echoPin);

long distance; // data type long is larger than an integer (int)

void setup() {
Serial.begin(57600); 
pinMode(triggerPin, OUTPUT);
pinMode(echoPin, INPUT);
}

void loop() {
delay(1000);
distance = mySonar.read(CM); // use the Ultrasonic function read to get distance
Serial.print(distance);
Serial.println(" cm");
}