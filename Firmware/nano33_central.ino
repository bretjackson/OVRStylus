/* ================================================================================
 * This file is part of the OVR Stylus Open Source Project, which is developed and 
 * maintained by Bret Jackson, Macalester College.
 *
 * File: nano33_central.ino
 * This file is meant to be loaded on an Arduino Nano 33 BLE to serve as a central device for 
 * The OVR Stylus. It scans for BLE peripherals until one with the advertised Nordic UART service
 * "6e400001-b5a3-f393-e0a9-e50e24dcca9e" UUID is found. Once discovered and connected,
 * it will forward BLE messages to the USB serial connection.
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

//#define DEBUG

#include <ArduinoBLE.h>

const int ledPin =  LED_BUILTIN;// the number of the LED pin
int ledState = LOW;             // ledState used to set the LED
unsigned long previousMillis = 0;        // will store last time LED was updated
const long interval = 500;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  // Set the timeout on input-string parsing to 1ms, so we
  // don't waste a lot of time waiting for the completion of
  // numbers.
  Serial.setTimeout(1);

  pinMode(ledPin, OUTPUT);

  // initialize the BLE hardware
  BLE.begin();

#ifdef DEBUG
  Serial.println("BLE Central - OVR Stylus");
  Serial.println("Scanning for Nordic UART peripherals named 'OVR Stylus'...");
#endif

  // start scanning for peripherals
  BLE.scanForUuid("6e400001-b5a3-f393-e0a9-e50e24dcca9e");
}

void loop() {
  // check if a peripheral has been discovered
  BLEDevice peripheral = BLE.available();

  if (peripheral) {
    ledState = LOW;
    digitalWrite(ledPin, ledState);
    
#ifdef DEBUG
    // discovered a peripheral, print out address, local name, and advertised service
    Serial.print("Found ");
    Serial.print(peripheral.address());
    Serial.print(" '");
    Serial.print(peripheral.localName());
    Serial.print("' ");
    Serial.print(peripheral.advertisedServiceUuid());
    Serial.println();
#endif

    if (peripheral.localName() == "OVR Stylus") {
      // stop scanning
      BLE.stopScan();

      repeatBLEToSerial(peripheral);

      // peripheral disconnected, start scanning again
#ifdef DEBUG
      Serial.println("Scanning for Nordic UART peripherals named 'OVR Stylus'...");
#endif
      BLE.scanForUuid("6e400001-b5a3-f393-e0a9-e50e24dcca9e");
    }
  }
  else {
    // Serial.println("Waiting for available peripheral");
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= interval) {
      // save the last time you blinked the LED
      previousMillis = currentMillis;

      // if the LED is off turn it on and vice-versa:
      if (ledState == LOW) {
        ledState = HIGH;
      } else {
        ledState = LOW;
      }

      // set the LED with the ledState of the variable:
      digitalWrite(ledPin, ledState);
    }
  }
}

void repeatBLEToSerial(BLEDevice peripheral) {
  // connect to the peripheral
#ifdef DEBUG
  Serial.print("Connecting ...   ");
#endif

  if (peripheral.connect()) {
#ifdef DEBUG
    Serial.println("Connected");
#endif
  } else {
#ifdef DEBUG
    Serial.println("Failed to connect");
#endif
    return;
  }

  // discover peripheral attributes
#ifdef DEBUG
  Serial.print("Discovering attributes ...   ");
#endif
  if (peripheral.discoverAttributes()) {
#ifdef DEBUG
    Serial.println("Attributes discovered");
#endif
  } else {
#ifdef DEBUG
    Serial.println("Attribute discovery failed");
#endif
    peripheral.disconnect();
    return;
  }

  // retrieve the characteristics
  BLECharacteristic rxCharacteristic = peripheral.characteristic("6e400002-b5a3-f393-e0a9-e50e24dcca9e");
  BLECharacteristic txCharacteristic = peripheral.characteristic("6e400003-b5a3-f393-e0a9-e50e24dcca9e");

  if (!rxCharacteristic) {
#ifdef DEBUG
    Serial.println("Peripheral does not have RX characteristic");
#endif
    peripheral.disconnect();
    return;
  } else if (!rxCharacteristic.canWrite()) {
#ifdef DEBUG
    Serial.println("Peripheral does not have a writable RX characteristic");
#endif
    peripheral.disconnect();
    return;
  } else {
#ifdef DEBUG
    Serial.println("Can write to RX characteristic");
#endif
  }

  if (!txCharacteristic) {
#ifdef DEBUG
    Serial.println("Peripheral does not have TX characteristic");
#endif
    peripheral.disconnect();
    return;
  } else if (!txCharacteristic.canSubscribe()) {
#ifdef DEBUG
    Serial.println("TX characteristic is not subscribable");
#endif
    peripheral.disconnect();
    return;
  } else if (!txCharacteristic.subscribe()) {
#ifdef DEBUG
    Serial.println("Subscription to TX characteristic failed");
#endif
    peripheral.disconnect();
    return;
  } else {
#ifdef DEBUG
    Serial.println("Subscribed to TX Characterisitic");
#endif
  }

  while (peripheral.connected()) {
    // while the peripheral is connected

    if (Serial.available() > 0) {
      // get incoming byte:
      byte inByte = Serial.read();
#ifdef DEBUG
      Serial.print("Writing: ");
      Serial.write(inByte);
      Serial.println(" ");
#endif

      rxCharacteristic.writeValue(inByte);
    }

    if (txCharacteristic.valueUpdated()) {
      byte outBytes[20]; // lp_BLESerial.h buffers in sets of 20 bytes
      int readLen = txCharacteristic.readValue(outBytes, 20);
      Serial.write(outBytes, readLen);
    }
  }

#ifdef DEBUG
  Serial.println("Peripheral disconnected");
#endif
}
