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

const int RESET_EXTRA_MS = 500;
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

// =============================================
// GAME STATE
// =============================================
enum GamePhase {
  PHASE_IDLE,
  PHASE_RESET_CURL,
  PHASE_RESET_UNCURL,
  PHASE_WAITING_OPP,
  PHASE_WAITING_RESULT
};

GamePhase gamePhase = PHASE_IDLE;

char myGesture1 = ' ';
char myGesture2 = ' ';
char myKept     = ' ';
char oppHand1   = ' ';
char oppHand2   = ' ';

// =============================================
// STRATEGY
// =============================================
#define PATTERN_LEN 5
int  comboCount[3]   = {0,0,0};
int  comboKeep[3][2] = {{0,0},{0,0},{0,0}};
int  totalRounds     = 0;
int  wins = 0, losses = 0, ties = 0;
char oppKeepHistory[PATTERN_LEN];
int  historyCount    = 0;

int getComboIndex(char a, char b) {
  if ((a=='R'&&b=='P')||(a=='P'&&b=='R')) return 0;
  if ((a=='R'&&b=='S')||(a=='S'&&b=='R')) return 1;
  if ((a=='P'&&b=='S')||(a=='S'&&b=='P')) return 2;
  return -1;
}

int scoreVs(char a, char b) {
  if (a==b) return 0;
  if ((a=='R'&&b=='S')||(a=='S'&&b=='P')||(a=='P'&&b=='R')) return 1;
  return -1;
}

char determineResult(char mine, char opp) {
  int s=scoreVs(mine,opp);
  if (s==1) return 'W';
  if (s==-1) return 'L';
  return 'T';
}

void pickNashCombo(char &g1, char &g2) {
  int r=random(5);
  if (r<2)      { g1='R'; g2='P'; }
  else if (r<3) { g1='R'; g2='S'; }
  else          { g1='P'; g2='S'; }
}

void predictAndCounter(int ci, char &g1, char &g2) {
  char likely;
  if      (ci==0) likely=(comboKeep[0][0]>=comboKeep[0][1])?'R':'P';
  else if (ci==1) likely=(comboKeep[1][0]>=comboKeep[1][1])?'R':'S';
  else            likely=(comboKeep[2][0]>=comboKeep[2][1])?'P':'S';
  if      (likely=='R') { g1='P'; g2='S'; }
  else if (likely=='P') { g1='R'; g2='S'; }
  else                  { g1='R'; g2='P'; }
}

void pickStage1(char &g1, char &g2) {
  if (totalRounds<6) {
    pickNashCombo(g1,g2);
    Serial.println(F("  Nash"));
    return;
  }
  int mf=0;
  for (int i=1;i<3;i++) if (comboCount[i]>comboCount[mf]) mf=i;
  if ((float)comboCount[mf]/totalRounds>0.5) {
    char cg1,cg2;
    predictAndCounter(mf,cg1,cg2);
    if (getComboIndex(cg1,cg2)>=0) {
      g1=cg1; g2=cg2;
      Serial.println(F("  Counter"));
      return;
    }
  }
  pickNashCombo(g1,g2);
  Serial.println(F("  Nash"));
}

int pickStage2(char g1, char g2, char o1, char o2) {
  int s1=scoreVs(g1,o1)+scoreVs(g1,o2);
  int s2=scoreVs(g2,o1)+scoreVs(g2,o2);
  return (s1>=s2)?1:2;
}

// =============================================
// MOTOR CONTROL CORE
// =============================================
void startMotor(int idx, int speed, int duration, int endVal, bool isCurl) {
  *holds[idx]  = speed;
  motors[idx]->write(speed);
  running[idx] = true;
  stopAt[idx]  = millis()+duration;
  stopVal[idx] = endVal;
  wasCurl[idx] = isCurl;
}

