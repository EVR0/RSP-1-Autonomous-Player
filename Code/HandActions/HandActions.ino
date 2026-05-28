#include <Servo.h>

Servo leftMI;
Servo leftPRT;
Servo rightMI;
Servo rightPRT;

const int PIN_LEFT_MI   = 5;
const int PIN_LEFT_PRT  = 6;
const int PIN_RIGHT_MI  = 10;
const int PIN_RIGHT_PRT = 11;

// =============================================
// TUNED VALUES (from HandTuner)
// =============================================
int LMI_STOP        = 91;
int LMI_LOCK        = 100;
int LMI_CURL_SPD    = 180;
int LMI_CURL_DUR    = 1000;
int LMI_UNCURL_SPD  = 0;
int LMI_UNCURL_DUR  = 525;

int LPRT_STOP       = 92;
int LPRT_LOCK       = 85;
int LPRT_CURL_SPD   = 0;
int LPRT_CURL_DUR   = 1000;
int LPRT_UNCURL_SPD = 180;
int LPRT_UNCURL_DUR = 600;

int RMI_STOP        = 88;
int RMI_LOCK        = 86;
int RMI_CURL_SPD    = 0;
int RMI_CURL_DUR    = 1000;
int RMI_UNCURL_SPD  = 180;
int RMI_UNCURL_DUR  = 580;

int RPRT_STOP       = 89;
int RPRT_LOCK       = 83;
int RPRT_CURL_SPD   = 0;
int RPRT_CURL_DUR   = 1000;
int RPRT_UNCURL_SPD = 180;
int RPRT_UNCURL_DUR = 540;
// =============================================

int holdLMI  = 0;
int holdLPRT = 0;
int holdRMI  = 0;
int holdRPRT = 0;

bool curledLMI  = false;
bool curledLPRT = false;
bool curledRMI  = false;
bool curledRPRT = false;

bool          running[4] = {false, false, false, false};
unsigned long stopAt[4]  = {0, 0, 0, 0};
int           stopVal[4] = {0, 0, 0, 0};
bool          wasCurl[4] = {false, false, false, false};

#define M_LMI  0
#define M_LPRT 1
#define M_RMI  2
#define M_RPRT 3

Servo*  motors[4];
int*    holds[4];
bool*   curleds[4];

void setup() {
  Serial.begin(9600);

  holdLMI  = LMI_STOP;
  holdLPRT = LPRT_STOP;
  holdRMI  = RMI_STOP;
  holdRPRT = RPRT_STOP;

  leftMI.attach(PIN_LEFT_MI);
  leftPRT.attach(PIN_LEFT_PRT);
  rightMI.attach(PIN_RIGHT_MI);
  rightPRT.attach(PIN_RIGHT_PRT);

  motors[M_LMI]  = &leftMI;
  motors[M_LPRT] = &leftPRT;
  motors[M_RMI]  = &rightMI;
  motors[M_RPRT] = &rightPRT;

  holds[M_LMI]  = &holdLMI;
  holds[M_LPRT] = &holdLPRT;
  holds[M_RMI]  = &holdRMI;
  holds[M_RPRT] = &holdRPRT;

  curleds[M_LMI]  = &curledLMI;
  curleds[M_LPRT] = &curledLPRT;
  curleds[M_RMI]  = &curledRMI;
  curleds[M_RPRT] = &curledRPRT;

  leftMI.write(holdLMI);
  leftPRT.write(holdLPRT);
  rightMI.write(holdRMI);
  rightPRT.write(holdRPRT);

  delay(500);
  Serial.println("Ready! Commands: LR RR LS RS LP RP LIV RIV HOME STATE");
}

void startMotor(int idx, int speed, int duration, int endVal, bool isCurl) {
  *holds[idx]  = speed;
  motors[idx]->write(speed);
  running[idx] = true;
  stopAt[idx]  = millis() + duration;
  stopVal[idx] = endVal;
  wasCurl[idx] = isCurl;
}

void updateMotors() {
  for (int i = 0; i < 4; i++) {
    if (running[i] && millis() >= stopAt[i]) {
      running[i]  = false;
      *holds[i]   = stopVal[i];
      *curleds[i] = wasCurl[i];
      motors[i]->write(stopVal[i]);
    }
    motors[i]->write(*holds[i]);
  }
}

