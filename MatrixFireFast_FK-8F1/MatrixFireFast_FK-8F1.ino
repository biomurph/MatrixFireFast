/**
 * MatrixFireFast - A fire simulation for NeoPixel (and other?) matrix
 * displays on Arduino (or ESP8266) using FastLED.
 *
 * Author: Patrick Rigney (https://www.toggledbits.com/)
 * Copyright 2020 Patrick H. Rigney, All Rights Reserved.
 *
 * Github: https://github.com/toggledbits/MatrixFireFast
 * License information can be found at the above Github link.
 *
    Modified by Joel Murphy (https://www.biomurph.com) Summer, 2024

    Target: ESP32-S2 mounted on FK-8F1 board Reference Link: (https://www.amazon.ca/FK-8F1-Color-Controller-Asynchronous-Pixels/dp/B0D5V8STB7)
 
    The FK-8F1 was found inside this 32H x 160W LED display: (link)

    The MakeFireFast code expects to see a string of neo pixels. 
    Thankfully, the MakeFireFast code uses a 2D array [row][col] to store the heat color data.
    I can use their array and assign the colors using protomatter, but I have to add a function to protomatter.

    I modified the function to handle the 24 bit color in MakeFireFast:
        // This is based on the HSV function in Adafruit_NeoPixel.cpp, but with
        // 16-bit RGB565 output for GFX lib rather than 24-bit. See that code for
        // an explanation of the math, this is stripped of comments for brevity.
        uint16_t Adafruit_Protomatter::color24bit(int c) {
          uint8_t r = (c >> 16) & 0xFF;
          uint8_t g = (c >> 8) & 0xFF;
          uint8_t b = c & 0xFF;
          return colorRGB(r,g,b);
        }

    Success!
    I am using the pix[][] paradigm to address an array of color values.
    pix array is [y][x] or [row][col] with the origin preferably at Lower Left of matrix [San Diego]
    If you see something funny, check the setRotation setting in setup.
 
 */

#include <Adafruit_Protomatter.h>

#define MATRIX_WIDTH 160
#define MATRIX_HEIGHT 32

#define E 34 // FK-8F1 is ABCDE driver. Needs to be pulled low?
                      // R1,G1,B1,R2,G2,B2
  uint8_t rgbPins[]  = {12, 13, 17, 21, 20, 16};
  uint8_t addrPins[] = {38, 37, 36, 35}; // E = 34
  uint8_t clockPin   = 10; // Must be on same port as rgbPins
  uint8_t latchPin   = 33;
  uint8_t oePin      = 11;


#define VERSION "ESP32"

// #define DISPLAY_TEST  /* define to show test patterns at startup */

Adafruit_Protomatter matrix(
  160,          // Width of matrix (or matrix chain) in pixels
  4,           // Bit depth, 1-6
  1, rgbPins,  // # of matrix chains, array of 6 RGB pins for each
  4, addrPins, // # of address pins (height is inferred), array of pins
  clockPin, latchPin, oePin, // Other matrix control pins
  false);      // No double-buffering here (see "doublebuffer" example)
#define FPS 15              /* Refresh rate */


/* Display size; can be smaller than matrix size, and if so, you can move the origin.
 * This allows you to have a small fire display on a large matrix sharing the display
 * with other stuff. See README at Github. */

const uint16_t rows = MATRIX_HEIGHT;
const uint16_t cols = MATRIX_WIDTH;

/* Flare constants */
const uint8_t flarerows = 3;    /* number of rows (from bottom) allowed to flare */
const uint8_t maxflare = 3;     /* max number of simultaneous flares */
const uint8_t flarechance = 40; /* chance (%) of a new flare (if there's room) */
const uint8_t flaredecay = 16;  /* decay rate of flare radiation; 14 is good */


// const uint32_t flameColor[] = {
//   0x000000,
//   0x100000,
//   0x300000,
//   0x600000,
//   0x800000,
//   0xA00000,
//   0xC20000,
//   0xC04000,
//   0xC06000,
//   0xC08000,
//   0x807080
// };

const uint32_t flameColor[] = {
  0x000000,
  0xFF0000,
  0xCF0100,
  0xAF1000,
  0xA51600,
  0xA52003,
  0xA52504,
  0xA52505,
  0xAB2C02,
  0xB82800,
  0xCF3405,
  0xDA4B03,
  0xE45401,
  0xF76504,
  0xFC6B04,
  0xFE9114,
  0xFCA01F,
  0xF6BD39
};

const uint8_t NCOLORS = (sizeof(flameColor)/sizeof(flameColor[0]));


