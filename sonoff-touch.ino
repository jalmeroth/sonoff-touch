#include <Homie.h>

#define PIN_BUTTON 0
#define PIN_RELAY 12
#define PIN_LED 13

HomieNode switchNode("switch", "switch");

#define FW_NAME "sonoff-touch"
#define FW_VERSION "1.0.0"

/* Magic sequence for Autodetectable Binary Upload */
const char *__FLAGGED_FW_NAME = "\xbf\x84\xe4\x13\x54" FW_NAME "\x93\x44\x6b\xa7\x75";
const char *__FLAGGED_FW_VERSION = "\x6a\x3f\x3e\x0e\xe1" FW_VERSION "\xb0\x30\x48\xd4\x1a";
/* End of magic sequence for Autodetectable Binary Upload */

int relayState = LOW;
int lastButtonState = LOW;           // the previous reading from the input pin
unsigned long lastSent = 0;
unsigned long debounceTime = 0;      // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

bool switchOnHandler(String value) {
  if (value == "true") {
    Homie.setNodeProperty(switchNode, "on", "true", true);
    relayState = HIGH;
    Serial.println("Switch is on");
  } else if (value == "false") {
    Homie.setNodeProperty(switchNode, "on", "false", true);
    relayState = LOW;
    Serial.println("Switch is off");
  } else {
    return false;
  }
  return true;
}

void setupHandler() {
  // publish current relayState when online
  Homie.setNodeProperty(switchNode, "on", (relayState == HIGH) ? "true" : "false", true);
}

void loopHandler() {
  // pass
}

void setup() {
  pinMode(PIN_BUTTON , INPUT);
  Homie.setFirmware(FW_NAME, FW_VERSION);
  Homie.setLedPin(PIN_LED, HIGH);
  Homie.setResetTrigger(PIN_BUTTON, LOW, 5000);
  Homie.registerNode(switchNode);
  switchNode.subscribe("on", switchOnHandler);
  Homie.setSetupFunction(setupHandler);
  Homie.setLoopFunction(loopHandler);
  Homie.setup();
}

void loop() {
  int reading = digitalRead(PIN_BUTTON);

  if (reading != lastButtonState) {
    // start timer for the first time
    if (debounceTime == 0) {
      debounceTime = millis();
    }

    if(reading == LOW) {
      Serial.println("Button push");
      // restart timer
      debounceTime = millis();
    } else {
      Serial.println("Button release");
      // let a unicorn pass 
      if ((millis() - debounceTime) > debounceDelay) {
        // invert relayState
        relayState = !relayState;
        if (Homie.isReadyToOperate()) {
          // normal mode and network connection up
          switchOnHandler((relayState == HIGH) ? "true" : "false");
        } else {
          // not in normal mode or network connection down
          // digitalWrite(PIN_RELAY, relayState);
        }
      }
    }
  }
  lastButtonState = reading;

  Homie.loop();
}