void updateMotors() {
  for (int i=0; i<4; i++) {
    if (running[i] && millis()>=stopAt[i]) {
      running[i]  = false;
      *holds[i]   = stopVal[i];
      *curleds[i] = wasCurl[i];
      motors[i]->write(stopVal[i]);
    }
    motors[i]->write(*holds[i]);
  }
}

bool allMotorsDone() {
  for (int i=0; i<4; i++) if (running[i]) return false;
  return true;
}

void curlMotor(int idx) {
  int s,d,lv;
  switch(idx) {
    case M_LMI:  s=LMI_CURL_SPD;  d=LMI_CURL_DUR;  lv=LMI_LOCK;  break;
    case M_LPRT: s=LPRT_CURL_SPD; d=LPRT_CURL_DUR; lv=LPRT_LOCK; break;
    case M_RMI:  s=RMI_CURL_SPD;  d=RMI_CURL_DUR;  lv=RMI_LOCK;  break;
    case M_RPRT: s=RPRT_CURL_SPD; d=RPRT_CURL_DUR; lv=RPRT_LOCK; break;
  }
  startMotor(idx,s,d,lv,true);
}

void curlMotorReset(int idx) {
  int s,d,lv;
  switch(idx) {
    case M_LMI:  s=LMI_CURL_SPD;  d=LMI_CURL_DUR  +RESET_EXTRA_MS; lv=LMI_LOCK;  break;
    case M_LPRT: s=LPRT_CURL_SPD; d=LPRT_CURL_DUR +RESET_EXTRA_MS; lv=LPRT_LOCK; break;
    case M_RMI:  s=RMI_CURL_SPD;  d=RMI_CURL_DUR  +RESET_EXTRA_MS; lv=RMI_LOCK;  break;
    case M_RPRT: s=RPRT_CURL_SPD; d=RPRT_CURL_DUR +RESET_EXTRA_MS; lv=RPRT_LOCK; break;
  }
  startMotor(idx,s,d,lv,true);
}

void uncurlMotor(int idx) {
  int s,d,sv;
  switch(idx) {
    case M_LMI:  s=LMI_UNCURL_SPD;  d=LMI_UNCURL_DUR;  sv=LMI_STOP;  break;
    case M_LPRT: s=LPRT_UNCURL_SPD; d=LPRT_UNCURL_DUR; sv=LPRT_STOP; break;
    case M_RMI:  s=RMI_UNCURL_SPD;  d=RMI_UNCURL_DUR;  sv=RMI_STOP;  break;
    case M_RPRT: s=RPRT_UNCURL_SPD; d=RPRT_UNCURL_DUR; sv=RPRT_STOP; break;
  }
  startMotor(idx,s,d,sv,false);
}

// =============================================
// GESTURE FUNCTIONS
// =============================================
void goHome() {
  if (curledLMI)  uncurlMotor(M_LMI);
  if (curledLPRT) uncurlMotor(M_LPRT);
  if (curledRMI)  uncurlMotor(M_RMI);
  if (curledRPRT) uncurlMotor(M_RPRT);
}

void leftRock()      { curlMotor(M_LMI);  curlMotor(M_LPRT); }
void rightRock()     { curlMotor(M_RMI);  curlMotor(M_RPRT); }
void leftScissors()  { curlMotor(M_LPRT); if(curledLMI) uncurlMotor(M_LMI); }
void rightScissors() { curlMotor(M_RPRT); if(curledRMI) uncurlMotor(M_RMI); }
void leftPaper()     { if(curledLMI) uncurlMotor(M_LMI); if(curledLPRT) uncurlMotor(M_LPRT); }
void rightPaper()    { if(curledRMI) uncurlMotor(M_RMI); if(curledRPRT) uncurlMotor(M_RPRT); }
void leftInvalid()   { curlMotor(M_LMI);  if(curledLPRT) uncurlMotor(M_LPRT); }
void rightInvalid()  { curlMotor(M_RMI);  if(curledRPRT) uncurlMotor(M_RPRT); }

