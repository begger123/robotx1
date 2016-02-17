const int controlPin = 9;
const int dataPin = 3;

void setup() {
  Serial.begin(9600);
  pinMode(controlPin, OUTPUT);
  pinMode(dataPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(dataPin), ultrasoundISR, CHANGE);
}

volatile long startTime = 0;
volatile long endTime = 0;

void loop() {
  requestUltrasound();

  // Sync:
  //   long duration = pulseIn(dataPin, HIGH, 500000);
  //   float dist = ultrasoundTimeToMeters(duration);

  // Async (interrupts-based):
  while (endTime == 0);
  float dist = ultrasoundTimeToMeters(endTime - startTime);

  if (dist > 0) {
    Serial.println("Duration: " + String(dist) + "m.");
  }
}

void requestUltrasound() {
  digitalWrite(controlPin, LOW);
  delayMicroseconds(2);
  digitalWrite(controlPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(controlPin, LOW);
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
