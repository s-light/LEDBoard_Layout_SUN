/******************************************************************************

    dmx handling

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

#ifndef DMX_HANDLING_H_
#define DMX_HANDLING_H_

#include <Arduino.h>

#include <DMXSerial.h>

namespace dmx_handling {
const uint8_t dmx_pin_direction = 15;
const uint8_t dmx_pin_valid_led = 13;

// 3x16bit + 3x16bit + 1x16bit + 1x16bit + 2spare
const uint16_t dmx_maxchannel_count = (3*2)+(3*2)+(1*2)+(1*2)+2;

// timeout in milliseconds
const uint32_t dmx_valid_timeout = 1000;

extern bool dmx_valid;
extern uint16_t dmx_start_channel;
extern uint8_t dmx_value;
// uint8_t dmx_values_old[] = {0, 0, 0, 0};

void setup(Print &out);
void update();

}  // namespace dmx_handling

#endif  // DMX_HANDLING_H_
