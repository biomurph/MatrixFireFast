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

    This version uses the serial connection to adjust fire variables
      - flareRows
      - nFlare
      - flareChance
      - flareDecay

      - colorDepth (?)
 
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


#define VERSION "ESP32 Serial Adjustable"

// #define DISPLAY_TEST  /* uncomment to show test patterns at startup */

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
 * with other stuff. 
 */
const uint16_t rows = MATRIX_HEIGHT;
const uint16_t cols = MATRIX_WIDTH;

const uint8_t MAX_FLARE_ROWS = MATRIX_HEIGHT;     /* set max array size for flare rows */

/* Flare-iables */
uint8_t flareRows = 3;    /* variable number of rows (from bottom) allowed to flare,  was const */
uint8_t MaxFlareRows = 10;
uint8_t MinFlareRows = 1;
uint8_t flareMax = 5;     /* variable max number of simultaneous flares */
uint8_t MaxFlares = 30;
uint8_t MinFlares = 2;
uint8_t flareChance = 40; /* variable chance (%) of a new flare (if maxflare allows room) */
uint8_t MaxFlareChance = 100;
uint8_t MinFlareChance = 10;
uint8_t flareDecay = 16;  /* variable decay rate of flare radiation; 14 is good?? */
uint8_t MaxFlareDecay = 30;
uint8_t MinFlareDecay = 3;


const uint32_t flameColor_11[] = {
  0x000000,
  0x100000,
  0x300000,
  0x600000,
  0x800000,
  0xA00000,
  0xC20000,
  0xC04000,
  0xC06000,
  0xC08000,
  0x807080
};
const uint8_t NCOLORS_11 = (sizeof(flameColor_11)/sizeof(flameColor_11[0]));

const uint32_t flameColor_18[] = {
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
const uint8_t NCOLORS_18 = (sizeof(flameColor_18)/sizeof(flameColor_18[0]));


uint32_t pix[rows][cols];  
uint8_t nflare = 0;
uint32_t flare[MAX_FLARE_ROWS];


uint32_t isqrt(uint32_t n) {
  if ( n < 2 ) return n;
  uint32_t smallCandidate = isqrt(n >> 2) << 1;
  uint32_t largeCandidate = smallCandidate + 1;
  return (largeCandidate*largeCandidate > n) ? smallCandidate : largeCandidate;
}

// Set pixels to intensity around flare
void glow( int x, int y, int z ) {  // pix[r][c]
  int b = z * 10 / flareDecay + 1;
  for ( int i=(y-b); i<(y+b); ++i ) {
    for ( int j=(x-b); j<(x+b); ++j ) {
      if ( i >=0 && j >= 0 && i < rows && j < cols ) {
        int d = ( flareDecay * isqrt((x-j)*(x-j) + (y-i)*(y-i)) + 5 ) / 10;
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
  if ( nflare < flareMax && random(1,101) <= flareChance ) {
    int x = random(0, cols);
    int y = random(0, flareRows);
    int z = NCOLORS_18 - 1;
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
      pix[0][c] = random(NCOLORS_18-13, NCOLORS_18-9);
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

/*
  Set and draw
  pix[row|height][collum|width] array stores the color array number
  after all the mods, that goes into the matrix array in drawPixel.
*/
  for(int r=0; r<rows; r++){
    for(int c=0; c<cols; c++){
      // matrix[pos(j,i)] = flameColor[pix[i][j]];
      matrix.drawPixel(r,c,matrix.color24bit(flameColor[pix[r][c]]));
    }
  }
  matrix.show();
}



void setup() {
  Serial.begin(115200);
  pinMode(6,OUTPUT);
  pinMode(E,OUTPUT); digitalWrite(E,LOW); // E address is on the HUB74 GND pin
  // Initialize matrix...
  ProtomatterStatus status = matrix.begin();
  Serial.print("\nProtomatter begin() status: ");  
  Serial.println((int)status);   
  matrix.setRotation(1); 
  matrix.fillScreen(0x0000); 
  matrix.show(); 

  Serial.begin(115200); while (!Serial){}
  Serial.print("MatrixFire Serial v"); Serial.print(VERSION);
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
