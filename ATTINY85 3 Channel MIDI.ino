byte inPins[] = {0, 1, 2};
byte outPins[] = {3, 4, 5};

enum { ON = HIGH, OFF = LOW };
#define N_BUT sizeof(inPins)
byte ToggleState [N_BUT];

int chkButtons() {
  for (unsigned n = 0; n < sizeof(inPins); n++) {
    byte BUT = digitalRead(inPins[n]);
      if (ToggleState[n] != BUT) {
        ToggleState[n] = BUT;
        if (OFF == BUT){
          return n;
        }    
      }
  while (digitalRead(inPins[n]) == LOW) {delay(10);}
  }
  return -1;
}

void setIO(){
  for (unsigned c = 0; c< sizeof(inPins); c++) {
    pinMode (inPins[c], INPUT_PULLUP);
    ToggleState[c] = digitalRead(inPins[c]);
  }
  for (unsigned d = 0; d< sizeof(outPins); d++) {
    pinMode (outPins[d], OUTPUT);
  } 
}

void setFunctions(){
  int n = chkButtons();
    if (0 <= n) {
      digitalWrite(outPins[n], !digitalRead(outPins[n]));
    } 
    delay(10);
}
void setOFF(){
  digitalWrite(outPins[0], HIGH);
  digitalWrite(outPins[1], HIGH);
  digitalWrite(outPins[2], HIGH);
}
void setup() {
  setIO();
  setOFF();
}
void loop() {
  setFunctions();
}