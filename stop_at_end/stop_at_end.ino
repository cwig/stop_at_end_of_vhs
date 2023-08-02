#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <Adafruit_NeoPixel.h>

#include "USB.h"
#include "USBHIDKeyboard.h"

#define NUMPIXELS        1
Adafruit_NeoPixel pixels(NUMPIXELS, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);

const uint16_t kJVCPower = 0xC2D0;
const uint16_t kJVCPlay = 0xC230;
const uint16_t kJVCStop = 0xC2C0;

const uint16_t kIrLed = A3;
IRsend irsend(kIrLed);

USBHIDKeyboard Keyboard;

const int LIGHT_THRESHOLD = 300

int val = 0;

bool waitingToStart = true;

const int WAITING_TO_START = 0;
const int WAITING_FOR_PLAY_TO_START = 1;
const int PLAYING = 2;
int state;

void setup() {

  Serial.begin(115200);
  irsend.begin();

  pinMode(0, INPUT_PULLUP); 

  state = 0;

  pixels.begin();
  pixels.setBrightness(20);

  Keyboard.begin();
  USB.begin();
}

void loop() {

  if(state == WAITING_TO_START) {
    bool btnIsDown = !digitalRead(0);
    if(btnIsDown) {
      pixels.fill(0x0000FF);
      pixels.show();
      Serial.println("Starting Keyboard...");
      Keyboard.press(KEY_F5);
      Keyboard.releaseAll();
      delay(3000);
      state = WAITING_FOR_PLAY_TO_START;
    }
  }

  if(state == WAITING_FOR_PLAY_TO_START) {
    val = analogRead(A0);

    if(val >= LIGHT_THRESHOLD) {
      pixels.fill(0x00FF00);
      pixels.show();

      Serial.println("Detected Playing...");
      delay(1000);

      state = PLAYING;
    }
    else {
      for(int j=0; j<3; j++) {
        irsend.sendJVC(kJVCPlay, 16, 1);
        delay(100);
      }
    }
  }

  if(state == PLAYING) {
    val = analogRead(A0);
    if(val < LIGHT_THRESHOLD) {
      pixels.fill(0x00FF00);
      pixels.show();
      Serial.println("Stopping Keyboard...");

      Keyboard.press(KEY_ESC);
      Keyboard.releaseAll();

      for(int j=0; j<10; j++) {
        irsend.sendJVC(kJVCPause, 16, 1);
        delay(100);
      }

      pixels.fill(0xFF0000);
      pixels.show();

      state = WAITING_TO_START;
    }
  }

  delay(100);
}
