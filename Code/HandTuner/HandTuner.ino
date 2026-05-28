#include <Servo.h>

Servo leftMI;
Servo leftPRT;
Servo rightMI;
Servo rightPRT;

const int PIN_LEFT_MI   = 5;
const int PIN_LEFT_PRT  = 6;
const int PIN_RIGHT_MI  = 10;
const int PIN_RIGHT_PRT = 11;

//=== COPY INTO MAIN CODE ===
int LMI_STOP        = 90;
int LMI_LOCK        = 100;
int LMI_CURL_SPD    = 180;
int LMI_CURL_DUR    = 1000;
int LMI_UNCURL_SPD  = 0;
int LMI_UNCURL_DUR  = 550;
//---
int LPRT_STOP       = 90;
int LPRT_LOCK       = 85;
int LPRT_CURL_SPD   = 0;
int LPRT_CURL_DUR   = 1000;
int LPRT_UNCURL_SPD = 180;
int LPRT_UNCURL_DUR = 575;
//---
int RMI_STOP        = 90;
int RMI_LOCK        = 88;
int RMI_CURL_SPD    = 0;
int RMI_CURL_DUR    = 1000;
int RMI_UNCURL_SPD  = 180;
int RMI_UNCURL_DUR  = 540;
//---
int RPRT_STOP       = 90;
int RPRT_LOCK       = 85;
int RPRT_CURL_SPD   = 0;
int RPRT_CURL_DUR   = 1000;
int RPRT_UNCURL_SPD = 180;
int RPRT_UNCURL_DUR = 540;
//===========================

bool curledLMI  = false;
bool curledLPRT = false;
bool curledRMI  = false;
bool curledRPRT = false;

int holdLMI  = 92;
int holdLPRT = 93;
int holdRMI  = 93;
int holdRPRT = 93;

bool          motorRunning = false;
unsigned long stopAt       = 0;
int           selectedMotor = 2; // 0=LMI, 1=LPRT, 2=RMI, 3=RPRT
bool          runningCurl  = false;

void setup() {
  Serial.begin(9600);
  leftMI.attach(PIN_LEFT_MI);
  leftPRT.attach(PIN_LEFT_PRT);
  rightMI.attach(PIN_RIGHT_MI);
  rightPRT.attach(PIN_RIGHT_PRT);

  holdLMI  = LMI_STOP;
  holdLPRT = LPRT_STOP;
  holdRMI  = RMI_STOP;
  holdRPRT = RPRT_STOP;

  leftMI.write(holdLMI);
  leftPRT.write(holdLPRT);
  rightMI.write(holdRMI);
  rightPRT.write(holdRPRT);

  printHelp();
}

void printHelp() {
  Serial.println("==================================");
  Serial.println("SELECT:  LMI | LPRT | RMI | RPRT");
  Serial.println("ACTIONS: CURL | UNCURL");
  Serial.println("TUNE:");
  Serial.println("  STOP xx  - open hold value");
  Serial.println("  LOCK xx  - curled hold value");
  Serial.println("  CS xxx   - curl speed");
  Serial.println("  CD xxxx  - curl duration ms");
  Serial.println("  US xxx   - uncurl speed");
  Serial.println("  UD xxxx  - uncurl duration ms");
  Serial.println("  STATUS | SAVE | HELP");
  Serial.println("==================================");
}

