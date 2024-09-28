
/*
   Colors for matrix fire
   list goes from least hot to most hot
*/

uint8_t red,grn,blu;

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
