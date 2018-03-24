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

#ifndef LEDBOARD_H_
#define LEDBOARD_H_

#include <Arduino.h>

#include <Tlc59711.h>

namespace LEDBoard {
    const uint8_t chips_per_board = 4;

    const uint8_t colors_per_led = 3;
    const uint8_t leds_per_chip = 4;

    const uint8_t colorchannels_per_board = (
        colors_per_led * leds_per_chip * chips_per_board);


    // mounting sun specials
    const uint8_t boards_count_sun_arms = (6 * 4);
    const uint8_t boards_count_sun_center = (12 + 6 + 1);
    const uint8_t boards_count_sun =
        (boards_count_sun_arms + boards_count_sun_center);
    const uint16_t colorchannels_mounting_sun =
        (colorchannels_per_board*boards_count_sun);
    const uint16_t colorchannels_mounting_sun_center =
        (colorchannels_per_board*boards_count_sun_center);



    // const uint8_t boards_count = 12+3;
    const uint8_t boards_count = boards_count_sun;

    const uint8_t leds_per_row = 4;
    const uint8_t leds_per_column = 4;
    const uint8_t leds_per_board = leds_per_row * leds_per_column;

    const uint8_t channel_position_map[leds_per_column][leds_per_row] = {
        {10, 11, 14, 15},
        { 8,  9, 12, 13},
        { 2,  3,  6,  7},
        { 0,  1,  4,  5},
    };

    // tlc info
    const uint8_t tlc_channels = colors_per_led * leds_per_chip;
    const uint8_t tlc_channels_per_board = tlc_channels * chips_per_board;
    const uint8_t tlc_chips = boards_count * chips_per_board;
    const uint8_t tlc_channels_total = (uint8_t)(tlc_chips * tlc_channels);

    extern Tlc59711 tlc;
    // const uint8_t clk_pin = 13;
    // const uint8_t data_pin = 12;
    // Tlc59711 tlc(tlc_chips, clk_pin, data_pin);

    extern bool output_enabled;

    void setup(Print &out);

}  // namespace LEDBoard

#endif  // LEDBOARD_H_