void printStatus() {
  Serial.println("--- LMI ---");
  Serial.print("  STOP="); Serial.print(LMI_STOP);   Serial.print(" LOCK="); Serial.print(LMI_LOCK);
  Serial.print(" CS=");    Serial.print(LMI_CURL_SPD); Serial.print(" CD="); Serial.print(LMI_CURL_DUR);
  Serial.print(" US=");    Serial.print(LMI_UNCURL_SPD); Serial.print(" UD="); Serial.println(LMI_UNCURL_DUR);

  Serial.println("--- LPRT ---");
  Serial.print("  STOP="); Serial.print(LPRT_STOP);   Serial.print(" LOCK="); Serial.print(LPRT_LOCK);
  Serial.print(" CS=");    Serial.print(LPRT_CURL_SPD); Serial.print(" CD="); Serial.print(LPRT_CURL_DUR);
  Serial.print(" US=");    Serial.print(LPRT_UNCURL_SPD); Serial.print(" UD="); Serial.println(LPRT_UNCURL_DUR);

  Serial.println("--- RMI ---");
  Serial.print("  STOP="); Serial.print(RMI_STOP);   Serial.print(" LOCK="); Serial.print(RMI_LOCK);
  Serial.print(" CS=");    Serial.print(RMI_CURL_SPD); Serial.print(" CD="); Serial.print(RMI_CURL_DUR);
  Serial.print(" US=");    Serial.print(RMI_UNCURL_SPD); Serial.print(" UD="); Serial.println(RMI_UNCURL_DUR);

  Serial.println("--- RPRT ---");
  Serial.print("  STOP="); Serial.print(RPRT_STOP);   Serial.print(" LOCK="); Serial.print(RPRT_LOCK);
  Serial.print(" CS=");    Serial.print(RPRT_CURL_SPD); Serial.print(" CD="); Serial.print(RPRT_CURL_DUR);
  Serial.print(" US=");    Serial.print(RPRT_UNCURL_SPD); Serial.print(" UD="); Serial.println(RPRT_UNCURL_DUR);
}

void printSave() {
  Serial.println("=== COPY INTO MAIN CODE ===");
  Serial.print("int LMI_STOP        = "); Serial.print(LMI_STOP);        Serial.println(";");
  Serial.print("int LMI_LOCK        = "); Serial.print(LMI_LOCK);        Serial.println(";");
  Serial.print("int LMI_CURL_SPD    = "); Serial.print(LMI_CURL_SPD);   Serial.println(";");
  Serial.print("int LMI_CURL_DUR    = "); Serial.print(LMI_CURL_DUR);   Serial.println(";");
  Serial.print("int LMI_UNCURL_SPD  = "); Serial.print(LMI_UNCURL_SPD); Serial.println(";");
  Serial.print("int LMI_UNCURL_DUR  = "); Serial.print(LMI_UNCURL_DUR); Serial.println(";");
  Serial.println("---");
  Serial.print("int LPRT_STOP       = "); Serial.print(LPRT_STOP);        Serial.println(";");
  Serial.print("int LPRT_LOCK       = "); Serial.print(LPRT_LOCK);        Serial.println(";");
  Serial.print("int LPRT_CURL_SPD   = "); Serial.print(LPRT_CURL_SPD);   Serial.println(";");
  Serial.print("int LPRT_CURL_DUR   = "); Serial.print(LPRT_CURL_DUR);   Serial.println(";");
  Serial.print("int LPRT_UNCURL_SPD = "); Serial.print(LPRT_UNCURL_SPD); Serial.println(";");
  Serial.print("int LPRT_UNCURL_DUR = "); Serial.print(LPRT_UNCURL_DUR); Serial.println(";");
  Serial.println("---");
  Serial.print("int RMI_STOP        = "); Serial.print(RMI_STOP);        Serial.println(";");
  Serial.print("int RMI_LOCK        = "); Serial.print(RMI_LOCK);        Serial.println(";");
  Serial.print("int RMI_CURL_SPD    = "); Serial.print(RMI_CURL_SPD);   Serial.println(";");
  Serial.print("int RMI_CURL_DUR    = "); Serial.print(RMI_CURL_DUR);   Serial.println(";");
  Serial.print("int RMI_UNCURL_SPD  = "); Serial.print(RMI_UNCURL_SPD); Serial.println(";");
  Serial.print("int RMI_UNCURL_DUR  = "); Serial.print(RMI_UNCURL_DUR); Serial.println(";");
  Serial.println("---");
  Serial.print("int RPRT_STOP       = "); Serial.print(RPRT_STOP);        Serial.println(";");
  Serial.print("int RPRT_LOCK       = "); Serial.print(RPRT_LOCK);        Serial.println(";");
  Serial.print("int RPRT_CURL_SPD   = "); Serial.print(RPRT_CURL_SPD);   Serial.println(";");
  Serial.print("int RPRT_CURL_DUR   = "); Serial.print(RPRT_CURL_DUR);   Serial.println(";");
  Serial.print("int RPRT_UNCURL_SPD = "); Serial.print(RPRT_UNCURL_SPD); Serial.println(";");
  Serial.print("int RPRT_UNCURL_DUR = "); Serial.print(RPRT_UNCURL_DUR); Serial.println(";");
  Serial.println("===========================");
}

