/* ================================================================================
 * This file is part of the OVR Stylus Open Source Project, which is developed and 
 * maintained by Bret Jackson, Macalester College.
 *
 * File: Stylus.ino
 * This file is meant to be loaded on the stylus hardware using the Sandeep Mistry's 
 * Arduino Core for nRF5 based boards (https://github.com/sandeepmistry/arduino-nRF5)
 * and Matthew Ford's Pfod Low power library (https://www.forward.com.au/pfod/BLE/LowPower/pfod_lp_nrf52.zip)
 * See the Readme.md for installation instructions
 * -----------------------------------------------------------------------------------
 * Copyright © 2020 Bret Jackson
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and 
 * associated documentation files (the “Software”), to deal in the Software without restriction, including 
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell 
 * copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the 
 * following conditions: The above copyright notice and this permission notice shall be included in all 
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT 
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN 
 * NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE 
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
 
#include <lp_BLESerial.h>

#include <Wire.h>
#include "Adafruit_DRV2605.h"

Adafruit_DRV2605 drv;


int led = 13;
int buttonPin0 = 4;
int buttonPin1 = 7;
int softpotPin = 5;

int button0State = 1;
int button1State = 1;
int softpotState = 0;
boolean stateChanged = false;

int previousSoftPotADC = 0;

lp_BLESerial ble;

lp_timer sleepTimer;
const unsigned long DELAY_TIME = 2; // mS

const byte MESSAGE_DELIMITER = '!';
char endMarker = '\n';
char rc;
const byte numChars = 32;
char receivedChars[numChars];
int hapticEffectId = 0; 
boolean newData = false;


void setup() {

  pinMode(buttonPin0, INPUT_PULLUP); // pin is LOW when pressed
  pinMode(buttonPin1, INPUT_PULLUP);
  pinMode(softpotPin, INPUT);
  pinMode(led, OUTPUT);

  drv.begin();
  drv.selectLibrary(1);
  // I2C trigger by sending 'go' command
  // default, internal trigger when sending GO command
  drv.setMode(DRV2605_MODE_INTTRIG);

  // initialize the digital pin as an output.
  ble.setName("OVR Stylus"); // Advertised name
  ble.setConnectedHandler(handleConnection);
  ble.setDisconnectedHandler(handleDisconnection);
  ble.setConnectionInterval(1,1);
  ble.setAdvertisingInterval(20);
  ble.begin(); // start advertising and be ready to accept connections
}

void loop() {
  sleep(); // just sleep here waiting a trigger

  static byte ndx = 0;
  while (ble.available() > 0) {
    rc = ble.read();

    if (rc != endMarker) {
      receivedChars[ndx] = rc;
      ndx++;
      if (ndx >= numChars) {
        ndx = numChars - 1;
      }
    }
    else {
      receivedChars[ndx] = '\0'; // terminate the string
      ndx = 0;
      newData = true;
    }
    
    if (newData == true) {
        hapticEffectId = 0;
        hapticEffectId = atoi(receivedChars);
        if (hapticEffectId > 0 && hapticEffectId < 124) {
          playHapticEffect(hapticEffectId);
        }
        newData = false;
    }
  }
}

void playHapticEffect(int effectId) {
  // set the effect to play
  drv.setWaveform(0, effectId);  // play effect
  drv.setWaveform(1, 0);       // end waveform

  // play the effect!
  drv.go();
}

void handleSleepTimer() {
  int button0 = digitalRead(buttonPin0);
  int button1 = digitalRead(buttonPin1);
  
  int softPotADC = analogRead(softpotPin);
  float alpha = 0.2;
  int EWMF = (1.0-alpha)*previousSoftPotADC + alpha*softPotADC; // exponential smoothing filter
  previousSoftPotADC = EWMF;
  int softPotPosition = map(EWMF, 0, 1023, 0, 63);
  
  if (button0 != button0State) {
    button0State = button0;
    stateChanged = true;
  }
  if (button1 != button1State) {
    button1State = button1;
    stateChanged = true;
  }
  if (softPotPosition != softpotState) {
    softpotState = softPotPosition;
    stateChanged = true;
  }

  if (stateChanged) {
    sendStateUpdate();
  }
  sleepTimer.start(DELAY_TIME, handleSleepTimer, APP_TIMER_MODE_SINGLE_SHOT);
}

void handleButton0PinLevelChange() {
  button0State = !button0State;
  stateChanged = true;
}

void handleButton1PinLevelChange() {
  button1State = !button1State;
  stateChanged = true;
}

void sendStateUpdate() {
  stateChanged = false;
  if (button0State == 0 || button1State == 0) {
    digitalWrite(led, HIGH);
  }
  else {
    digitalWrite(led, LOW);
  }

  //Message Format:
  //    '!' as delimiter byte
  //    second byte contains states:
  //        - Leftmost bit for button0,
  //        - Second highest bit is button1,
  //        - 6 rightmost bits to represent softpot value (converting 0-1023 to 0-63)
    byte states = 0b00000000;
    if (button0State == LOW) {
      bitSet(states, 7);
    }
    if (button1State == LOW) {
      bitSet(states, 6);
    }
 
    for(int i = 0; i < 6; i++) {
      if (bitRead(softpotState, i)) {
        bitSet(states, i);
      }
    }

  //ble.write(MESSAGE_DELIMITER); ble.print(button0State); ble.print(button1State); ble.println(softpotState);//ble.write(states);
  ble.write(MESSAGE_DELIMITER); ble.write(states);
  ble.flush();
}

void handleConnection(BLECentral& central) {
  sleepTimer.start(DELAY_TIME, handleSleepTimer, APP_TIMER_MODE_SINGLE_SHOT);
}

void handleDisconnection(BLECentral& central) {
  sleepTimer.stop();
}