void showGesture(char g, bool isLeft) {
  if (isLeft) {
    if      (g=='R') leftRock();
    else if (g=='S') leftScissors();
    else             leftPaper();
  } else {
    if      (g=='R') rightRock();
    else if (g=='S') rightScissors();
    else             rightPaper();
  }
}

void invalidateHand(bool isLeft) {
  if (isLeft) leftInvalid();
  else        rightInvalid();
}

void printState() {
  Serial.println(F("--- State ---"));
  Serial.print(F("L MI:")); Serial.print(curledLMI ?"CURL":"open");
  Serial.print(F(" PRT:")); Serial.println(curledLPRT?"CURL":"open");
  Serial.print(F("R MI:")); Serial.print(curledRMI ?"CURL":"open");
  Serial.print(F(" PRT:")); Serial.println(curledRPRT?"CURL":"open");
}

// =============================================
// GAME FLOW
// =============================================
void startRound() {
  Serial.println(F(""));
  Serial.println(F("===================="));
  Serial.print(F("ROUND ")); Serial.print(totalRounds+1);
  Serial.print(F(" W:")); Serial.print(wins);
  Serial.print(F(" L:")); Serial.print(losses);
  Serial.print(F(" T:")); Serial.println(ties);
  Serial.println(F("===================="));
  pickStage1(myGesture1,myGesture2);
  Serial.print(F("  L:")); Serial.print(myGesture1);
  Serial.print(F(" R:")); Serial.println(myGesture2);
  Serial.println(F("  Resetting..."));

  // Stagger motor starts 200ms apart to avoid current spike
  curlMotorReset(M_LMI);  delay(500);
  curlMotorReset(M_LPRT); delay(500);
  curlMotorReset(M_RMI);  delay(500);
  curlMotorReset(M_RPRT);

  gamePhase=PHASE_RESET_CURL;
}

void updateGamePhase() {
  if (gamePhase==PHASE_RESET_CURL && allMotorsDone()) {
    uncurlMotor(M_LMI); uncurlMotor(M_LPRT);
    uncurlMotor(M_RMI); uncurlMotor(M_RPRT);
    gamePhase=PHASE_RESET_UNCURL;
  } else if (gamePhase==PHASE_RESET_UNCURL && allMotorsDone()) {
    showGesture(myGesture1,true);
    showGesture(myGesture2,false);
    gamePhase=PHASE_WAITING_OPP;
    Serial.println(F("  Showing."));
    Serial.println(F("  > Opp Stage1 e.g. RP:"));
  }
}

void processOpponentInput(String input) {
  input.trim(); input.toUpperCase();
  if (input.length()!=2) { Serial.println(F("  ! 2 gestures e.g. RP")); return; }
  char h1=input.charAt(0), h2=input.charAt(1);
  if ((h1!='R'&&h1!='P'&&h1!='S')||(h2!='R'&&h2!='P'&&h2!='S')) {
    Serial.println(F("  ! R P S only")); return; }
  if (h1==h2) { Serial.println(F("  ! Need 2 different")); return; }

  oppHand1=h1; oppHand2=h2;
  int ci=getComboIndex(h1,h2);
  if (ci>=0) comboCount[ci]++;

  int keep=pickStage2(myGesture1,myGesture2,h1,h2);
  myKept=(keep==1)?myGesture1:myGesture2;

  Serial.print(F("  Opp:")); Serial.print(h1); Serial.print(F("+")); Serial.println(h2);
  Serial.print(F("  Keep:")); Serial.print(myKept);
  Serial.println(keep==1?F(" L"):F(" R"));

  if (keep==1) invalidateHand(false);
  else         invalidateHand(true);

  gamePhase=PHASE_WAITING_RESULT;
  Serial.println(F("  > Opp kept+result e.g. R W:"));
}