// Getters for selected motor
int   getStop()      { if(selectedMotor==0) return LMI_STOP;        if(selectedMotor==1) return LPRT_STOP;       if(selectedMotor==2) return RMI_STOP;        return RPRT_STOP; }
int   getLock()      { if(selectedMotor==0) return LMI_LOCK;        if(selectedMotor==1) return LPRT_LOCK;       if(selectedMotor==2) return RMI_LOCK;        return RPRT_LOCK; }
int   getCurlSpd()   { if(selectedMotor==0) return LMI_CURL_SPD;    if(selectedMotor==1) return LPRT_CURL_SPD;   if(selectedMotor==2) return RMI_CURL_SPD;    return RPRT_CURL_SPD; }
int   getCurlDur()   { if(selectedMotor==0) return LMI_CURL_DUR;    if(selectedMotor==1) return LPRT_CURL_DUR;   if(selectedMotor==2) return RMI_CURL_DUR;    return RPRT_CURL_DUR; }
int   getUncurlSpd() { if(selectedMotor==0) return LMI_UNCURL_SPD;  if(selectedMotor==1) return LPRT_UNCURL_SPD; if(selectedMotor==2) return RMI_UNCURL_SPD;  return RPRT_UNCURL_SPD; }
int   getUncurlDur() { if(selectedMotor==0) return LMI_UNCURL_DUR;  if(selectedMotor==1) return LPRT_UNCURL_DUR; if(selectedMotor==2) return RMI_UNCURL_DUR;  return RPRT_UNCURL_DUR; }
int*  getHold()      { if(selectedMotor==0) return &holdLMI;        if(selectedMotor==1) return &holdLPRT;       if(selectedMotor==2) return &holdRMI;        return &holdRPRT; }
bool* getCurled()    { if(selectedMotor==0) return &curledLMI;      if(selectedMotor==1) return &curledLPRT;     if(selectedMotor==2) return &curledRMI;      return &curledRPRT; }
Servo* getMotor()    { if(selectedMotor==0) return &leftMI;         if(selectedMotor==1) return &leftPRT;        if(selectedMotor==2) return &rightMI;        return &rightPRT; }

void setStop(int v) {
  if(selectedMotor==0) LMI_STOP=v;  else if(selectedMotor==1) LPRT_STOP=v;
  else if(selectedMotor==2) RMI_STOP=v; else RPRT_STOP=v;
  if(!*getCurled()) { *getHold()=v; getMotor()->write(v); }
  Serial.print("STOP="); Serial.println(v);
}
void setLock(int v) {
  if(selectedMotor==0) LMI_LOCK=v;  else if(selectedMotor==1) LPRT_LOCK=v;
  else if(selectedMotor==2) RMI_LOCK=v; else RPRT_LOCK=v;
  if(*getCurled()) { *getHold()=v; getMotor()->write(v); }
  Serial.print("LOCK="); Serial.println(v);
}
void setCurlSpd(int v)   { if(selectedMotor==0) LMI_CURL_SPD=v;    else if(selectedMotor==1) LPRT_CURL_SPD=v;   else if(selectedMotor==2) RMI_CURL_SPD=v;   else RPRT_CURL_SPD=v;   Serial.print("CS="); Serial.println(v); }
void setCurlDur(int v)   { if(selectedMotor==0) LMI_CURL_DUR=v;    else if(selectedMotor==1) LPRT_CURL_DUR=v;   else if(selectedMotor==2) RMI_CURL_DUR=v;   else RPRT_CURL_DUR=v;   Serial.print("CD="); Serial.println(v); }
void setUncurlSpd(int v) { if(selectedMotor==0) LMI_UNCURL_SPD=v;  else if(selectedMotor==1) LPRT_UNCURL_SPD=v; else if(selectedMotor==2) RMI_UNCURL_SPD=v; else RPRT_UNCURL_SPD=v; Serial.print("US="); Serial.println(v); }
void setUncurlDur(int v) { if(selectedMotor==0) LMI_UNCURL_DUR=v;  else if(selectedMotor==1) LPRT_UNCURL_DUR=v; else if(selectedMotor==2) RMI_UNCURL_DUR=v; else RPRT_UNCURL_DUR=v; Serial.print("UD="); Serial.println(v); }

