/**
 * MatrixFireFast - A fire simulation for NeoPixel (and other?) matrix
 * displays on Arduino (or ESP8266) using FastLED.
 *
 * Author: Patrick Rigney (https://www.toggledbits.com/)
 * Copyright 2020 Patrick H. Rigney, All Rights Reserved.
 *
    Modified by Joel Murphy (https://www.biomurph.com) Summer, 2024

    Target: ESP32-S2 mounted on FK-8F1 board Reference Link: (https://www.amazon.ca/FK-8F1-Color-Controller-Asynchronous-Pixels/dp/B0D5V8STB7)
 
    The FK-8F1 was found inside this 32H x 160W LED display: (https://www.amazon.com/LED-Resolution-P10-Technology-Advertising/dp/B07Q3NB1D5)

    The MakeFireFast code expects to see a string of neo pixels. 
    Thankfully, the MakeFireFast code uses a 2D array [row][col] to store the heat color data.
    I can use their array and assign the colors using protomatter, but I have to add a function to protomatter.

    I added a function to handle the 24 bit color in MakeFireFast compatible with the Protomatter library:
        // This is based on the HSV function in Adafruit_NeoPixel.cpp, but with
        // 16-bit RGB565 output for GFX lib rather than 24-bit. See that code for
        // an explanation of the math, this is stripped of comments for brevity.
        uint16_t Adafruit_Protomatter::color24bit(int c) {
          uint8_t r = (c >> 16) & 0xFF;
          uint8_t g = (c >> 8) & 0xFF;
          uint8_t b = c & 0xFF;
          return colorRGB(r,g,b);
        }

    This version uses the serial connection to adjust fire variables
      - flareRows (rows from bottom flare can happen)
      - flareMax (this needs testing) 
      - flareChance (random based percent chance)
      - flareDecay (lower = slower)
    
    TO DO:
      - color (?)
      - colorDepth (?)
 
 */

#include <Adafruit_Protomatter.h>
#include "colors.h"

#define MATRIX_WIDTH 160
#define MATRIX_HEIGHT 32
#define E 34 // FK-8F1 is ABCDE driver. E needs to be pulled low?
#define LED 6
#define VERSION "ESP32 Serial Adjustable"
#define FPS 15              /* frame rate */
unsigned long t = 0;        /* frame rate timer */
// #define DISPLAY_TEST  /* uncomment to show test patterns at startup */
                   // R1, G1, B1, R2, G2, B2
uint8_t rgbPins[]  = {12, 13, 17, 21, 20, 16};
uint8_t addrPins[] = {38, 37, 36, 35}; // E = 34
uint8_t clockPin   = 10; // Must be on same port as rgbPins
uint8_t latchPin   = 33;
uint8_t oePin      = 11;
/* Display size; can be smaller than matrix size, and if so, you can move the origin.
 * This allows you to have a small fire display on a large matrix sharing the display
 * with other stuff. 
 */
const uint16_t rows = MATRIX_HEIGHT;
const uint16_t cols = MATRIX_WIDTH;
const uint8_t MAX_FLARE_ROWS = MATRIX_HEIGHT;     /* set max array size for flare rows */
uint32_t pix[rows][cols];  
uint8_t nflare = 0;
uint32_t flare[MAX_FLARE_ROWS];
/* Flareiables */
uint8_t flareRows = 3;    /* variable number of rows (from bottom) allowed to flare,  was const */
uint8_t MaxFlareRows = MAX_FLARE_ROWS;
uint8_t MinFlareRows = 1;
uint8_t flareMax = 5;     /* variable max number of simultaneous flares */
uint8_t MaxFlares = 30;
uint8_t MinFlares = 1;
uint8_t flareChance = 40; /* variable chance (%) of a new flare (if maxflare allows room) */
uint8_t MaxFlareChance = 100;
uint8_t MinFlareChance = 10;
uint8_t flareDecay = 16;  /* variable decay rate of flare radiation; 14 is good?? */
uint8_t MaxFlareDecay = 30;
uint8_t MinFlareDecay = 3;

Adafruit_Protomatter matrix(
  160,          // Width of matrix (or matrix chain) in pixels
  4,           // Bit depth, 1-6
  1, rgbPins,  // # of matrix chains, array of 6 RGB pins for each
  4, addrPins, // # of address pins (height is inferred), array of pins
  clockPin, latchPin, oePin, // Other matrix control pins
  false);      // No double-buffering here (see "doublebuffer" example)





void setup() {
  Serial.begin(115200); while (!Serial){}
  delay(1000);
  printControl();
  pinMode(LED,OUTPUT); digitalWrite(E,LOW); // led on control board useful for debigging.
  pinMode(E,OUTPUT); digitalWrite(E,LOW); // E address is on the HUB74 GND pin
  // Initialize matrix...
  ProtomatterStatus status = matrix.begin();
  Serial.print("\nProtomatter status: ");  
  Serial.println((int)status);   
  matrix.setRotation(1); 
  matrix.fillScreen(0x0000); 
  matrix.show(); 
#ifdef DISPLAY_TEST
  displayTest();
#endif
}

void loop() {
  make_fire();
  serialCheck();
}