void processResult(String input) {
  input.trim(); input.toUpperCase();
  if (input.length()<3) { Serial.println(F("  ! e.g. R W")); return; }
  char ok=input.charAt(0), res=input.charAt(2);
  if (ok!='R'&&ok!='P'&&ok!='S') { Serial.println(F("  ! R P S")); return; }
  if (res!='W'&&res!='L'&&res!='T') { Serial.println(F("  ! W L T")); return; }

  int ci=getComboIndex(oppHand1,oppHand2);
  if (ci>=0) { if (ok==oppHand1) comboKeep[ci][0]++; else comboKeep[ci][1]++; }

  if (historyCount<PATTERN_LEN) oppKeepHistory[historyCount++]=ok;
  else {
    for (int i=0;i<PATTERN_LEN-1;i++) oppKeepHistory[i]=oppKeepHistory[i+1];
    oppKeepHistory[PATTERN_LEN-1]=ok;
  }

  if (res=='W') wins++;
  else if (res=='L') losses++;
  else ties++;
  totalRounds++;

  char exp=determineResult(myKept,ok);
  Serial.println(F("  --- Result ---"));
  Serial.print(F("  Me:")); Serial.print(myKept);
  Serial.print(F(" Opp:")); Serial.print(ok);
  Serial.print(F(" ")); Serial.println(res);
  if (exp!=res) { Serial.print(F("  !Expected ")); Serial.println(exp); }
  Serial.print(F("  W:")); Serial.print(wins);
  Serial.print(F(" L:")); Serial.print(losses);
  Serial.print(F(" T:")); Serial.println(ties);
  Serial.println(F("  HOME then ROUND"));
  gamePhase=PHASE_IDLE;
}

void printStats() {
  Serial.println(F("  === STATS ==="));
  Serial.print(F("  Rounds:")); Serial.println(totalRounds);
  Serial.print(F("  W:")); Serial.print(wins);
  Serial.print(F(" L:")); Serial.print(losses);
  Serial.print(F(" T:")); Serial.println(ties);
  if (totalRounds>0) {
    Serial.print(F("  Win%:"));
    Serial.print((float)wins/totalRounds*100,1);
    Serial.println(F("%"));
  }
  Serial.print(F("  RP:")); Serial.print(comboCount[0]);
  Serial.print(F(" RS:")); Serial.print(comboCount[1]);
  Serial.print(F(" PS:")); Serial.println(comboCount[2]);
  Serial.print(F("  RP R:")); Serial.print(comboKeep[0][0]); Serial.print(F(" P:")); Serial.println(comboKeep[0][1]);
  Serial.print(F("  RS R:")); Serial.print(comboKeep[1][0]); Serial.print(F(" S:")); Serial.println(comboKeep[1][1]);
  Serial.print(F("  PS P:")); Serial.print(comboKeep[2][0]); Serial.print(F(" S:")); Serial.println(comboKeep[2][1]);
}

void printHelp() {
  Serial.println(F("  GAME: ROUND HOME STATS STATE"));
  Serial.println(F("  MANUAL:"));
  Serial.println(F("  LR RR LS RS LP RP LIV RIV"));
  Serial.println(F("  CLR/CLR L/CLR R"));
  Serial.println(F("  OPN/OPN L/OPN R"));
}

