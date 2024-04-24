#include <MIDI.h>
#include <midi_Defs.h>
#include <midi_Message.h>
#include <midi_Namespace.h>
#include <midi_Settings.h>
#include <EEPROM.h>
#include <StensTimer.h>
#include <OneButton.h>

//CHANNELS AND TOGGLES MUST MATCH IN WITH OUT AND CC
byte inChannel[] = {2, 3, 4};                       // Channel Switch Pins
byte outChannel[] = {19, 18, 17};                   // Channel Relay Pins
byte ccChannel[] = {85, 86, 87};                 // CC MSG for Channels
byte inToggle [] = {5, 6, 7, 8, 9};                 // Toggle Switch Pins
byte outToggle [] = {12, 13, 14, 15, 16};           // Toggle Relay Pins
byte ccToggle[] = {88, 89, 90, 91, 92};        // CC MSG for Toggles

byte outPins[] = {12, 13, 14, 15, 16, 17, 18, 19};  // All Relay Pins
byte mutePin = 11; //MUTE LED
byte storePin = 10; //MIDI STORE
byte storeLED = mutePin;

// LED & SWITCH STATE VARIABLES
byte buttonState = 0;
byte toggleState = 0;
byte lastButtonState = LOW;

// MIDI VARIABLES
OneButton storeButton(storePin);
byte midi_channel = 0;
const int CC_ON  =  127;
const int CC_OFF = 0;
int currentProgramNumber = 0;
int muteMs = 0;

StensTimer* stensTimer = NULL;
#define  MUTE_TIMER 1
boolean  muteState = false;
#define  defaultMuteMs 20 //20ms
#define  muteAddress 128  //0 to 127 used for MIDI program presets
#define  channelAddress 129  //0 to 127 used for MIDI program presets
#define  firstOnAddress 130  //0 to 127 used for MIDI program presets
#define  lastProgram 131  //load the last selected program on startup

MIDI_CREATE_DEFAULT_INSTANCE();

void handleProgramChange(byte channel, byte number);
void handleControlChange(byte channel, byte value);
void recallPinStates(int programNumber);
void timerCallback(Timer* timer);

void mute() {
  if (!muteState && muteMs > 0) {
    muteState = true;
    digitalWrite(mutePin, HIGH);
    Timer* myTimer = stensTimer->setTimer(MUTE_TIMER, muteMs);
  }    
}

void timerCallback(Timer* timer){
  /* check if the timer is one we expect, if timer has elapsed then turn mute off*/
  if(MUTE_TIMER == timer->getAction() && muteState){
    digitalWrite(mutePin, LOW);
    muteState = false;
  }
}

void setupMIDI(){
  MIDI.setHandleControlChange(handleControlChange);
  MIDI.setHandleProgramChange(handleProgramChange);
  muteMs = EEPROM.read(muteAddress);
  pinMode(mutePin, OUTPUT);
  muteState = false;
  stensTimer = StensTimer::getInstance();
  stensTimer->setStaticCallback(timerCallback);
}

void setupPins(){
	for(int i = 0; i< sizeof(inChannel); i++) { 
		pinMode(inChannel[i], INPUT_PULLUP);	
	}
  for(int i = 0; i< sizeof(outChannel); i++) { 
		pinMode(outChannel[i], OUTPUT);	
	}
  for (unsigned i = 0; i< sizeof(inToggle); i++) {
    pinMode (inToggle[i], INPUT_PULLUP);
  }
  for (unsigned i = 0; i< sizeof(outToggle); i++) {
    pinMode (outToggle[i], OUTPUT);
  } 
  pinMode (mutePin, OUTPUT);
  pinMode(storePin, INPUT_PULLUP);
  pinMode(storeLED, OUTPUT);
  storeButton.attachClick(midiStore);
  storeButton.attachLongPressStart(midiReset);
}

