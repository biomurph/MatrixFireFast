/*
    MatrixFireFast is an algorithmic flame generator made by toggledbits (https://github.com/toggledbits/MatrixFireFast)
    
    Modified by Joel Murphy (https://www.biomurph.com) Summer, 2024

    Target Microcontroller: ESP32-S2 mounted on FK-8F1 board Reference Link: (https://www.amazon.ca/FK-8F1-Color-Controller-Asynchronous-Pixels/dp/B0D5V8STB7)
 
    The FK-8F1 was found inside this 32H x 160W LED display: (https://www.amazon.com/LED-Resolution-P10-Technology-Advertising/dp/B07Q3NB1D5)

    The MakeFireFast code expects to see a string of neo pixels. 
    Thankfully, the MakeFireFast code uses a 2D array pix[row][col] to store the heat color data for the matrix.
    I can use the pix array to set colors and then assign the colors using protomatter.
    Before I set the color in protomatter, I have to convert 24bit RGB to 16bit 565.

    This version sets up a server with user controls to adjust fire parameters
      - flareRows (rows from bottom flare can happen)
      - flareMax (maximum number of flares allowed at one time) 
      - flareChance (random based percent chance of a flare happening)
      - flareDecay (how flast the flare dies down: lower = slower)
   
     To update the server data file:
  	First, put the ESP32 into bootloader mode, then close any open serial monitors connected to it.
  	Then, use [CMD][SHIFT][P] to open the command palett and then "Upload LittleFS"
   
    TO DO:
      - color adjust (?)
      - colorDepth adjust (?)
      - make flareDecay into a random range (?)
*/

#include <Adafruit_Protomatter.h>
#include "colors.h"
#include <Arduino.h>
#include "FS.h"
#include "LittleFS.h"  
#include "ESPmDNS.h"
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "credz.h"  // create a credz.h file in this sketch folder to save your network credentials

#define MATRIX_WIDTH 160
#define MATRIX_HEIGHT 32
#define E 34 // FK-8F1 is ABCDE driver. E needs to be pulled low?
#define LED 6
// #define VERSION "ESP32 MatrixFire Server"
#define FPS 15              /* frame rate */
unsigned long t = 0;        /* frame rate timer */
// #define DISPLAY_TEST  /* uncomment to show test patterns at startup */
                   // R1, G1, B1, R2, G2, B2
uint8_t rgbPins[]  = {12, 13, 17, 21, 20, 16};
uint8_t addrPins[] = {38, 37, 36, 35}; // E = 34
uint8_t clockPin   = 10; // Must be on same port as rgbPins
uint8_t latchPin   = 33;
uint8_t oePin      = 11;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80); // Create a WebSocket object
AsyncWebSocket ws("/ws"); // Set LED GPIO
const int ledPin = 6;
char msg[1024];
char gasoline[256];
char flariables[256];
bool gotUpdated = true;
// Initialize files
void initLittleFS(){  
  if (!LittleFS.begin(true)) {
    Serial.println(F("An error has occurred while mounting LittleFS")); 
  }
}

bool isBurning = true;
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

char versionString[] = "Matrix Fire Server v1";
const char* hostname = "pixel-fire";

void setup() {
  Serial.begin(115200);
  delay(1000);
  printControl();
  pinMode(LED,OUTPUT); digitalWrite(LED,LOW); // led on control board useful for debugging.
  pinMode(E,OUTPUT); digitalWrite(E,LOW); // E address is on the HUB74 GND pin
  // Initialize matrix...
  ProtomatterStatus status = matrix.begin();
  Serial.print(F("\nProtomatter status: "));  
  Serial.println((int)status);   
  matrix.setRotation(1); 
  matrix.fillScreen(0x0000); 
  matrix.show(); 
#ifdef DISPLAY_TEST
  displayTest();
#endif

  initLittleFS();
  initWiFi();
  initWebSocket();
// Web Server Root URL
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/windex.html", "text/html");
  });
  server.serveStatic("/", LittleFS, "/");
  // Start server
  server.begin();
}

void loop() {
  if(gotUpdated){
    gotUpdated = false;
    printFlariables();
  }
  make_fire();
  serialCheck();
  ws.cleanupClients();
}

