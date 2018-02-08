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
bool effect_control = false;


// const size_t values_count = dmx_maxchannel_count / (int16_t);
// const size_t values_count = 8;
size_t values_dirty = 0b00000000;
// int16_t values[values_count];
uint8_t values[values_count];

bool serial_out_enabled = false;
uint32_t serial_out_timestamp_last = 0;
uint16_t serial_out_interval = 1000;

void handle_new_values(Print &out);

size_t chname2chindex(channel_names name) {
    // return (name*2);
    return (name*1);
}


void print_values(Print &out) {
    char line[100];
    // snprintf(
    //     line,
    //     sizeof(line),
    //     "A: %6d %6d %6d M: %6d %6d %6d H: %6d T: %6d",
    //     values[ch_a_x],
    //     values[ch_a_y],
    //     values[ch_a_z],
    //     values[ch_m_x],
    //     values[ch_m_y],
    //     values[ch_m_z],
    //     values[ch_heading],
    //     values[ch_temp]);
    // out.println(line);
    snprintf(
        line,
        sizeof(line),
        "A: %3u %3u %3u H: %3u T: %3u",
        values[ch_a_x],
        values[ch_a_y],
        values[ch_a_z],
        values[ch_heading],
        values[ch_temp]);
    out.print(line);
}

void print_raw(Print &out) {
    char line[100];
    snprintf(
        line,
        sizeof(line),
        "[%3u, %3u, %3u, %3u, %3u, %3u, %3u, %3u, %3u, %3u]",
        DMXSerial.read(1),
        DMXSerial.read(2),
        DMXSerial.read(3),
        DMXSerial.read(4),
        DMXSerial.read(5),
        DMXSerial.read(6),
        DMXSerial.read(7),
        DMXSerial.read(8),
        DMXSerial.read(9),
        DMXSerial.read(10));
    out.print(line);
}






void setup(Print &out) {
    out.println(F("setup DMX:"));

    pinMode(dmx_pin_valid_led, OUTPUT);
    pinMode(dmx_pin_newdata_led, OUTPUT);
    digitalWrite(dmx_pin_valid_led, HIGH);
    digitalWrite(dmx_pin_newdata_led, HIGH);
    delay(100);
    digitalWrite(dmx_pin_valid_led, LOW);
    digitalWrite(dmx_pin_newdata_led, LOW);
    delay(100);
    digitalWrite(dmx_pin_valid_led, HIGH);
    digitalWrite(dmx_pin_newdata_led, HIGH);
    delay(100);
    digitalWrite(dmx_pin_valid_led, LOW);
    digitalWrite(dmx_pin_newdata_led, LOW);
    delay(100);
    digitalWrite(dmx_pin_valid_led, HIGH);
    digitalWrite(dmx_pin_newdata_led, HIGH);

    // set to receive mode
    Serial.println(F("\t init as DMXReceiver"));
    DMXSerial.init(DMXReceiver, dmx_pin_direction);

    // set to send mode
    // Serial.println(F("\t init as DMXController"));
    // DMXSerial.init(DMXController, dmx_pin_direction);

    Serial.print(F("\t set maxChannel "));
    Serial.println(dmx_maxchannel_count);
    DMXSerial.maxChannel(dmx_maxchannel_count);

    // Serial.println(F("\t set some values"));
    // DMXSerial.write(10, 255);
    // DMXSerial.write(11, 255);
    // DMXSerial.write(12, 1);
    // read dmx values
    // DMXSerial.read(1);

    out.println(F("\t finished."));
}

void update(Print &out) {
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
        for (size_t i = 0; i < values_count; i++) {
            size_t ch = chname2chindex(channel_names(i));
            // uint16_t value_new = 0;
            // value_new |= DMXSerial.read(dmx_start_channel + ch + 0) << 8;
            // value_new |= DMXSerial.read(dmx_start_channel + ch + 1);
            // if (values[i] != int16_t(value_new)) {
            //     values[i] = int16_t(value_new);
            //     bitSet(values_dirty, i);
            // } else {
            //     bitClear(values_dirty, i);
            // }
            uint8_t value_new = DMXSerial.read(dmx_start_channel + ch + 0);
            if (values[i] != value_new) {
                values[i] = value_new;
                bitSet(values_dirty, i);
            } else {
                bitClear(values_dirty, i);
            }
        }
        if (values_dirty) {
            digitalWrite(dmx_pin_newdata_led, LOW);
            // print_values(out);
            // out.println();
            handle_new_values(out);
        } else {
            digitalWrite(dmx_pin_newdata_led, HIGH);
        }
    } else {
        digitalWrite(dmx_pin_valid_led, HIGH);
    }

    if (serial_out_enabled) {
        if (
            (millis() - serial_out_timestamp_last) > serial_out_interval
        ) {
            serial_out_timestamp_last =  millis();
            // print_values(out);
            // out.println();
            print_raw(out);
            out.println();
        }
    }
    // combine 16bit value
    // uiDMXValue_Pan  = DMXSerial.read(11) << 8;
    // uiDMXValue_Pan  = uiDMXValue_Pan | DMXSerial.read(12);
    // read 8bit value
    // uiDMXValue_Pan  = DMXSerial.read(12);
}


void map_as_color() {
    // effect_engine::set_hsv_color(
    //     // hue
    //     map(
    //         constrain(values[ch_a_x], -15000, 15000),
    //         -15000, 15000, 0, 65535),
    //     // saturation
    //     65535,
    //     // value
    //     map(
    //         constrain(values[ch_heading], 0, 360),
    //         0, 360, 0, 65535)
    // );
    // effect_engine::set_hsv_color(
    //     // hue
    //     map(
    //         constrain(values[ch_a_x], -15000, 15000),
    //         -15000, 15000, 0, 255),
    //     // saturation
    //     255,
    //     // value
    //     map(
    //         constrain(values[ch_heading], 0, 360),
    //         0, 360, 0, 255));
    effect_engine::set_hsv_color(
        // hue
        values[ch_heading],
        // saturation
        255,
        // value
        120);
        // values[ch_heading]);
}

void map_as_sequencer_interval() {
    // effect_engine::sequencer_interval = map(
    //     constrain(values[ch_a_y], -15000, 15000),
    //     -15000, 15000,
    //     0, 2000);
    effect_engine::sequencer_interval = map(
        values[ch_a_y],
        0, 255,
        0, 2000);
}

// private functions
void handle_new_values(Print &out) {
    // for (size_t i = 0; i < values_count; i = i + 2) {
    //     if (bitRead(values_dirty, i)) {
    //         // clear
    //         bitClear(values_dirty, i);
    //         // handle
    //     }
    // }
    if (effect_control) {
        // if (bitRead(values_dirty, ch_heading)) {
        //     // clear
        //     bitClear(values_dirtx, ch_heading);
        //     // handle
        //     effect_engine::sequencer_interval = map(
        //         constrain(values[ch_heading], -15000, 15000),
        //         -15000, 15000,
        //         0, 2000);
        // }
        if (
            bitRead(values_dirty, ch_a_x) ||
            bitRead(values_dirty, ch_heading)
        ) {
            // clear
            bitClear(values_dirty, ch_a_x);
            bitClear(values_dirty, ch_heading);
            // handle
            print_raw(out);
            out.println(" map_as_color()");
            map_as_color();
        }
        if (bitRead(values_dirty, ch_a_y)) {
            // clear
            bitClear(values_dirty, ch_a_y);
            // handle
            print_raw(out);
            out.println(" map_as_sequencer_interval()");
            map_as_sequencer_interval();
        }
    }
}

}  // namespace dmx_handling