void handleProgramChange(byte channel, byte number) {
  mute();
  currentProgramNumber = number;
  recallPinStates(currentProgramNumber);
  EEPROM.write(lastProgram,currentProgramNumber);
}
void handleControlChange(byte channel, byte value) {
  mute();  
  MIDI.sendControlChange(channel, value, 0xB0 | midi_channel);
  for(int i = 0;i< sizeof(outPins[i]); i++){
  if (value == 127){value = CC_ON;}
  if (value == 0){value = CC_OFF;}
  if (channel == 85) {     
    digitalWrite(outPins[7],value);//CH1
  }
  if (channel == 86) {
    digitalWrite(outPins[6],value);//CH2
  }
  if (channel == 87) {
    digitalWrite(outPins[5],value);//CH3
  }
  if (channel == 88) {
    digitalWrite(outPins[0],value);//T1
  }
  if (channel == 89) {
    digitalWrite(outPins[1],value);//T2
  }
  if (channel == 90) {
    digitalWrite(outPins[2],value);//T3
  }
  if (channel == 91) {
    digitalWrite(outPins[3],value);//T4
  }
  if (channel == 92) {
    digitalWrite(outPins[4],value);//T5
  }
  if (channel == 93) {
    mute();
  }
  if (channel == 94) {
    mute();
  }
  if (channel == 102) { //CC RESET
    midiReset();
  }
  if (channel == 103) { //SET MIDI
    midi_channel = (int)value;
    if (midi_channel > 0 && midi_channel < 17) {
      MIDI.setInputChannel(midi_channel);
    } else {  //if 0 set to OMNI
    MIDI.setInputChannel(MIDI_CHANNEL_OMNI);    
  }
  EEPROM.write(channelAddress, midi_channel);
  }
  if (channel == 104 && value == 0) { //CC SAVE
    midiFunction();
  }
  if (channel == 105){
    muteMs = (int)value;
    EEPROM.write(muteAddress, muteMs);
  }
}
}

void resetChannel() {
    for(int i = 0 ; i< sizeof(inChannel); i++){
     digitalWrite(outChannel[i], LOW); 
    }
}

void Channels(){
	for(int i = 0;i< sizeof(inChannel); i++){
		buttonState = digitalRead(inChannel[i]); 
		if(buttonState == LOW) {
      resetChannel();
      handleControlChange(ccChannel[i], CC_ON); // Sends CC Message
		}
    buttonState = lastButtonState;
    while (digitalRead(inChannel[i]) == LOW){delay(10);}
	}
}

void Toggles(){
  for(int i = 0;i< sizeof(inToggle); i++){
		buttonState = digitalRead(inToggle[i]); 
    toggleState = digitalRead(outToggle[i]);
		  if(buttonState == LOW && toggleState == LOW) {
        handleControlChange(ccToggle[i], CC_ON); // Sends CC Message
      }
      if(buttonState == LOW && toggleState == HIGH) {
        handleControlChange(ccToggle[i], CC_OFF); // Sends CC Message
      }
      delay(5); 
    while (digitalRead(inToggle[i]) == LOW){delay(10);}
  }
}

void recallPinStates(int programNumber)  {
  int loopCount = 0;
  int outputPin = storeLED; 
  int pinStates = 0;
  pinStates = EEPROM.read(programNumber);
  for (int loopCount = 7; loopCount >=0 ; loopCount--)  {
    //read individual bits and set matching output pin
    digitalWrite(outputPin, bitRead(pinStates, loopCount));
        outputPin++;
    }
}

void midiFunction(){
  int address = currentProgramNumber;
  byte pinStateArray[8] = {0,0,0,0,0,0,0,0};
  int pinState = 0;
  for(int i = 0;i< sizeof(outPins[i]); i++){
      pinStateArray[i] = digitalRead(outPins[i]);
      pinState += (pinStateArray[i] << (i-7));
      i++;  
    }
    EEPROM.write(address, pinState);
    digitalWrite(storeLED, HIGH);
    delay(400);
    digitalWrite(storeLED, LOW);
    delay(10);
}
static void midiStore() {
    handleControlChange(104, 0);
}
static void midiReset() {
  for (int resetAddress = 0; resetAddress < 128; resetAddress++) {
    EEPROM.write(resetAddress, 0x01); 
    }
    EEPROM.write(muteAddress, defaultMuteMs); 
    delay(10);
    digitalWrite(storeLED, HIGH);
    delay(800);
    digitalWrite(storeLED, LOW);
}

void setup() {
  Serial.begin(31250);
  setupMIDI();
  MIDI.begin(MIDI_CHANNEL_OMNI);
  setupPins();
  digitalWrite(outChannel[0], HIGH);
}

void loop() {
  MIDI.read();
  stensTimer->run();
  Channels();
  Toggles();
  storeButton.tick();
}