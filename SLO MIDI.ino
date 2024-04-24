#include <SoftwareSerial.h>
SoftwareSerial mySerial(2, 3); // RX, TX
#include <EEPROM.h>

const uint8_t NUM_LEDS = 3;

struct Info {
  const uint8_t buttonPin;
  const uint8_t ledPin;
  uint8_t reading;
  bool buttonState;
  bool lastButtonState;
  bool ledState;
};

Info INFOS[NUM_LEDS] = {
  {4, 8, 0, HIGH, HIGH, HIGH},
  {5, 9, 0, HIGH, HIGH, LOW},
  {6, 10, 0, HIGH, HIGH, LOW},
};

unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;
unsigned long programmingDelay = 2000;

byte statusByte;
byte dataByte;
byte midiChannel;
byte pgmNumber;

void setup() {
  Serial.begin(9600);
  mySerial.begin(31250);
  delay(100);

  for (uint8_t i = 0; i < NUM_LEDS; i++) {
    pinMode(INFOS[i].ledPin, OUTPUT);
    digitalWrite(INFOS[i].ledPin, INFOS[i].ledState);
    pinMode(INFOS[i].buttonPin, INPUT_PULLUP);
  }
}

void loop() {
  for (uint8_t i = 0; i < NUM_LEDS; i++) {
    INFOS[i].reading = digitalRead(INFOS[i].buttonPin);
    if (INFOS[i].reading != INFOS[i].lastButtonState) {
      lastDebounceTime = millis();
    }

    if ((millis() - lastDebounceTime) > debounceDelay) {
      if (INFOS[i].reading != INFOS[i].buttonState) {
        INFOS[i].buttonState = INFOS[i].reading;
        if (INFOS[i].buttonState == LOW) {

          for (uint8_t l = 0; l < NUM_LEDS; l++) {
            INFOS[l].ledState = LOW;
          }
          INFOS[i].ledState = HIGH;
        }
      }
    }

    if (((millis() - lastDebounceTime) > programmingDelay) && (INFOS[i].buttonState == LOW)) {
      EEPROM.update(dataByte, i);
    }

    digitalWrite(INFOS[i].ledPin, INFOS[i].ledState);
    INFOS[i].lastButtonState = INFOS[i].reading;
  }

  if ( mySerial.available() > 1) {
    statusByte = mySerial.read();
    dataByte = mySerial.read();

    if ( (statusByte & B11110000) == B11000000) {
      midiChannel = statusByte & B00001111;
      pgmNumber = dataByte + 1;
    };

    int CHANNELS = EEPROM.read(dataByte);
    for (uint8_t l = 0; l < NUM_LEDS; l++) {
      INFOS[l].ledState = LOW;
      digitalWrite(INFOS[l].ledPin, INFOS[l].ledState);
    }
    INFOS[CHANNELS].ledState = HIGH;
    digitalWrite(INFOS[CHANNELS].ledPin, INFOS[CHANNELS].ledState);
  }
}