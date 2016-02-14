const int controlPin = 9;
const int dataPin = 8;

void setup() {
  Serial.begin(9600);
  pinMode(controlPin, OUTPUT);
  pinMode(dataPin, INPUT);
}

void loop() {
  digitalWrite(controlPin, LOW);
  delayMicroseconds(2);
  digitalWrite(controlPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(controlPin, LOW);
  long duration = pulseIn(dataPin, HIGH, 300000);
  float dist = duration * 0.000001 * 340 / 2.0;
  if (dist > 0 && dist < 6.0) {
    Serial.println("Duration: " + String(dist) + "m.");
  }
}