// =============================================
// SETUP & LOOP
// =============================================
void setup() {
  Serial.begin(9600);
  randomSeed(analogRead(A0));

  holdLMI=LMI_STOP; holdLPRT=LPRT_STOP;
  holdRMI=RMI_STOP; holdRPRT=RPRT_STOP;

  leftMI.attach(PIN_LEFT_MI);
  leftPRT.attach(PIN_LEFT_PRT);
  rightMI.attach(PIN_RIGHT_MI);
  rightPRT.attach(PIN_RIGHT_PRT);

  motors[M_LMI]=&leftMI;   motors[M_LPRT]=&leftPRT;
  motors[M_RMI]=&rightMI;  motors[M_RPRT]=&rightPRT;
  holds[M_LMI]=&holdLMI;   holds[M_LPRT]=&holdLPRT;
  holds[M_RMI]=&holdRMI;   holds[M_RPRT]=&holdRPRT;
  curleds[M_LMI]=&curledLMI;  curleds[M_LPRT]=&curledLPRT;
  curleds[M_RMI]=&curledRMI;  curleds[M_RPRT]=&curledRPRT;

  leftMI.write(holdLMI);   leftPRT.write(holdLPRT);
  rightMI.write(holdRMI);  rightPRT.write(holdRPRT);

  delay(500);
  Serial.println(F("  RPS-1 READY"));
  Serial.println(F("  HELP for commands"));
  Serial.println(F("  ROUND to start"));
}

void loop() {
  updateMotors();
  updateGamePhase();

  if (Serial.available()>0) {
    String cmd=Serial.readStringUntil('\n');
    cmd.trim();
    String cu=cmd; cu.toUpperCase();

    // --- GAME ---
    if      (cu==F("ROUND"))  { if(gamePhase!=PHASE_IDLE) Serial.println(F("  !Finish round first")); else startRound(); }
    else if (cu==F("HOME"))   { goHome(); gamePhase=PHASE_IDLE; Serial.println(F("  Homing")); }
    else if (cu==F("STATS"))  { printStats(); }
    else if (cu==F("STATE"))  { printState(); }
    else if (cu==F("HELP"))   { printHelp(); }

    // --- MANUAL OVERRIDES ---
    else if (cu==F("LR"))     { Serial.println(F("Left: ROCK"));     leftRock(); }
    else if (cu==F("RR"))     { Serial.println(F("Right: ROCK"));    rightRock(); }
    else if (cu==F("LS"))     { Serial.println(F("Left: SCISSORS")); leftScissors(); }
    else if (cu==F("RS"))     { Serial.println(F("Right: SCISSORS"));rightScissors(); }
    else if (cu==F("LP"))     { Serial.println(F("Left: PAPER"));    leftPaper(); }
    else if (cu==F("RP"))     { Serial.println(F("Right: PAPER"));   rightPaper(); }
    else if (cu==F("LIV"))    { Serial.println(F("Left: INVALID"));  leftInvalid(); }
    else if (cu==F("RIV"))    { Serial.println(F("Right: INVALID")); rightInvalid(); }

    // --- RESET OVERRIDES ---
    else if (cu==F("CLR"))    { curlMotorReset(M_LMI); curlMotorReset(M_LPRT); curlMotorReset(M_RMI); curlMotorReset(M_RPRT); Serial.println(F("  Curl all")); }
    else if (cu==F("CLR L"))  { curlMotorReset(M_LMI); curlMotorReset(M_LPRT); Serial.println(F("  Curl L")); }
    else if (cu==F("CLR R"))  { curlMotorReset(M_RMI); curlMotorReset(M_RPRT); Serial.println(F("  Curl R")); }
    else if (cu==F("OPN"))    { uncurlMotor(M_LMI); uncurlMotor(M_LPRT); uncurlMotor(M_RMI); uncurlMotor(M_RPRT); Serial.println(F("  Open all")); }
    else if (cu==F("OPN L"))  { uncurlMotor(M_LMI); uncurlMotor(M_LPRT); Serial.println(F("  Open L")); }
    else if (cu==F("OPN R"))  { uncurlMotor(M_RMI); uncurlMotor(M_RPRT); Serial.println(F("  Open R")); }

    // --- PHASE INPUT ---
    else {
      if      (gamePhase==PHASE_WAITING_OPP)    processOpponentInput(cmd);
      else if (gamePhase==PHASE_WAITING_RESULT)  processResult(cmd);
      else    Serial.println(F("  ? Type HELP"));
    }
  }
}