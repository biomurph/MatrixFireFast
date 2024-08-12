

void displayTest()
  FastLED.clear();
  for ( uint16_t i=0; i<cols; ++i ) {
    matrix[pos(i,0)] = colors[NCOLORS-1];
    FastLED.show();
    delay(1000/FPS);
    matrix[pos(i,0)] = 0;
  }
  for ( uint16_t i=0; i<rows; ++i ) {
    matrix[pos(0,i)] = colors[NCOLORS-1];
    FastLED.show();
    delay(1000/FPS);
    matrix[pos(0,i)] = 0;
  }
  /** Show the color map briefly at the extents of the display. This "demo"
   *  is meant to help establish correct origin, extents, colors, and
   *  brightness. You can cut or comment this out if you don't need it;
   *  it's not important to functionality otherwise.
   */
  uint16_t y = 0;
  FastLED.clear();
  for ( int i=NCOLORS-1; i>=0; --i ) {
    if ( y < rows ) {
      matrix[pos(0,y)] = colors[i];
      matrix[pos(cols-1,y++)] = colors[i];
    }
    else break;
  }
  FastLED.show();
  delay(2000);
}