/******************************************************************************

    dmx_handling

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

#include "./dmx_handling.h"

namespace dmx_handling {

bool dmx_valid = false;
uint16_t dmx_start_channel = 1;
uint8_t dmx_value = 0;


void setup(Print &out) {
    out.println(F("setup DMX:"));

    pinMode(dmx_pin_valid_led, OUTPUT);
    digitalWrite(dmx_pin_valid_led, HIGH);
    delay(100);
    digitalWrite(dmx_pin_valid_led, LOW);
    delay(100);
    digitalWrite(dmx_pin_valid_led, HIGH);
    delay(100);
    digitalWrite(dmx_pin_valid_led, LOW);
    delay(100);
    digitalWrite(dmx_pin_valid_led, HIGH);

    // set to receive mode
    Serial.println(F("\t init as DMXReceiver"));
    DMXSerial.init(DMXReceiver, dmx_pin_direction);

    // set to send mode
    // Serial.println(F("\t init as DMXController"));
    // DMXSerial.init(DMXController, dmx_pin_direction);

    // Serial.println(F("\t set some values"));
    // DMXSerial.write(10, 255);
    // DMXSerial.write(11, 255);
    // DMXSerial.write(12, 1);
    // read dmx values
    // DMXSerial.read(1);

    out.println(F("\t finished."));
}

void update() {
    bool dmx_valid_new = false;
    if (DMXSerial.noDataSince() < dmx_valid_timeout) {
        dmx_valid_new = true;
    }

    // check if dmx_valid state has changed
    if (dmx_valid != dmx_valid_new) {
        dmx_valid = dmx_valid_new;
        // print_mode(lcd);
        // print_DMXSignal(lcd);
    }

    if (dmx_valid) {
        digitalWrite(dmx_pin_valid_led, LOW);
        // check if values are new
        // this triggers really to much with no changing data...
        // if (DMXSerial.dataUpdated()) {
        //     DMXSerial.resetUpdated();
        //     print_DMXValues(lcd);
        // }

        // check if values are new
        // bool flag_new = false;
        // for (size_t i = 0; i < sizeof(dmx_values_old); i++) {
        //     uint8_t value_new = DMXSerial.read(dmx_start_channel + i);
        //     if (dmx_values_old[i] != value_new) {
        //         dmx_values_old[i] = value_new;
        //         flag_new = true;
        //     }
        // }
        // if (flag_new) {
        //     print_DMXValues(lcd);
        // }

        // uint8_t dmx_value_new = DMXSerial.read(dmx_start_channel);
        // set_value(dmx_value_new);
    } else {
        digitalWrite(dmx_pin_valid_led, HIGH);
        // set_value(
        //     map(
        //         analogRead(analog_pin),
        //          0, 1023,
        //          0, 255
        //     )
        // );
    }

    // combine 16bit value
    // uiDMXValue_Pan  = DMXSerial.read(11) << 8;
    // uiDMXValue_Pan  = uiDMXValue_Pan | DMXSerial.read(12);
    // read 8bit value
    // uiDMXValue_Pan  = DMXSerial.read(12);
}


}  // namespace dmx_handling