void curlMotor(int idx) {
  int spd, dur, lockVal;
  switch(idx) {
    case M_LMI:  spd=LMI_CURL_SPD;  dur=LMI_CURL_DUR;  lockVal=LMI_LOCK;  break;
    case M_LPRT: spd=LPRT_CURL_SPD; dur=LPRT_CURL_DUR; lockVal=LPRT_LOCK; break;
    case M_RMI:  spd=RMI_CURL_SPD;  dur=RMI_CURL_DUR;  lockVal=RMI_LOCK;  break;
    case M_RPRT: spd=RPRT_CURL_SPD; dur=RPRT_CURL_DUR; lockVal=RPRT_LOCK; break;
  }
  startMotor(idx, spd, dur, lockVal, true);
}

void uncurlMotor(int idx) {
  int spd, dur, sVal;
  switch(idx) {
    case M_LMI:  spd=LMI_UNCURL_SPD;  dur=LMI_UNCURL_DUR;  sVal=LMI_STOP;  break;
    case M_LPRT: spd=LPRT_UNCURL_SPD; dur=LPRT_UNCURL_DUR; sVal=LPRT_STOP; break;
    case M_RMI:  spd=RMI_UNCURL_SPD;  dur=RMI_UNCURL_DUR;  sVal=RMI_STOP;  break;
    case M_RPRT: spd=RPRT_UNCURL_SPD; dur=RPRT_UNCURL_DUR; sVal=RPRT_STOP; break;
  }
  startMotor(idx, spd, dur, sVal, false);
}

void goHome() {
  Serial.println("Homing...");
  if (curledLMI)  uncurlMotor(M_LMI);
  if (curledLPRT) uncurlMotor(M_LPRT);
  if (curledRMI)  uncurlMotor(M_RMI);
  if (curledRPRT) uncurlMotor(M_RPRT);
}

// ROCK = all fingers curled
void leftRock() {
  Serial.println("Left: ROCK");
  curlMotor(M_LMI);
  curlMotor(M_LPRT);
}
void rightRock() {
  Serial.println("Right: ROCK");
  curlMotor(M_RMI);
  curlMotor(M_RPRT);
}

// SCISSORS = PRT curled, MI open
void leftScissors() {
  Serial.println("Left: SCISSORS");
  curlMotor(M_LPRT);
  if (curledLMI) uncurlMotor(M_LMI);
}
void rightScissors() {
  Serial.println("Right: SCISSORS");
  curlMotor(M_RPRT);
  if (curledRMI) uncurlMotor(M_RMI);
}

// PAPER = all fingers open
void leftPaper() {
  Serial.println("Left: PAPER");
  if (curledLMI)  uncurlMotor(M_LMI);
  if (curledLPRT) uncurlMotor(M_LPRT);
}
void rightPaper() {
  Serial.println("Right: PAPER");
  if (curledRMI)  uncurlMotor(M_RMI);
  if (curledRPRT) uncurlMotor(M_RPRT);
}

// INVALID = MI curled only, PRT open
void leftInvalid() {
  Serial.println("Left: INVALID");
  curlMotor(M_LMI);
  if (curledLPRT) uncurlMotor(M_LPRT);
}
void rightInvalid() {
  Serial.println("Right: INVALID");
  curlMotor(M_RMI);
  if (curledRPRT) uncurlMotor(M_RPRT);
}

void printState() {
  Serial.println("--- State ---");
  Serial.print("L MI:  "); Serial.println(curledLMI  ? "CURLED" : "open");
  Serial.print("L PRT: "); Serial.println(curledLPRT ? "CURLED" : "open");
  Serial.print("R MI:  "); Serial.println(curledRMI  ? "CURLED" : "open");
  Serial.print("R PRT: "); Serial.println(curledRPRT ? "CURLED" : "open");
  Serial.println("-------------");
}

void loop() {
  updateMotors();

  if (Serial.available() > 0) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    cmd.toUpperCase();

    if      (cmd == "LR")   { leftRock(); }
    else if (cmd == "RR")   { rightRock(); }
    else if (cmd == "LS")   { leftScissors(); }
    else if (cmd == "RS")   { rightScissors(); }
    else if (cmd == "LP")   { leftPaper(); }
    else if (cmd == "RP")   { rightPaper(); }
    else if (cmd == "LIV")  { leftInvalid(); }
    else if (cmd == "RIV")  { rightInvalid(); }
    else if (cmd == "HOME") { goHome(); }
    else if (cmd == "STATE"){ printState(); }
    else { Serial.println("Unknown. Use: LR RR LS RS LP RP LIV RIV HOME STATE"); }
  }
}