/******************************************************************************

    effect engine

******************************************************************************/
/******************************************************************************
    The MIT License (MIT)

    Copyright (c) 2018 Stefan Krüger

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
******************************************************************************/

#include "./LEDBoard.h"

namespace LEDBoard {

Tlc59711 tlc(tlc_chips);
// const uint8_t clk_pin = 13;
// const uint8_t data_pin = 12;
// Tlc59711 tlc(tlc_chips, clk_pin, data_pin);

bool output_enabled = true;


void setup(const Print &out) {
    out.println(F("setup LEDBoards:"));

    out.println(F("\t init tlc lib"));
    LEDBoard::tlc.beginFast();
    out.println(F("\t start with leds off"));
    LEDBoard::tlc.setRGB();
    LEDBoard::tlc.write();
    out.println(F("\t set leds to 0, 0, 1"));
    LEDBoard::tlc.setRGB(0, 0, 1);
    LEDBoard::tlc.write();

    out.println(F("\t finished."));
}


}  // namespace LEDBoard
