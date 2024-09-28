

void displayTest(){
  Serial.println("Testing Display");
  matrix.fillScreen(0x0000); // FastLED.clear();
  matrix.show();
  delay(100);
  for ( uint16_t i=0; i<cols; ++i ) { // pix[r][c]
    pix[0][i] = NCOLORS_18-1;
    matrix.drawPixel(0,i,color24bitTo565(flameColor_18[pix[0][i]]));
    matrix.show(); // FastLED.show();
    delay(1000/FPS);
    pix[0][i] = flameColor_18[0];  // matrix[pos(i,0)] = 0;
    matrix.drawPixel(0,i,color24bitTo565(flameColor_18[pix[0][i]]));
  }
  for ( uint16_t i=0; i<rows; ++i ) {
    pix[i][0] = NCOLORS_18-1;
    matrix.drawPixel(i,0,color24bitTo565(flameColor_18[pix[i][0]]));
    matrix.show(); // FastLED.show();
    delay(1000/FPS);
    pix[i][0] = flameColor_18[0];
    matrix.drawPixel(i,0,color24bitTo565(flameColor_18[pix[i][0]]));
  }
  /** Show the color map briefly at the extents of the display. This "demo"
   *  is meant to help establish correct origin, extents, colors, and
   *  brightness. You can cut or comment this out if you don't need it;
   *  it's not important to functionality otherwise.
   */
  uint16_t r = 0;
  matrix.fillScreen(0x0000);  // FastLED.clear();
  matrix.show();
  delay(100);
  for(int i=NCOLORS_18-1; i>=0; i--){
    if(r < rows){  // pix[r][c]
      pix[r][0] = i;
      matrix.drawPixel(r,0,color24bitTo565(flameColor_18[pix[r][0]]));
      pix[r][cols-1] = i;
      matrix.drawPixel(r,cols-1,color24bitTo565(flameColor_18[pix[r][cols-1]]));
      r++;
    }
    else break;
  }
  matrix.show(); // FastLED.show();
  delay(2000);
}

