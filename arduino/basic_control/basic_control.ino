HardwareSerial *motorL;
HardwareSerial *motorR;

const int batteryStatusPin = /*Analog Pin*/ 0;
const int batteryDiodeVdrop = 0.5;
const int batteryR1 = 1800;
const int batteryR2 = 1200;

String input;
int m;
int s1;  // in mode 4, speed of motor 1.
int s2;  // in mode 4, speed of motor 2.

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
  Serial2.begin(9600);

  motorL = &Serial1;
  motorR = &Serial2;
  SetupServo(motorL);
  SetupServo(motorR);
  Serial.println("Servos reset.");
  
  input = "";
  m = 0;

  delay(150);
  ClearBuffer(motorL);
  ClearBuffer(motorR);
  Serial.println("Serial connection started, waiting for instructions...");
}

/**
 * USER COMMANDS:
 *  - Modes (exit a mode with Enter, ie. C-j):
 *    - 1<...> -- send <...> to motor 1.
 *    - 2<...> -- send <...> to motor 2.
 *    - 3<...> -- send <...> to motors 1 and 2.
 *    - A - enter dual motor control mode:
 *       - + -- increase speed by 10/255.
 *       - - -- decrease speed by 10/255.
 *       - 8/9 -- decrease/increase speed of only motor 1.
 *       - 5/6 -- decrease/increase speed of only motor 2.
 *       - 1   -- set both motors to the speed of the slowest motor.
 *       - 0 -- reset speeds to 0.
 *  - Single commands:
 *    - B -- print battery status.
 *    - S<aaaa><bbbb> -- set speed for both motors, eg. "S+050+030".
 *  - Special commands (work any time):
 *    - X -- stop all motors at once, reset mode.
 */
void loop() {
  if (Serial1.available()) {
    Serial.write(Serial1.read());
  }
  if (Serial2.available()) {
    Serial.write(Serial2.read());
  }

  if (Serial.available()) {
    if (Serial.peek() == 'X') {
      Serial.read();
      Serial.println("STOP");
      SetSpeed(motorL, 0);
      SetSpeed(motorR, 0);
      input = "";
      m = 0;
    } else if (m == 0 && Serial.peek() == 'S') {
      if (Serial.available() >= 1 + 4 + 4) {
        Serial.read();
        int speedL = Read4CharSpeed(&Serial);
        int speedR = Read4CharSpeed(&Serial);
        Serial.println("S" + String(speedL) + "," + String(speedR));
        if (speedL < -255 || speedL > 255 || speedR < -255 || speedR > 255) {
          Serial.println("S: Invalid speed.");
        } else {
          SetRawSpeed(motorL, speedL);
          SetRawSpeed(motorR, speedR);
        }
      }
    } else {
      char ch = Serial.read();
      Serial.println((String("mode: ") + m + ", ch: " + ch).c_str());
      if (m == 0) {
        if (ch == '1') { m = 1; }
        else if (ch == '2') { m = 2; }
        else if (ch == '3') { m = 3; }
        else if (ch == 'A') { m = 4; s1 = 0; s2 = 0; }
        else if (ch == 'B') {
          // Battery status check.
          const float divVoltage = analogRead(batteryStatusPin) * (5.0 / 1024);
          const float realVoltage = divVoltage * (batteryR1 + batteryR2) / batteryR2 + batteryDiodeVdrop;
          Serial.println("Battery: " + String(realVoltage, 2) + "V (measured: " + String(divVoltage, 2) + "V).");
          const int batMin = (3.2 + 0.2 /* safety margin */) * 3;  // 10.2V
          const int batMax = 4.2 * 3;  // 12.6V
          const float perc = (realVoltage - batMin) / (batMax - batMin);
          Serial.println("Battery: " + String(perc * 100, 0) + "%.");
        }
        else Serial.println("Invalid command.");
      } else if (m <= 3) {
        input += ch;
        if (ch == '\n') {
          input += '\r';
          if (m == 1 || m == 3) {
            Serial1.write(input.c_str());
          }
          if (m == 2 || m == 3) {
            Serial2.write(input.c_str());
          }
          input = "";
          m = 0;
        }
      } else if (m == 4) {
        if (ch == '\n') m = 0;
        else {
          if (ch == '+') {
            s1 += 10;
            s2 += 10;
          } else if (ch == '-') {
            s1 -= 10;
            s2 -= 10;
          } else if (ch == '0') {
            s1 = 0;++++
            s2 = 0;
          } else if (ch == '8') {
            s1 -= 10;
          } else if (ch == '9') {
            s1 += 10;
          } else if (ch == '5') {
            s2 -= 10;
          } else if (ch == '6') {
            s2 += 10;
          } else if (ch == '1') {
            s1 = s2 = min(s1, s2);
          }
          SetSpeed(motorL, s1);
          SetSpeed(motorR, s2);
        }
      } else {
        Serial.println("Unrecognized condition.");
      }
    }
  }
}


int Read4CharSpeed(HardwareSerial *s) {
 char s_in[4] = {s->read(), s->read(), s->read(), s->read()};
 return String(s_in).toInt();
}


void ClearBuffer(HardwareSerial *s) {
  while (s->available()) {
    s->read();
  }
}

String ReadLine(HardwareSerial *s) {
  int i = 0;
  String line;
  
  while (!s->available());
  
  while (true) {
    if (s->available()) {
      char ch = s->read();
      if (ch == '\r') continue;
      if (ch == '\n') break;
      line += ch;
    }
  }
  
  return line;
}

void SendCmd(HardwareSerial *s, const char* cmd) {
  ClearBuffer(s);
  s->write(cmd);
  s->write("\n\r");
  s->flush();
  delay(100);
  while (!s->available()) {
    if (ReadLine(s) != (":" + String(cmd))) {
      Serial.println("Executing command failed: " + String(cmd));
    }
  }
}

void SetupServo(HardwareSerial* s) {
  // Speed config:
  SendCmd(s, "S0");
  SendCmd(s, "M250");  // (max. 255)
  SendCmd(s, "D10");

  // Encoder:
  SendCmd(s, "P0");

  // PID:
  SendCmd(s, "A500");  // (D)
  SendCmd(s, "B1200");  // (P)
  SendCmd(s, "C0");  // (I)
}

void SetRawSpeed(HardwareSerial* s, int speed_) {
  String cmd;
  cmd = "S";
  cmd += String(speed_);
  
  ClearBuffer(s);
  s->write(cmd.c_str());
  s->write("\n\r");

  //if (ReadLine(s) != (":" + cmd)) {
  //  Serial.println("Executing command failed: " + cmd);
  //}
}


void SetSpeed(HardwareSerial* s, int speed_) {
  if (s == motorL) {
    speed_ = -speed_;
  }
  SetRawSpeed(s, speed_);
}
