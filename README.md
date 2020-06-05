# MatrixFireFast

Just on a whim, I decided to make my own fire simulation using a 44x11 WS2812 matrix I had purchased on Amazon. There's plenty of code available to do this, but I just wanted to figure out for myself how to code a nice-looking animation. So I ignored all of those other implementations, and just set about failing repeatedly... until I didn't.

The result is MatrixFireFast. It works on any LED matrix supported by [FastLED](http://fastled.io/). You can see it [here on YouTube](#).

I started development with an Arduino Uno and the Adafruit NeoMatrix libarary, and that worked fine as long as I kept the display size down. The Uno's RAM topped out at about 11x12, meaning I was using less than 1/4th of the entire matrix width available. So when things started to look like they were going to work well, I dug out a Mega 2560, which had enough RAM to manage the entire 44x11 display. But it was still not refreshing quite fast enough for my taste, so I moved off the Adafruit libraries to FastLED. That worked famously, so after some tweaking and a couple of hours playing with it on both Arduino and a NodeMCU/ESP8266, I was satisfied (for the moment).

The simulation does what I think pretty much everyone does at the most basic level: establishes "heat" near the bottom of the display and percolates it up, reducing the heat as you go. To simulate the licking flames of fire, I added random "flares" in the fire that rise from the bottom of the display, and also radiate outward, which I think is really the thing that most improves the effect.

The implementation is straightforward. After getting the effect right, I went through and tried to optimize a bit, particularly for memory use, to try and squeeze in larger displays on the lesser Arduinos. 

> Note: When I say "large" in this context, I do not mean physical size; it always and only means the total number of pixels.

As it stands today, I have tested it on:

* an Arduino Mega 2560 using a matrix of 44x11;
* an Arduino Uno using a 32x8 matrix;
* a NodeMCU/ESP8266 using the 44x11 matrix.

If you get it running in a different configuration, please let me know! Also, see "Matrix Size and Processor Selection" below for more information.

> Tip: I think the simulation also looks really good on a long one-pixel wide linear string of LEDs.

Also note that FastLED is a really fast libarary for sending pixels out to LED strings (kudos!), but it's not a matrix library. It views a matrix as it is *electrically* &mdash; a linear arrangement of LEDs. In order to map the rectangular physical arrangement of LEDs into their linear electrical implementation, the function `pos()` is used extensively. It maps a "canonical" column and row address to a linear pixel address. The included implementation of `pos()` uses several preprocessor macros in the attempt to support the most common arrangements. You will only need to customize `pos()` if the configuration switches provided don't adequately cover your matrix's pixel arrangement. See "Customizing the `pos()` Function" below for more details about this. 

## Matrix Size and Processor Selection

So far, the ESP8266 clearly wins for RAM and processor speed from among the devices I've tested with. High frame rates (too high to be realistic, I think) are easy, and I haven't yet probed the limits of display size. The form factor is also much better. If you were going to choose a processor for a semi-permanent installation, I would definitely go with the ESP8266. Just be aware that level shifting may be required, as the ESP is a 3.3V device, and the voltage of the matrix may not be compatible, and could in fact, damage the microprocessor if you hook it up incorrectly.

As of this writing, the RAM requirement is basically a baseline of 180 bytes (without other libraries, including Serial), plus four bytes per pixel (i.e. at least `width * height * 4 + 180` bytes). Using the full size of my 484 pixel 44x11 matrix, the RAM requirement is just over 2K (hence the need for the Arduino Mega 2560). A 32x8 display squeezes into an Uno. Increasing or decreasing either the number of possible flares or the number of displayed colors slightly modifies the baseline 180-byte requirement but is not a significant contributor. The matrix is the greedy consumer.

> Note: I've made attempts to reduce this requirement to three bytes per pixel (just what FastLED requires), but thus far my attempts have increased code complexity and reduced performance, and the exchange isn't worth it, IMO. RAM is cheap.

Another thing to be aware of is that the refresh rate will be limited by the size of the display and the type of LEDs. For NeoPixels, for example, there are strict timing requirements for data transmission and it takes about 30&micro;s per pixel. So 100 pixels takes the library 3ms to update. During this time, FastLED must disable interrupts, so anything else going on is going to be delayed accordingly. My 44x11 test matrix, with 484 pixels, takes 14.52ms to transmit to the display, and therefore the theoretical limit of the frame rate is around 68 frames per second. Forunately, this is higher than what I feel looks good (for this display, something in the low 20s is most pleasing to my eye).

## Basic Configuration

The most basic thing you need to do is configure the size of the display you are using, and what data pin is used to transmit the pixel stream to the display. If you do nothing else to the code when you first try it, you have to get these right. These are set near the stop of the code:

* `MAT_W` - Width of the matrix in pixels;
* `MAT_H` - Height of the matrix in pixels;
* `MAT_PIN` - The pin to use for serial pixel stream.

Note: If you are using an ESP8266, you may need to use a level shifter. ESP8266 is a 3.3V device, and presenting voltages much higher may damage the microprocessor. The 3.3V data output signal may also be insufficient to be recognized as data by your LED matrix. As it happened, the 5V 44x11 matrix I used worked fine without a level shifter--it's data line is high impedence (presents no voltage, just accepts whatever the processor gives it), and senses edges fine at the 3.3V level.

You also need to know a little bit about your matrix. In a pixel matrix, although you are looking at a rectangular arrangement, electrically the pixels are linear. That is, if you have a 16x16 matrix, you have 256 pixels numbered from 0 to 255. Depending on the manufacturer, pixel 0 can be in any corner (and hopefully nowhere else but one of the four corners, because that would just be weird). Pixel 1 then, depending on where pixel 0 is, could be to the left or right, above or below. You need to work out the order of the pixels in your display. If you don't know, and the documentation for the matrix doesn't tell you, or your dog ate it, or whatever, you can run the pixel test program included in the distribution to determine it. See "Using PixelTest" below.

If your display has pixel 0 in the top row, either at the left or right, make sure `MAT_TOP` is *define*d. If pixel 0 is in the bottom row, then `MAT_TOP` must be *undef*ined.

If pixel 0 is on the left edge of your display, then `MAT_LEFT` must be *define*d; otherwise, it should be *undef*ined.

Finally, if your pixels zig-zag, that is, if each row goes the opposite direction of the other, then `MAT_ZIGZAG` should be *define*d; otherwise, it must be *undef*ined.

### Your First Run

When you start up the code, if you've left the `DISPLAY_TEST` macro defined as it is by default, it will run a display test by first displaying white vertical bars sweeping from left to right, following by horizontal bars sweeping bottom to top. If these bars are broken or look odd, your display configuration is different from what you have set in the foregoing instructions. Recheck `MAT_TOP`, `MAT_LEFT`, and `MAT_ZIGZAG`. If you are unsure, read "Using PixelTest" below.

If the display test displayed properly, you should now see the fire simulation. The default settings for fire "behavior" are set up for wider displays (32 pixel width or more). If your display is smaller, please read on, as you will likely need to tone down some of the flare settings so the display is less "busy" in the smaller matrix. Tuning is part of the fun.

## Advanced Configuration

The most common thing you'll probably want to tweak is the apparent "intensity" of the inferno. The licking of flames is simulated by randomly injecting "flares" into the fire near the bottom. The following constants effect where and when these flares occur, and how big they can be:

* `flarerows` - How many of the bottom rows are allowed to flare; flares occur in a randomly selected row up to `flamerows` from the bottom; if 0, only the bottom row can flare.
* `maxflare` - The maximum number of simultaneous flares. A flare lives until it cools out and dies, so it lives for several frames. Multiple flares can exist at various stages of cooling, but no more than `maxflare`. New flares are not created unless there is room. The default of 8 is pleasant for a 44-pixel wide matrix, but will likely look too busy and intense if you are using a narrower matrix, like a 16x16. As a rule of thumb, start with a value equal to about 1/4 of the display width, and then tweak it up or down for the look you want.
* `flarechance` - Even if there's room for a flare doesn't mean there will be one. This number sets the probability of a flare (0-100%). Lower numbers make a more tame-looking fire (I think it also looks more fakey).
* `flaredecay` - This sets the rate at which the flare's radiation decays. Smaller values yield more radiation; values under 10 will likely produce too much radiation and overwhelm the display (the smaller the display, the worse that will be); values over 20 will likely produce too little radiation to be interesting. The default is 14.

Other knobs you can turn:

* `BRIGHT` - Brightness of the display, from 0-255. This value is passed directly to `FastLED.setBrightness()` at initialization.
* `FPS` - (frames per second) the frequency of display updates. Lower values is a slower flame effect, and higher values make a roaring fire. The upper limit of this is a function of your processor speed and anything else you may have the processor doing.

If you're *really* into customization, you can change the color of the flames. This is done by replacing the values in the `colors` array. Just make sure element 0 is black. You can have as few or as many colors as you wish. Heat "decays" from the highest-numbered color to 0. As a rule of thumb, though, the number of colors should be +/-2 from the height of the display. If you have too few colors, the flames will die out closer to the bottom of the display. If you have too many, the flames will die out somewhere above the display, and the display will just look like a solid sheet of color.

## Watch Your Power!

A large LED matrix can draw substantial power. If you are powering your matrix from your microprocessor, and you've powered *it* from USB, then it's very easy for a large, bright display to swamp that power supply. This will cause a drop in voltage, and the microprocessor will crash/reset. To avoid this being a problem right at the start, the default brightness given to FastLED is pretty low. If you have a good power supply or if the display is small and its power budget fits your supply, you can increase this brightness (via the `BRIGHT` constant).

A better approach is to either power your matrix separately from a larger supply, or if possible (compatible voltages), power your microprocessor and matrix from the same larger supply. An "adequate" supply is one that will hold up the voltage within 5% of spec when all LEDs are going full color and full brightness (and the microprocessor is attached and running, if powered from that same supply). Regardless of your arrangement, just remember that **you must connect the ground of the matrix and its power supply together with the ground of your microprocessor**, or you may have unpredictable behavior or maybe worse, a more realistic fire effect than this code provides on its own.

## Customizing the `pos()` Function

If the given switches for display configuration don't cover your matrix product, you can code your own replacement for `pos()`. It needs to convert a canonical (col,row) to a pixel address. If the pixel at that address is illuminated, it should be "row" rows up from the bottom of the display and "col" columns from the left of the display. That is, the canonical form is (0,0) (origin) at the bottom of the display, with columns progressing from left to right, and rows from bottom to top as displayed. 

The 44x11 matrix I used for development, for example, has its original at upper-left with rows advancing downward (so opposite the desired canonical addressing), and just for added fun, columns in the "top" row progress from left to right, but the next row is *right-to-left*, and the row following back to left-to-right, and so on (a "zig-zag" arrangement of LEDs). All of this is handled by setting preprocessor macros at the top of the code, which are used in `pos()` to compute the correct pixel address. 

But I have not provided preprocessor values for every possible arrangement of LED matrix. For example, if the pixel at address 0 is the top left pixel, but pixel 1 is the first pixel in the next *row* rather than the next *column*, I have not provided a flag for that. You'll have to work it out. 

If you need to do surgery on `pos()` to make it work with the matrix you are using, just make sure that it returns (`int`) 0 for the bottom left corner pixel of the entire display, 1 for the next pixel to the *right*, and when it returns the value of `MAT_W` (display width), that should be the pixel immediately above the origin.

Your `pos()` implementation should also take into account that the displayed size of the animation may be portion of the matrix, and that the position of this "subwindow" in the display has its own origin. See "Sharing the Display" below for how that works.

Here are the global values you may need to perform the necessary calculation:

* `MAT_W` and `MAT_H` - the configured pixel width and height, respectively, of the entire matrix;
* `cols` and `rows` - the pixel width and height, respectively, of the subwindow in which the animation is displayed (defaults to `MAT_W` and `MAT_H` unless you change them);
* `xorg` and `yorg` - the offset column and row, respectively, of the origin of the subwindow in which the animation is displayed (default to 0,0 unless change them).

## Sharing the Display

It is possible for the fire simulation to share a large matrix with other displayed data. In order to do this, you need to set the following constants in the code:

* `cols` - Normally this is set to `MAT_W`, the full width of the matrix. If you are using the fire as a sub-display, set this constant to the desired width (must be \<= `MAT_W`).
* `rows` - Normally this is set to `MAT_H`, the full height of the matrix. Set this to the number of rows for your sub-display.
* `xorg` and `yorg` - Default 0, 0 respectively; the origin of the sub-display, offset from canonical (0,0) (bottom left corner).

Anything else you need to display with the fire simulation you can now set yourself directly into the `matrix` array. This array is `MAT_W * MAT_H` pixels long &mdash; the entire matrix. Whatever you do to the array, calling `FastLED.show()` will display it. In your program's `loop()` function, just call `make_fire()` either before or after all your other work to set your pixels. Make sure you call `make_fire()` often enough to uphold the configured refresh rate in the `FPS` constant.

## Using PixelTest

PixelTest is a simple sketch that displays a series of patterns meant to expose the arrangement of pixels in the matrix. By watching how the pixels display, you can determine the arrangement of the matrix and get MatrixFireFast configured correctly.

Before you run PixelTest, you will need to set (in the sketch) the matrix data pin and its width and height in the same manner as described for MatrixFireFast in "Basic Configuration" above.

If you are using a separate (non-USB) power supply, you may also want to turn on `BRIGHTNESS_TEST` and set `BRIGHT` (0-255) as well. If enabled, the brightness test will allow you to measure the worst-case current draw of your matrix at full brightness with all pixels on (and white). Since this test can crash a USB-powered matrix configuration, it is normally disabled in the distribution. A good way to do this is to simply use a good-quality bench power supply, as these usually display the (ampere) measurement of the load. This test also allows you to try out various values for `BRIGHT` to see what you might want to use in MatrixFireFast.

> NOTE: Make sure the matrix is in the orientation in which you want to install it before starting the test. The advice in this section assumes you are looking at the matrix in the same orientation as that in which it will be installed.

Download the configured sketch to the micro. PixelTest will begin by blinking the first pixel, the "origin" pixel 0, for 15 seconds. If pixel 0 is anywhere in the *top* row of the matrix, you need to *define* `MAT_TOP` in MatrixFireFast (e.g. `#define MAT_TOP`). Otherwise, undefine it (e.g. `#undef MAT_TOP`). If pixel 0 is on the *left* edge of the matrix, you should *define* `MAT_LEFT`; otherwise, undefine it.

PixelTest will next turn on (in various colors) the remaining pixels in the row. If the matrix displays a _horizontal line of pixels at the top or bottom edge_, proceed to the next step. Otherwise, you are most likely to see one or more vertical columns of pixels lit (and possibly a partial column). In this case, the display is arranged in column-major order. If that's the case, the easiest thing to do is to install the display rotated 90 degrees clockwise or counter-clockwise. Rotate the display, reconfigure (width and height are now swapped) and re-run PixelTest, and set the preprocessor macros according to those results. Alternately, you could write a custom `pos()` function if rotating the display is not possible or practical.

PixelTest will then try to bounce a single pixel up and down in a single column. The "ball" should move straight up and down in the same column, without changing columns as it ascends and descends. If the pixel jumps from left to right while moving up and down, you should define `MAT_ZIGZAG` in MatrixFireFast; otherwise (it keeps a single straight line), undefine it.

If you enabled the brightness test, PixelTest will now turn on all LEDs at your configured brightness (`BRIGHT`) and hold for 30 seconds. This will allow you to measure the maximum draw on your power supply with that brightness. If you have USB-powered your matrix through the microprocessor, and you have a large matrix, there's a good chance it will crash at this point. Don't use USB as a power supply; you need something more.

PixelTest will then start another test cycle.

> NOTE: If you get no display at all, you likely have the wrong data pin, it's not connected correctly, there's a power problem, or another configuration problem. Check that you are configured for the correct type of LED. Both PixelTest and MatrixFireFast are set for a NeoPixel LEDs by default. Change the `MATRIX_TYPE` macro to value that matches your matrix LED type from the [FastLED documentation](https://github.com/FastLED/FastLED/wiki/Overview). You will find the supported LED types and their respective type names.

## Donations

Donations in support of my projects are always greatly appreciated, regardless of size. You can [make a one-time donation here](https://www.toggledbits.com/donate).

## License

MatrixFastFire is licensed under the Gnu Public License version 3. Please see the LICENSE file.

Copyright 2020 Patrick H. Rigney, All Rights Reserved.
