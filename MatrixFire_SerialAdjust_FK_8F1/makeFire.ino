


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
      pix[0][c] = random(NCOLORS_18-9, NCOLORS_18-5);
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
      matrix.drawPixel(r,c,matrix.color24bit(flameColor_18[pix[r][c]]));
    }
  }
  matrix.show();
}