void doCurl() {
  if (motorRunning) { Serial.println("Already running!"); return; }
  Serial.print("Curling "); Serial.println(selectedMotor==0?"LMI":selectedMotor==1?"LPRT":selectedMotor==2?"RMI":"RPRT");
  *getHold() = getCurlSpd();
  getMotor()->write(getCurlSpd());
  motorRunning = true;
  runningCurl  = true;
  stopAt = millis() + getCurlDur();
}

void doUncurl() {
  if (motorRunning) { Serial.println("Already running!"); return; }
  Serial.print("Uncurling "); Serial.println(selectedMotor==0?"LMI":selectedMotor==1?"LPRT":selectedMotor==2?"RMI":"RPRT");
  *getHold() = getUncurlSpd();
  getMotor()->write(getUncurlSpd());
  motorRunning = true;
  runningCurl  = false;
  stopAt = millis() + getUncurlDur();
}

void loop() {
  if (motorRunning && millis() >= stopAt) {
    motorRunning = false;
    if (runningCurl) {
      *getCurled() = true;
      *getHold()   = getLock();
      getMotor()->write(getLock());
      Serial.print("Curled. Lock="); Serial.println(getLock());
    } else {
      *getCurled() = false;
      *getHold()   = getStop();
      getMotor()->write(getStop());
      Serial.print("Uncurled. Stop="); Serial.println(getStop());
    }
  }

  // Continuously refresh all 4 motors
  leftMI.write(holdLMI);
  leftPRT.write(holdLPRT);
  rightMI.write(holdRMI);
  rightPRT.write(holdRPRT);

  if (Serial.available() > 0) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    cmd.toUpperCase();

    if      (cmd == "LMI")    { selectedMotor = 0; Serial.println("Selected: LMI"); }
    else if (cmd == "LPRT")   { selectedMotor = 1; Serial.println("Selected: LPRT"); }
    else if (cmd == "RMI")    { selectedMotor = 2; Serial.println("Selected: RMI"); }
    else if (cmd == "RPRT")   { selectedMotor = 3; Serial.println("Selected: RPRT"); }
    else if (cmd == "CURL")   { doCurl(); }
    else if (cmd == "UNCURL") { doUncurl(); }
    else if (cmd.startsWith("STOP "))  { setStop(cmd.substring(5).toInt()); }
    else if (cmd.startsWith("LOCK "))  { setLock(cmd.substring(5).toInt()); }
    else if (cmd.startsWith("CS "))    { setCurlSpd(cmd.substring(3).toInt()); }
    else if (cmd.startsWith("CD "))    { setCurlDur(cmd.substring(3).toInt()); }
    else if (cmd.startsWith("US "))    { setUncurlSpd(cmd.substring(3).toInt()); }
    else if (cmd.startsWith("UD "))    { setUncurlDur(cmd.substring(3).toInt()); }
    else if (cmd == "STATUS")          { printStatus(); }
    else if (cmd == "SAVE")            { printSave(); }
    else if (cmd == "HELP")            { printHelp(); }
    else { Serial.println("Unknown. Type HELP."); }
  }
}