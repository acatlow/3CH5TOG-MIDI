#include <MIDI.h>

MIDI_CREATE_DEFAULT_INSTANCE();

const uint8_t patchButtonPins[] = {2, 3, 4, 5};
const uint8_t patchLEDPins[] = {8, 9, 10, 11};
const uint8_t patches[] = {0, 1, 2, 3};

const size_t numberOfButtons = sizeof(patchButtonPins) / sizeof(patchButtonPins[0]);
const size_t numberOfLEDs = sizeof(patchLEDPins) / sizeof(patchLEDPins[0]);
const size_t numberOfPatches = sizeof(patches) / sizeof(patches[0]);

static_assert(numberOfButtons == numberOfLEDs, "The number of buttons doesn't match the number of LEDs");
static_assert(numberOfButtons == numberOfPatches, "The number of buttons doesn't match the number of patches");

const uint8_t CCButtonPin = 6;
const uint8_t CCLEDPin = 12;
const uint8_t controller = 14;
const uint8_t ccValueOff = 0;
const uint8_t ccValueOn = 1;

const uint8_t channel = 1;
 
void setup() {
  for (const uint8_t &pin : patchButtonPins)
    pinMode(pin, INPUT_PULLUP);
  pinMode(CCButtonPin, INPUT_PULLUP);
  for (const uint8_t &pin : patchLEDPins)
    pinMode(pin, OUTPUT);
  pinMode(CCLEDPin, OUTPUT);

  MIDI.begin(MIDI_CHANNEL_OMNI) ;
}

int8_t getPressedPatchButton() {
  for (int8_t i = 0; i < numberOfButtons; i++)
    if (digitalRead(patchButtonPins[i]) == LOW)
      return i;
  return -1;
}

void setPatchLED(int8_t led, bool state) {
  if (led >= 0) 
    digitalWrite(patchLEDPins[led], state);
}

void updatePatchSelect() {
  static int8_t activeSelection = -1;
  int8_t pressedButton = getPressedPatchButton();
  if (pressedButton >= 0 && pressedButton != activeSelection) {
    setPatchLED(activeSelection, LOW);
    activeSelection = pressedButton;
    setPatchLED(activeSelection, HIGH);
    MIDI.sendProgramChange(patches[activeSelection], channel);
  }
}

bool controllerButtonIsPressed() { // Read and debounce button, return true on falling edge
  const static unsigned long debounceTime = 25;
  const static int8_t rising = HIGH - LOW;
  const static int8_t falling = LOW - HIGH;
  static bool previousState = HIGH;
  static unsigned long previousBounceTime = 0;
  bool pressed = false;
  bool state = digitalRead(CCButtonPin);               // read the button's state
  int8_t stateChange = state - previousState;  // calculate the state change since last time
  if (stateChange == falling) { // If the button is pressed (went from high to low)
    if (millis() - previousBounceTime > debounceTime) { // check if the time since the last bounce is higher than the threshold
      pressed = true; // the button is pressed
    }
  } else if (stateChange == rising) { // if the button is released or bounces
    previousBounceTime = millis();      // remember when this happened
  }
  previousState = state; // remember the current state
  return pressed; // return true if the button was pressed and didn't bounce
}

void updateController() {
  static bool controllerState = false;
  if (controllerButtonIsPressed()) {
    controllerState = !controllerState;
    digitalWrite(CCLEDPin, controllerState);
    MIDI.sendControlChange(controller, controllerState ? ccValueOn : ccValueOff, channel);
  }
}

void loop() {
  updatePatchSelect();
  updateController();
}