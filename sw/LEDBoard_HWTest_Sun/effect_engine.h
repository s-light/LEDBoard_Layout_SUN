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

#ifndef EFFECT_ENGINE_H_
#define EFFECT_ENGINE_H_

#include <Arduino.h>

#include "LEDBoard.h"
#include "dmx_handling.h"

#include <FastLED.h>

namespace effect_engine {
enum sequencer_modes {
    sequencer_OFF,
    sequencer_CHANNELCHECK,
    sequencer_HORIZONTAL,
    sequencer_HORIZONTAL4,
    sequencer_SPIRAL,
    sequencer_SPIRAL2,
    sequencer_SUN_SPIRAL,
    sequencer_HPLINE,
    sequencer_SUN_WAVE_BLUE,
    sequencer_SUN_WAVE_ORANGE,
};

extern sequencer_modes sequencer_mode;

extern uint16_t value_low;
extern uint16_t value_high;

extern bool sequencer_running;
extern uint32_t sequencer_interval;
extern uint32_t calculate_interval;

extern int16_t sequencer_current_step;
extern uint8_t sequencer_direction_forward;


extern uint16_t sequencer_color[];

const uint8_t tail_simple_count = 3;
const uint16_t tail_simple[tail_simple_count][LEDBoard::colors_per_led] {
    //  red, green,   blue
    {     0,     0,     0},
    { 55000,     0, 20000},
    {     0,     0,     0},
};

const uint8_t tail_pink_count = 5;
const uint16_t tail_pink[tail_pink_count][LEDBoard::colors_per_led] {
    //  red, green,   blue
    {     0,     0,     0},
    {     0, 10000, 20000},
    { 55000,     0, 20000},
    { 55000, 10000,     0},
    {     0,     0,     0},
};

const uint8_t tail_orange_count = 10;
const uint16_t tail_orange[tail_orange_count][LEDBoard::colors_per_led] {
    //  red, green,   blue
    {     0,     0,     0},
    { 55000, 20000,     0},
    { 20000,  7200,     0},
    {  8000,  2900,     0},
    {  3000,  1000,     0},
    {  1000,   360,     0},
    {   500,   200,     0},
    {    50,    20,     0},
    {     5,     2,     0},
    {     0,     0,     0},
};

const uint8_t tail_water_count = 8;
const uint16_t tail_water[tail_water_count][LEDBoard::colors_per_led] {
    //  red, green,   blue
    {     0,     0,     0},
    {     0, 20000, 60000},
    {     0,  7000, 20000},
    {     0,  2000,  6000},
    {     0,  1000,  3000},
    {     0,   200,   600},
    {     0,    20,    60},
    {     0,     0,     0},
};

const uint8_t tail_sun_count = 12;
const uint16_t tail_sun[tail_sun_count][LEDBoard::colors_per_led] {
    //  red, green,   blue
    {   600,   180,     0},
    {  6000,  1800,     0},
    {  3000,   900,     0},
    {  1000,   300,     0},
    {  1000,   300,     0},
    {   600,   180,     0},
    {   600,   180,     0},
    {   600,   180,     0},
    {   600,   180,     0},
    {   600,   180,     0},
    {   600,   180,     0},
    {   600,   180,     0},
};
// const uint8_t tail_sun_count = 14;
// const uint16_t tail_sun[tail_sun_count][LEDBoard::colors_per_led] {
//     //  red, green,   blue
//     { 60000, 18000,     0},
//     { 30000,  9000,     0},
//     { 10000,  3000,     0},
//     {  6000,  1800,     0},
//     {  3000,   900,     0},
//     {  1000,   300,     0},
//     {  1000,   300,     0},
//     {   600,   180,     0},
//     {   600,   180,     0},
//     {   600,   180,     0},
//     {   600,   180,     0},
//     {   600,   180,     0},
//     {   600,   180,     0},
//     {   600,   180,     0},
// };



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// function definitions

void sequencer_off();

void calculate_step();
void calculate_step_next();

void update();

void toggle_sequencer(
    Print &out,
    sequencer_modes sequence,
    uint32_t interval
);

void test_update(Print &out);

// void set_hsv_color(uint16_t hue, uint16_t saturation, uint16_t value);
void set_hsv_color(uint8_t hue, uint8_t saturation, uint8_t value);

void print_aligned_int8(Print &out, const int8_t value);
void print_aligned_int16(Print &out, const int16_t value);

}  // namespace effect_engine

#endif  // EFFECT_ENGINE_H_
