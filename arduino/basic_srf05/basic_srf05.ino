const int controlPin1 = 8;
const int controlPin2 = 9;
// Both HY-SRF05 are connected to the same interrupt-enabled pin, protecting them
// from each other using Schottky diodes.
const int dataPin = 3;

void setup() {
  Serial.begin(9600);
  pinMode(controlPin1, OUTPUT);
  pinMode(controlPin2, OUTPUT);
  pinMode(dataPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(dataPin), ultrasoundISR, CHANGE);
}

volatile long startTime = 0;
volatile long endTime = 0;

void loop() {
  printUltrasound("front", controlPin1);
  delay(100);
  printUltrasound("front down", controlPin2);
  delay(100);
}

void printUltrasound(const String& sensor_name, const int pin) {
  requestUltrasound(pin);

  while (endTime == 0);
  float dist = ultrasoundTimeToMeters(endTime - startTime);
  if (dist > 0) {
    Serial.println("Duration [" + sensor_name + "]: " + String(dist) + "m.");
  }
}

void requestUltrasound(const int pin) {
  digitalWrite(pin, LOW);
  delayMicroseconds(2);
  digitalWrite(pin, HIGH);
  delayMicroseconds(10);
  digitalWrite(pin, LOW);
  delayMicroseconds(10);
  startTime = endTime = 0;
}

void ultrasoundISR() {
  if (endTime == 0) {
    if (digitalRead(dataPin) == HIGH) {
      startTime = micros();
    } else if (startTime != 0) {
      endTime = micros();
    }
  }
}

float ultrasoundTimeToMeters(long duration) {
  float dist = duration * 0.000001 * 340 / 2.0;
  if (dist >= 6.0) dist = 0;
  return dist;
}
