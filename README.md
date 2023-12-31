# E-paper dashboard

ESP-32 application for displaying stock market data, weather data, date and news updates on a 7.5" Waveshare e-paper display.

![e-paper dashboard](img.jpeg)

## Components used:

 - ESP-32-WROOM microcontroller
 - Waveshare 7.5-inch V2 e-paper display HAT
 - DHT22 temperature and humidity sensor

## Utilities

The fonttool.py and imgtool.py files help create C header files with bitmaps from jpeg files. Files generated using them are in the _main/sprites_ diretory. 

The offsets for imgtool.py are relative to the 800x480 canvas of the e-paper display.

To use the fonttool.py, place all the JPEG files containign the glyphs in the same directory as the fonttool.py and run the fonttool program specifying the destination file name and the font size.

## Circuit layout

 - 3V3: D10
 - GND: K0, K0
 - IO27: J0
 - IO15: C3
 - IO26: I10
 - IO13: C10
 - IO14: E13
 - IO25: I0
 - IO19: G10