uint32_t pix[rows][cols];  // x, y uint8_t pix[rows][cols];
uint8_t nflare = 0;
uint32_t flare[maxflare];


uint32_t isqrt(uint32_t n) {
  if ( n < 2 ) return n;
  uint32_t smallCandidate = isqrt(n >> 2) << 1;
  uint32_t largeCandidate = smallCandidate + 1;
  return (largeCandidate*largeCandidate > n) ? smallCandidate : largeCandidate;
}

// Set pixels to intensity around flare
void glow( int x, int y, int z ) {  // pix[r][c]
  int b = z * 10 / flaredecay + 1;
  for ( int i=(y-b); i<(y+b); ++i ) {
    for ( int j=(x-b); j<(x+b); ++j ) {
      if ( i >=0 && j >= 0 && i < rows && j < cols ) {
        int d = ( flaredecay * isqrt((x-j)*(x-j) + (y-i)*(y-i)) + 5 ) / 10;
        uint8_t n = 0;
        if ( z > d ) n = z - d;
        if ( n > pix[i][j] ) { // can only get brighter
          pix[i][j] = n;
        }
      }
    }
  }
}

void newflare() {
  if ( nflare < maxflare && random(1,101) <= flarechance ) {
    int x = random(0, cols);
    int y = random(0, flarerows);
    int z = NCOLORS - 1;
    flare[nflare++] = (z<<16) | (y<<8) | (x&0xff);
    glow( x, y, z );
  }
}

/** make_fire() animates the fire display. It should be called from the
 *  loop periodically (at least as often as is required to maintain the
 *  configured refresh rate). Better to call it too often than not enough.
 *  It will not refresh faster than the configured rate. But if you don't
 *  call it frequently enough, the refresh rate may be lower than
 *  configured.
 */
unsigned long t = 0; /* keep time */
void make_fire(){
  uint16_t i, j;
  if(t > millis()){ return; }
  t = millis() + (1000 / FPS);

  // First, move all existing heat points up the display and fade
  // saving and modifying everything in the pix array
  for(int decay=0; decay<3; decay++){  // can you do this multiple times?
    for(uint16_t r=rows-1; r>0; r--){  // stack the row from top to bottom
      for(uint16_t c=0; c<cols; c++){  // stack the col from L to R
        uint8_t n = 0;
        if(pix[r-1][c] > 0){ // if the row beneath is hot
          n = pix[r-1][c] - 1; // do the math
          pix[r][c] = n; // make yourself one less hot
        }
      }
    }
  }

  // Heat the bottom row
  for(uint16_t c=0; c<cols; c++){
    int i = pix[0][c];
    if ( i > 0 ) {
      pix[0][c] = random(NCOLORS-13, NCOLORS-9);
    }
  }

  // flare
  for ( i=0; i<nflare; ++i ) {
    int x = flare[i] & 0xff;
    int y = (flare[i] >> 8) & 0xff;
    int z = (flare[i] >> 16) & 0xff;
    glow( x, y, z );
    if ( z > 1 ) {
      flare[i] = (flare[i] & 0xffff) | ((z-1)<<16);
    } else {
      // This flare is out
      for ( int j=i+1; j<nflare; ++j ) {
        flare[j-1] = flare[j];
      }
      --nflare;
    }
  }
  newflare();

  // Set and draw
  for(int r=0; r<rows; r++){
    for(int c=0; c<cols; c++){
      matrix.drawPixel(r,c,matrix.color24bit(flameColor[pix[r][c]]));
    }
  }
  matrix.show();
}

/*
  pix[r-h][c-w] array stores the color array number
  after all the mods, that goes into the matrix array.
*/

void setup() {
  Serial.begin(115200);
  pinMode(6,OUTPUT);
  pinMode(E,OUTPUT); digitalWrite(E,LOW); // E address is on the HUB74 GND pin
  // Initialize matrix...
  ProtomatterStatus status = matrix.begin();
  Serial.print("\nProtomatter begin() status: ");  
  Serial.println((int)status);   
  matrix.setRotation(1); // this is based on the native protomatter orientation. Mechanical and software just need to meet up.
  matrix.fillScreen(0x0000); 
  matrix.show(); 

  Serial.begin(115200); while (!Serial){}
  Serial.print("MatrixFireFast v"); Serial.print(VERSION);
  Serial.print(", FPS "); Serial.println(FPS);
  delay(2000);

#ifdef DISPLAY_TEST
  displayTest();
#endif
}

void loop() {
  make_fire();
  delay(10);
}
