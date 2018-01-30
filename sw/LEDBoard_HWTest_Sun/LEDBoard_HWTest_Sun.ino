/******************************************************************************

    LEDBoard_HWTest_Sun
        test software for LEDBoard_4x4_16bit in mounting_sun
        debugout on usbserial interface: 115200baud

    hardware:
        Board:
            Arduino compatible (with serial port)
            with big RAM
            best fit: ATmega1284
            LED on pin 13
        Connections:
            D4 --> push button to GND
            SCK --> Clock input of TLC5791
            MOSI --> Data input of TLC5791
            only if TLC5791 is used with 5V@ VCC and VREG
            see www.ti.com/lit/ds/symlink/tlc5971.pdf
            page31 9.3 System Examples - Figure 38
            (eventually use a 74HC4050 as Levelshifter 5V-->3.8V
            if you use different VCC/VREG for Chips.)


    libraries used:
        ~ slight_DebugMenu
        ~ slight_Button
        ~ slight_FaderLin
            License: MIT
            written by stefan krueger (s-light),
                github@s-light.eu, http://s-light.eu, https://github.com/s-light/
        ~ Tlc59711.h
            License: MIT
            Copyright (c) 2016 Ulrich Stern
            https://github.com/ulrichstern/Tlc59711

    written by stefan krueger (s-light),
        github@s-light.eu, http://s-light.eu, https://github.com/s-light/

    changelog / history
        check git commit messages

    TO DO:
        ~ xx


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

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Includes
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// use "file.h" for files in same directory as .ino
// #include "file.h"
// use <file.h> for files in library directory
// #include <file.h>

#include <slight_DebugMenu.h>
#include <slight_FaderLin.h>
#include <slight_ButtonInput.h>

#include <Tlc59711.h>


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Info
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void sketchinfo_print(Print &out) {
    out.println();
    //             "|~~~~~~~~~|~~~~~~~~~|~~~..~~~|~~~~~~~~~|~~~~~~~~~|"
    out.println(F("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"));
    out.println(F("|                       ^ ^                      |"));
    out.println(F("|                      (0,0)                     |"));
    out.println(F("|                      ( _ )                     |"));
    out.println(F("|                       \" \"                      |"));
    out.println(F("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"));
    out.println(F("| LEDBoard_HWTest_Sun.ino"));
    out.println(F("|   Test LEDBoard_Layout_Sun"));
    out.println(F("|"));
    out.println(F("| This Sketch has a debug-menu:"));
    out.println(F("| send '?'+Return for help"));
    out.println(F("|"));
    out.println(F("| dream on & have fun :-)"));
    out.println(F("|"));
    out.println(F("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"));
    out.println(F("|"));
    //out.println(F("| Version: Nov 11 2013  20:35:04"));
    out.print(F("| version: "));
    out.print(F(__DATE__));
    out.print(F("  "));
    out.print(F(__TIME__));
    out.println();
    out.println(F("|"));
    out.println(F("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"));
    out.println();

    //out.println(__DATE__); Nov 11 2013
    //out.println(__TIME__); 20:35:04
}


// Serial.print to Flash: Notepad++ Replace RegEx
//     Find what:        Serial.print(.*)\("(.*)"\);
//     Replace with:    Serial.print\1\(F\("\2"\)\);



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// definitions (global)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Debug Output

boolean infoled_state = 0;
const byte infoled_pin = 1;

unsigned long debugOut_LiveSign_TimeStamp_LastAction = 0;
const uint16_t debugOut_LiveSign_UpdateInterval = 1000; //ms

boolean debugOut_LiveSign_Serial_Enabled = 0;
boolean debugOut_LiveSign_LED_Enabled = 1;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Menu

// slight_DebugMenu(Stream &in_ref, Print &out_ref, uint8_t input_length_new);
slight_DebugMenu myDebugMenu(Serial, Serial, 15);


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// LEDBoard


const uint8_t chips_per_board = 4;

const uint8_t colors_per_led = 3;
const uint8_t leds_per_chip = 4;

const uint8_t colorchannels_per_board = (
    colors_per_led * leds_per_chip * chips_per_board
);



// mounting sun specials
const uint8_t boards_count_sun_arms = (6 * 4);
const uint8_t boards_count_sun_center = (3 + 4 + 4 + 3);
// const uint8_t boards_count_sun_arms = (3 * 4);
// const uint8_t boards_count_sun_center = (3 + 0 + 0 + 0);
const uint8_t boards_count_sun = boards_count_sun_arms + boards_count_sun_center;
const uint16_t colorchannels_mounting_sun =  (colorchannels_per_board*boards_count_sun);
const uint16_t colorchannels_mounting_sun_center =  (colorchannels_per_board*boards_count_sun_center);



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

Tlc59711 tlc(tlc_chips);
// const uint8_t clk_pin = 13;
// const uint8_t data_pin = 12;
// Tlc59711 tlc(tlc_chips, clk_pin, data_pin);

bool output_enabled = true;




//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// sequencer

enum sequencer_modes {
    sequencer_OFF,
    sequencer_CHANNELCHECK,
    sequencer_HORIZONTAL,
    sequencer_HORIZONTAL4,
    sequencer_SPIRAL,
    sequencer_SPIRAL2,
    sequencer_SUN_SPIRAL,
    sequencer_HPLINE,
    sequencer_SUN_LINES,
};

uint8_t sequencer_mode = sequencer_OFF;

uint32_t sequencer_timestamp_last = millis();
uint32_t sequencer_interval = 1000; // ms

int16_t sequencer_current_step = 0;
uint8_t sequencer_direction_forward = true;


//
uint16_t value_low = 1;
uint16_t value_high = 1000;



const uint8_t tail_orange_count = 8;
const uint16_t tail_orange[tail_orange_count][colors_per_led] {
    //  red, green,   blue
    { 55000, 20000,      0},
    { 20000,  7200,      0},
    {  8000,  2900,      0},
    {  3000,  1000,      0},
    {  1000,   360,      0},
    {   500,   200,      0},
    {    50,    20,      0},
    {     5,     2,      0},
};

const uint8_t tail_water_count = 6;
const uint16_t tail_water[tail_water_count][colors_per_led] {
    //  red, green,   blue
    {     0, 20000, 60000},
    {     0,  7000, 20000},
    {     0,  2000,  6000},
    {     0,  1000,  3000},
    {     0,   200,   600},
    {     0,    20,    60},
};


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// FaderLin

const uint8_t myFaderRGB__channel_count = colors_per_led;
slight_FaderLin myFaderRGB(
    0, // byte cbID_New
    myFaderRGB__channel_count, // byte cbChannelCount_New
    myFaderRGB_callback_OutputChanged, // tCbfuncValuesChanged cbfuncValuesChanged_New
    myCallback_onEvent // tCbfuncStateChanged cbfuncStateChanged_New
);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// button

const uint8_t button_pin = 3;

slight_ButtonInput button(
    0,  // byte cbID_New
    button_pin,  // byte cbPin_New,
    button_getInput,  // tCbfuncGetInput cbfuncGetInput_New,
    button_onEvent,  // tcbfOnEvent cbfCallbackOnEvent_New,
      30,  // const uint16_t cwDuration_Debounce_New = 30,
     500,  // const uint16_t cwDuration_HoldingDown_New = 1000,
      50,  // const uint16_t cwDuration_ClickSingle_New =   50,
     500,  // const uint16_t cwDuration_ClickLong_New =   3000,
     500   // const uint16_t cwDuration_ClickDouble_New = 1000
);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// lowbat

const uint8_t bat_voltage_pin = A0;
const uint8_t lowbat_warning_pin = 3;

uint16_t bat_voltage = 420;

uint32_t lowbat_timestamp_last = millis();
uint32_t lowbat_interval = 1000;


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// other things..

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// functions
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// debug things

// freeRam found at
// http://forum.arduino.cc/index.php?topic=183790.msg1362282#msg1362282
// posted by mrburnette
int freeRam () {
    extern int __heap_start, *__brkval;
    int v;
    return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Menu System

// Main Menu
void handleMenu_Main(slight_DebugMenu *pInstance) {
    Print &out = pInstance->get_stream_out_ref();
    char *command = pInstance->get_command_current_pointer();
    // out.print("command: '");
    // out.print(command);
    // out.println("'");
    switch (command[0]) {
        case 'h':
        case 'H':
        case '?': {
            // help
            out.println(F("____________________________________________________________"));
            out.println();
            out.println(F("Help for Commands:"));
            out.println();
            out.println(F("\t '?': this help"));
            out.println(F("\t 'i': sketch info"));
            out.println(F("\t 'y': toggle DebugOut livesign print"));
            out.println(F("\t 'Y': toggle DebugOut livesign LED"));
            out.println(F("\t 'x': tests"));
            out.println();
            // out.println(F("\t 'A': Show 'HelloWorld' "));
            out.println(F("\t 'a': toggle sequencer"));
            out.println(F("\t 'a': toggle SPIRAL"));
            out.println(F("\t 'b': toggle SPIRAL2"));
            out.println(F("\t 'B': toggle SUN SPIRAL"));
            out.println(F("\t 'c': toggle HORIZONTAL"));
            out.println(F("\t 'C': toggle SUN LINES"));
            out.println();
            out.print(F("\t 'I': set sequencer interval 'i65535' ("));
            out.print(sequencer_interval);
            out.print(F(")"));
            out.println();
            out.print(F("\t 'v': set effect value_low 'v65535' ("));
            out.print(value_low);
            out.print(F(")"));
            out.println();
            out.print(F("\t 'V': set effect value_high 'V65535' ("));
            out.print(value_high);
            out.print(F(")"));
            out.println();
            out.println();
            out.println(F("\t 's': set channel 's1:65535'"));
            // out.println(F("\t 'f': DemoFadeTo(ID, value) 'f1:65535'"));
            out.println();
            out.println(F("____________________________________________________________"));
        } break;
        case 'i': {
            sketchinfo_print(out);
        } break;
        case 'y': {
            out.println(F("\t toggle DebugOut livesign Serial:"));
            debugOut_LiveSign_Serial_Enabled = !debugOut_LiveSign_Serial_Enabled;
            out.print(F("\t debugOut_LiveSign_Serial_Enabled:"));
            out.println(debugOut_LiveSign_Serial_Enabled);
        } break;
        case 'Y': {
            out.println(F("\t toggle DebugOut livesign LED:"));
            debugOut_LiveSign_LED_Enabled = !debugOut_LiveSign_LED_Enabled;
            out.print(F("\t debugOut_LiveSign_LED_Enabled:"));
            out.println(debugOut_LiveSign_LED_Enabled);
        } break;
        case 'x': {
            // get state
            out.println(F("__________"));
            out.println(F("Tests:"));

            out.println(F("nothing to do."));

            // uint16_t wTest = 65535;
            uint16_t wTest = atoi(&command[1]);
            out.print(F("wTest: "));
            out.print(wTest);
            out.println();

            out.print(F("1: "));
            out.print((byte)wTest);
            out.println();

            out.print(F("2: "));
            out.print((byte)(wTest>>8));
            out.println();

            out.println();

            out.println(F("__________"));
        } break;
        //---------------------------------------------------------------------
        // case 'A': {
        //     out.println(F("\t Hello World! :-)"));
        // } break;
        case 'a': {
            out.println(F("\t toggle sequencer:"));
            if (sequencer_mode == sequencer_CHANNELCHECK) {
                sequencer_mode = sequencer_OFF;
                out.print(F("\t sequencer_mode: OFF\n"));
            }
            else {
                sequencer_mode = sequencer_CHANNELCHECK;
                out.print(F("\t sequencer_mode: CHANNELCHECK\n"));
            }
        } break;
        case 'A': {
            out.println(F("\t toggle SPIRAL:"));
            if (sequencer_mode == sequencer_SPIRAL) {
                sequencer_mode = sequencer_OFF;
                out.print(F("\t sequencer_mode: OFF\n"));
            }
            else {
                sequencer_mode = sequencer_SPIRAL;
                out.print(F("\t sequencer_mode: SPIRAL\n"));
            }
        } break;
        case 'b': {
            out.println(F("\t toggle SPIRAL2:"));
            if (sequencer_mode == sequencer_SPIRAL2) {
                sequencer_mode = sequencer_OFF;
                out.print(F("\t sequencer_mode: OFF\n"));
            }
            else {
                sequencer_mode = sequencer_SPIRAL2;
                out.print(F("\t sequencer_mode: SPIRAL2\n"));
                sequencer_interval = 100;
            }
        } break;
        case 'B': {
            out.println(F("\t toggle SUN SPIRAL:"));
            if (sequencer_mode == sequencer_SPIRAL) {
                sequencer_mode = sequencer_OFF;
                out.print(F("\t sequencer_mode: OFF\n"));
            }
            else {
                sequencer_mode = sequencer_SUN_SPIRAL;
                out.print(F("\t sequencer_mode: SUN SPIRAL\n"));
                sequencer_interval = 100;
            }
        } break;
        case 'c': {
            out.println(F("\t toggle HORIZONTAL:"));
            if (sequencer_mode == sequencer_HORIZONTAL) {
                sequencer_mode = sequencer_OFF;
                out.print(F("\t sequencer_mode: OFF\n"));
            }
            else {
                sequencer_mode = sequencer_HORIZONTAL;
                out.print(F("\t sequencer_mode: HORIZONTAL\n"));
                sequencer_interval = 200;
            }
        } break;
        case 'C': {
            out.println(F("\t toggle SUN_LINES:"));
            if (sequencer_mode == sequencer_SUN_LINES) {
                sequencer_mode = sequencer_OFF;
                out.print(F("\t sequencer_mode: OFF\n"));
            }
            else {
                sequencer_mode = sequencer_SUN_LINES;
                out.print(F("\t sequencer_mode: SUN_LINES\n"));
                sequencer_interval = 100;
                sequencer_direction_forward = true;
            }
        } break;
        case 'I': {
            out.print(F("\t set sequencer interval "));
            // convert part of string to int
            // (up to first char that is not a number)
            uint8_t command_offset = 1;
            uint16_t value = atoi(&command[command_offset]);
            out.print(value);
            out.println();
            sequencer_interval = value;
        } break;
        case 'v': {
            out.print(F("\t set effect value_low"));
            // convert part of string to int
            // (up to first char that is not a number)
            uint8_t command_offset = 1;
            uint16_t value = atoi(&command[command_offset]);
            out.print(value);
            out.println();
            value_low = value;
        } break;
        case 'V': {
            out.print(F("\t set effect value_high"));
            // convert part of string to int
            // (up to first char that is not a number)
            uint8_t command_offset = 1;
            uint16_t value = atoi(&command[command_offset]);
            out.print(value);
            out.println();
            value_high = value;
        } break;
        // ------------------------------------------
        case 's': {
            out.print(F("\t set channel "));
            // convert part of string to int
            // (up to first char that is not a number)
            uint8_t command_offset = 1;
            uint8_t ch = atoi(&command[command_offset]);
            // convert single character to int representation
            // uint8_t id = &command[1] - '0';
            command_offset = 3;
            if (ch > 9) {
                command_offset = command_offset +1;
            }
            out.print(ch);
            out.print(F(" : "));
            uint16_t value = atoi(&command[command_offset]);
            out.print(value);
            out.println();

            if (output_enabled) {
                tlc.setChannel(ch, value);
                tlc.write();
            }
        } break;
        // case 'f': {
        //     out.print(F("\t DemoFadeTo "));
        //     // convert part of string to int
        //     // (up to first char that is not a number)
        //     uint8_t id = atoi(&command[1]);
        //     // convert single character to int representation
        //     // uint8_t id = &command[1] - '0';
        //     out.print(id);
        //     out.print(F(" : "));
        //     uint16_t value = atoi(&command[3]);
        //     out.print(value);
        //     out.println();
        //     //demo_fadeTo(id, value);
        //     tlc.setChannel()
        //     out.println(F("\t demo for parsing values --> finished."));
        // } break;
        //---------------------------------------------------------------------
        default: {
            if(strlen(command) > 0) {
                out.print(F("command '"));
                out.print(command);
                out.println(F("' not recognized. try again."));
            }
            pInstance->get_command_input_pointer()[0] = '?';
            pInstance->set_flag_EOC(true);
        }
    } // end switch

    // end Command Parser
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// LEDBoard


void setup_Boards(Print &out) {
    out.println(F("setup LEDBoards:"));

    out.println(F("\t init tlc lib"));
    tlc.beginFast();
    out.println(F("\t start with leds off"));
    tlc.setRGB();
    tlc.write();
    out.println(F("\t set leds to 0, 0, 1"));
    tlc.setRGB(0, 0, 1);
    tlc.write();

    out.println(F("\t finished."));
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Sequencer



// parse the pointer to the first element of the effect_map
void calculate_step__effectmap(
    const uint8_t *effect_map,
    uint8_t row_count,
    uint8_t column_count,
    uint8_t board_start_index,
    // const uint16_t tail[][colors_per_led],
    const uint16_t tail[][3],
    const uint8_t tail_count
) {
    // Serial.println("calculate_step__effectmap: ");

    uint16_t ch_offset = colorchannels_per_board * board_start_index;

    uint8_t boards_per_row = column_count / leds_per_row;
    uint8_t boards_per_column = row_count / leds_per_column;

    for (size_t row = 0; row < row_count; row++) {
        for (size_t column = 0; column < column_count; column++) {


            // Serial.print("step ");
            // Serial.print(sequencer_current_step);
            // Serial.print("; r");
            // Serial.print(row);
            // Serial.print("; c");
            // Serial.print(column);

            uint8_t pixel = 0;
            uint16_t ch = 0;

            uint8_t board_column = column;
            uint8_t board_row = row;

            if (column >= leds_per_row) {
                board_column = column % leds_per_row;
            }
            if (row >= leds_per_column) {
                board_row = row % leds_per_column;
            }

            // Serial.print("; br: ");
            // Serial.print(board_row);
            // Serial.print("; bc: ");
            // Serial.print(board_column);

            // calculate board offset count

            uint8_t boards_offset_row = (row / leds_per_column);
            uint8_t boards_offset_column = (column / leds_per_row);
            uint8_t boards_offset = (boards_offset_row * boards_per_row) + boards_offset_column;
            // Serial.print("; bor: ");
            // Serial.print(boards_offset_row);
            // Serial.print("; boc: ");
            // Serial.print(boards_offset_column);
            // Serial.print("; bo: ");
            // Serial.print(boards_offset);

            pixel = channel_position_map[board_row][board_column];
            ch = pixel * 3;
            // Serial.print("; ch: ");
            // Serial.print(ch);

            ch = ch + (colorchannels_per_board * (uint16_t)boards_offset);
            // Serial.print("; ch: ");
            // Serial.print(ch);

            // Serial.print("; ch_offset: ");
            // Serial.print(ch_offset + );

            // Serial.print("; pixel: ");
            // Serial.print(pixel);
            // Serial.print("; ch: ");
            // Serial.print(ch);


            // uint8_t effect_step = effect_map[row][column];
            uint8_t effect_step = effect_map[(row *column_count) + column];

            // if (effect_step == sequencer_current_step) {
            //     // Serial.print(" ON");
            //     // set pixel to high
            //     values[ch + 0] = 20000;
            //     values[ch + 1] = 55000;
            //     values[ch + 2] = 0;
            //     // values[ch + 0] = 1000;
            //     // values[ch + 1] = 4000;
            //     // values[ch + 2] = 0;
            // }
            // else {
            //     // set pixel to low
            //     values[ch + 0] = 0;
            //     values[ch + 1] = 0;
            //     values[ch + 2] = 0;
            // }

            // tail
            int8_t tail_step = effect_step - sequencer_current_step;

            if (sequencer_direction_forward) {
                // change tail direction
                tail_step = tail_count - tail_step;
            }

            // add offset
            ch = ch_offset + ch;

            if ((tail_step >= 0) && (tail_step < tail_count)) {
                tlc.setChannel(ch + 0, tail[tail_step][0]);
                tlc.setChannel(ch + 1, tail[tail_step][1]);
                tlc.setChannel(ch + 2, tail[tail_step][2]);
            }
            else {
                // set pixel to low
                tlc.setChannel(ch + 0, 0);
                tlc.setChannel(ch + 1, 0);
                tlc.setChannel(ch + 2, 0);
            }

            // Serial.println();

        }
    }

}





void sequencer_off() {
    Serial.println("sequencer_off");
    // uint16_t values[colorchannels_per_board];
    // // init array with 0
    // memset(values, 0, colorchannels_per_board);
    //
    // for (size_t ch = 0; ch < colorchannels_per_board; ch++) {
    //     values[ch] = value_low;
    // }
    tlc.setRGB(0, 0, value_low);

    // reset sequencer
    sequencer_current_step = 0;

    // now map values to tlc chips and write theme to output
    // map_to_allBoards(values);
}

void calculate_step__channelcheck() {
    // Serial.print("calculate_step__channelcheck: ");

    for (size_t ch = 0; ch < colorchannels_per_board; ch++) {
        if (ch == (uint8_t)sequencer_current_step) {
            // set pixel to high
            tlc.setChannel(ch, value_high);
        }
        else {
            // set pixel to low
            tlc.setChannel(ch, value_low);
        }
    }

    // prepare next step
    // Serial.print("sequencer_current_step: ");
    // Serial.println(sequencer_current_step);
    sequencer_current_step = sequencer_current_step + 1;
    if (sequencer_current_step >= colorchannels_per_board) {
        sequencer_current_step = 0;
    }
}

void calculate_step__horizontal() {
    // Serial.println("calculate_step__horizontal: ");

    for (size_t column = 0; column < leds_per_row; column++) {
        for (size_t row = 0; row < leds_per_column; row++) {

            uint8_t pixel = channel_position_map[column][row];
            uint8_t ch = pixel * 3;


            // tail
            int8_t tail_step = column - sequencer_current_step;
            tail_step = tail_water_count - tail_step;
            // if (!sequencer_direction_forward) {
            //     // change tail direction
            //     tail_step = tail_count - tail_step;
            // }
            //
            // add offset
            // ch = ch_offset + ch;

            if ((tail_step >= 0) && (tail_step < tail_water_count)) {
                tlc.setChannel(ch + 0, tail_water[tail_step][0]);
                tlc.setChannel(ch + 1, tail_water[tail_step][1]);
                tlc.setChannel(ch + 2, tail_water[tail_step][2]);
            }
            else {
                // set pixel to low
                tlc.setChannel(ch + 0, 0);
                tlc.setChannel(ch + 1, 0);
                tlc.setChannel(ch + 2, 0);
            }

        }
    }

    // prepare next step
    // Serial.print("sequencer_current_step: ");
    // Serial.println(sequencer_current_step);
    // sequencer_current_step = sequencer_current_step + 1;
    // if (sequencer_current_step >= leds_per_column) {
    //     sequencer_current_step = 0;
    // }
    if (sequencer_current_step > 0) {
        sequencer_current_step = sequencer_current_step - 1;
    }
    else {
        sequencer_current_step = leds_per_column-1;
    }
    // if (sequencer_direction_forward) {
    //     // forward
    //     if (sequencer_current_step >= leds_per_column-1 ) {
    //         sequencer_current_step = sequencer_current_step - 1;
    //         sequencer_direction_forward = false;
    //         // Serial.println("direction switch to backwards");
    //     }
    //     else {
    //         sequencer_current_step = sequencer_current_step + 1;
    //     }
    // }
    // else {
    //     // backwards
    //     if (sequencer_current_step == 0 ) {
    //         sequencer_current_step = sequencer_current_step + 1;
    //         sequencer_direction_forward = true;
    //         // Serial.println("direction switch to forward");
    //     }
    //     else {
    //         sequencer_current_step = sequencer_current_step - 1;
    //     }
    // }

}

void calculate_step__spiral() {
    // Serial.println("calculate_step__spiral: ");

    const uint8_t spiral_order[leds_per_column][leds_per_row] {
        { 0,  1,  2,  3},
        {11, 12, 13,  4},
        {10, 15, 14,  5},
        { 9,  8,  7,  6},
    };

    for (size_t column = 0; column < leds_per_column; column++) {
        for (size_t row = 0; row < leds_per_row; row++) {

            uint8_t pixel = channel_position_map[column][row];
            uint8_t ch = pixel * 3;

            // set pixel to low
            // values[ch + 0] = value_low;
            // values[ch + 1] = value_low;
            // values[ch + 2] = value_low;

            if (spiral_order[column][row] == (uint8_t)sequencer_current_step) {
                // set pixel to high
                tlc.setChannel(ch + 0, 0);
                tlc.setChannel(ch + 1, 0);
                tlc.setChannel(ch + 2, value_high);
            }
            else {
                // set pixel to low
                tlc.setChannel(ch + 0, 0);
                tlc.setChannel(ch + 1, value_low);
                tlc.setChannel(ch + 2, 0);
            }


        }
    }

    // prepare next step
    // Serial.print("sequencer_current_step: ");
    // Serial.println(sequencer_current_step);
    if (sequencer_direction_forward) {
        // forward
        if (sequencer_current_step >= (leds_per_column * leds_per_row)-1 ) {
            sequencer_current_step = sequencer_current_step - 1;
            sequencer_direction_forward = false;
            // Serial.println("direction switch to backwards");
        }
        else {
            sequencer_current_step = sequencer_current_step + 1;
        }
    }
    else {
        // backwards
        if (sequencer_current_step == 0 ) {
            sequencer_current_step = sequencer_current_step + 1;
            sequencer_direction_forward = true;
            // Serial.println("direction switch to forward");
        }
        else {
            sequencer_current_step = sequencer_current_step - 1;
        }
    }
    // Serial.print("next step: ");
    // Serial.println(sequencer_current_step);

}

void calculate_step__hpline() {
    // Serial.println("calculate_step__spiral: ");

    for (size_t column = 0; column < leds_per_column; column++) {

        uint8_t pixel = channel_position_map[0][column];
        uint8_t ch = pixel * 3;

        if (column == (uint8_t)sequencer_current_step) {
            // set pixel to high
            tlc.setChannel(ch + 0, 0);
            tlc.setChannel(ch + 1, 65535);
            tlc.setChannel(ch + 2, 65535);
        }
        else {
            // set pixel to low
            tlc.setChannel(ch + 0, 0);
            tlc.setChannel(ch + 1, 40000);
            tlc.setChannel(ch + 2, 65535);
        }

    }

    // prepare next step
    // Serial.print("sequencer_current_step: ");
    // Serial.println(sequencer_current_step);
    // sequencer_current_step = sequencer_current_step + 1;
    // if (sequencer_current_step >= leds_per_column) {
    //     sequencer_current_step = 0;
    // }
    if (sequencer_current_step == 0) {
        sequencer_current_step = leds_per_column-1;
    }
    else {
        sequencer_current_step = sequencer_current_step - 1;
    }

    // reset step numbers to range..
    if (sequencer_current_step >= leds_per_column) {
        sequencer_current_step = leds_per_column-1;
    }

}



void calculate_step__spiral2(uint8_t board_start_index = 0, bool flag_horizontal = false) {

    const uint8_t row_count_vertical = leds_per_column*2;
    const uint8_t column_count_vertical = leds_per_row;
    const uint8_t spiral_order_vertical[row_count_vertical][column_count_vertical] {
        { 7,  8,  9, 10},
        { 6, 25, 26, 11},
        { 5, 24, 27, 12},
        { 4, 23, 28, 13},

        { 3, 22, 29, 14},
        { 2, 21, 30, 15},
        { 1, 20, 31, 16},
        { 0, 19, 18, 17},
    };

    const uint8_t row_count_horizontal = leds_per_column;
    const uint8_t column_count_horizontal = leds_per_row*2;
    const uint8_t spiral_order_horizontal[row_count_horizontal][column_count_horizontal] {
        {17, 16, 15, 14,   13, 12, 11, 10},
        {18, 31, 30, 29,   28, 27, 26,  9},
        {19, 20, 21, 22,   23, 24, 25,  8},
        { 0,  1,  2,  3,    4,  5,  6,  7},
    };

    if (flag_horizontal) {
        calculate_step__effectmap(
            &spiral_order_horizontal[0][0],
            row_count_horizontal,
            column_count_horizontal,
            board_start_index,
            tail_orange,
            tail_orange_count
        );
    }
    else {
        calculate_step__effectmap(
            &spiral_order_vertical[0][0],
            row_count_vertical,
            column_count_vertical,
            board_start_index,
            tail_orange,
            tail_orange_count
        );
    }
}

void calculate_step__sun_spiral_center3(uint8_t board_start_index = 0) {
    // Serial.println("calculate_step__sun_spiral_center3: ");

    const uint8_t column_count = leds_per_row*3;
    const uint8_t row_count = leds_per_column*1;
    const uint8_t spiral_order[row_count][column_count] {
        { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11},
        {13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 12},
        {12, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13},
        {11, 10,  9,  8,  7,  6,  5,  4,  3,  2,  1,  0},
    };

    calculate_step__effectmap(
        &spiral_order[0][0],
        row_count,
        column_count,
        board_start_index,
        tail_orange,
        tail_orange_count
    );

}

void calculate_step__spiral2_next() {
    // Serial.println("calculate_step__spiral: ");

    // prepare next step
    // Serial.print("sequencer_current_step: ");
    // Serial.println(sequencer_current_step);
    const uint8_t effect_step_count = (leds_per_board * 2);
    if (sequencer_direction_forward) {
        // forward
        if (sequencer_current_step > effect_step_count ) {
            sequencer_current_step = sequencer_current_step - 1;
            sequencer_direction_forward = false;
            // Serial.println("direction switch to backwards");
        }
        else {
            sequencer_current_step = sequencer_current_step + 1;
        }
    }
    else {
        // backwards
        if (sequencer_current_step <= (tail_orange_count*-1) ) {
            sequencer_current_step = sequencer_current_step + 1;
            sequencer_direction_forward = true;
            // Serial.println("direction switch to forward");
        }
        else {
            sequencer_current_step = sequencer_current_step - 1;
        }
    }
    // Serial.print("next step: ");
    // Serial.println(sequencer_current_step);

}


void calculate_step__line4(uint8_t board_start_index = 0, bool flag_horizontal = false) {

    const uint8_t row_count_vertical = leds_per_column*4;
    const uint8_t column_count_vertical = leds_per_row;
    const uint8_t effect_order_vertical[row_count_vertical][column_count_vertical] {
        { 0,  0,  0,  0},
        { 1,  1,  1,  1},
        { 2,  2,  2,  2},
        { 3,  3,  3,  3},

        { 4,  4,  4,  4},
        { 5,  5,  5,  5},
        { 6,  6,  6,  6},
        { 7,  7,  7,  7},

        { 8,  8,  8,  8},
        { 9,  9,  9,  9},
        {10, 10, 10, 10},
        {11, 11, 11, 11},

        {12, 12, 12, 12},
        {13, 13, 13, 13},
        {14, 14, 14, 14},
        {15, 15, 15, 15},

    };

    const uint8_t row_count_horizontal = leds_per_column;
    const uint8_t column_count_horizontal = leds_per_row*4;
    const uint8_t effect_order_horizontal[row_count_horizontal][column_count_horizontal] {
        { 0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0},
        { 1,  1,  1,  1,    1,  1,  1,  1,    1,  1,  1,  1,    1,  1,  1,  1},
        { 2,  2,  2,  2,    2,  2,  2,  2,    2,  2,  2,  2,    2,  2,  2,  2},
        { 3,  3,  3,  3,    3,  3,  3,  3,    3,  3,  3,  3,    3,  3,  3,  3},
    };

    if (flag_horizontal) {
        calculate_step__effectmap(
            &effect_order_horizontal[0][0],
            row_count_horizontal,
            column_count_horizontal,
            board_start_index,
            tail_water,
            tail_water_count
        );
    }
    else {
        calculate_step__effectmap(
            &effect_order_vertical[0][0],
            row_count_vertical,
            column_count_vertical,
            board_start_index,
            tail_water,
            tail_water_count
        );
    }
}

void calculate_step__line_center2x4(uint8_t board_start_index = 0) {
    // const uint8_t row_count = leds_per_column;
    // const uint8_t column_count = leds_per_row*8;
    // const uint8_t effect_order[row_count][column_count] {
    //     { 0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,    3,  3,  3,  3,    3,  3,  3,  3,    3,  3,  3,  3,    3,  3,  3,  3},
    //     { 1,  1,  1,  1,    1,  1,  1,  1,    1,  1,  1,  1,    1,  1,  1,  1,    2,  2,  2,  2,    2,  2,  2,  2,    2,  2,  2,  2,    2,  2,  2,  2},
    //     { 2,  2,  2,  2,    2,  2,  2,  2,    2,  2,  2,  2,    2,  2,  2,  2,    1,  1,  1,  1,    1,  1,  1,  1,    1,  1,  1,  1,    1,  1,  1,  1},
    //     { 3,  3,  3,  3,    3,  3,  3,  3,    3,  3,  3,  3,    3,  3,  3,  3,    0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0},
    // };
    const uint8_t row_count = leds_per_column*2;
    const uint8_t column_count = leds_per_row*4;
    const uint8_t effect_order[row_count][column_count] {
        { 0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0},
        { 1,  1,  1,  1,    1,  1,  1,  1,    1,  1,  1,  1,    1,  1,  1,  1},
        { 2,  2,  2,  2,    2,  2,  2,  2,    2,  2,  2,  2,    2,  2,  2,  2},
        { 3,  3,  3,  3,    3,  3,  3,  3,    3,  3,  3,  3,    3,  3,  3,  3},
        { 3,  3,  3,  3,    3,  3,  3,  3,    3,  3,  3,  3,    3,  3,  3,  3},
        { 2,  2,  2,  2,    2,  2,  2,  2,    2,  2,  2,  2,    2,  2,  2,  2},
        { 1,  1,  1,  1,    1,  1,  1,  1,    1,  1,  1,  1,    1,  1,  1,  1},
        { 0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0},
    };

    calculate_step__effectmap(
        &effect_order[0][0],
        row_count,
        column_count,
        board_start_index,
        tail_water,
        tail_water_count
    );
}

void calculate_step__line_center3(uint8_t board_start_index = 0, bool flag_mirror = false) {
    const uint8_t row_count_horizontal = leds_per_column;
    const uint8_t column_count_horizontal = leds_per_row*3;
    const uint8_t effect_order_horizontal_mirror[row_count_horizontal][column_count_horizontal] {
        {7,  7,    7,  7,  7,  7,    7,  7,  7,  7,    7,  7},
        {6,  6,    6,  6,  6,  6,    6,  6,  6,  6,    6,  6},
        {5,  5,    5,  5,  5,  5,    5,  5,  5,  5,    5,  5},
        {4,  4,    4,  4,  4,  4,    4,  4,  4,  4,    4,  4},
    };
    const uint8_t effect_order_horizontal[row_count_horizontal][column_count_horizontal] {
        {4,  4,    4,  4,  4,  4,    4,  4,  4,  4,    4,  4},
        {5,  5,    5,  5,  5,  5,    5,  5,  5,  5,    5,  5},
        {6,  6,    6,  6,  6,  6,    6,  6,  6,  6,    6,  6},
        {7,  7,    7,  7,  7,  7,    7,  7,  7,  7,    7,  7},
    };

    if (flag_mirror) {
        calculate_step__effectmap(
            &effect_order_horizontal_mirror[0][0],
            row_count_horizontal,
            column_count_horizontal,
            board_start_index,
            tail_water,
            tail_water_count
        );
    }
    else {
        calculate_step__effectmap(
            &effect_order_horizontal[0][0],
            row_count_horizontal,
            column_count_horizontal,
            board_start_index,
            tail_water,
            tail_water_count
        );
    }
}

void calculate_step__line4_next() {
    // prepare next step
    // Serial.print("sequencer_current_step: ");
    // Serial.println(sequencer_current_step);
    const uint8_t effect_step_count = (16);
    // without direction change
    // 'forward'
    if (sequencer_current_step < effect_step_count ) {
        sequencer_current_step = sequencer_current_step + 1;
    }
    else {
        sequencer_current_step = (tail_water_count*-1)+1;
        // Serial.println("line4_next: start new itteration");
    }
    // backwards
    // sequencer_direction_forward = false;
    // if (sequencer_current_step <= (tail_water_count*-1) ) {
    //     sequencer_current_step = effect_step_count;
    //     Serial.println("line4_next: start new itteration");
    // }
    // else {
    //     sequencer_current_step = sequencer_current_step - 1;
    // }

    // with direction change
    // if (sequencer_direction_forward) {
    //     // forward
    //     if (sequencer_current_step > effect_step_count ) {
    //         sequencer_current_step = sequencer_current_step - 1;
    //         sequencer_direction_forward = false;
    //         // Serial.println("direction switch to backwards");
    //     }
    //     else {
    //         sequencer_current_step = sequencer_current_step + 1;
    //     }
    // }
    // else {
    //     // backwards
    //     if (sequencer_current_step <= (tail_water_count*-1) ) {
    //         sequencer_current_step = sequencer_current_step + 1;
    //         sequencer_direction_forward = true;
    //         // Serial.println("direction switch to forward");
    //     }
    //     else {
    //         sequencer_current_step = sequencer_current_step - 1;
    //     }
    // }

    // Serial.print("next step: ");
    // Serial.println(sequencer_current_step);

}


void calculate_step_mounting_sun_spiral() {
    // Serial.print("calculate_step: ");

    uint8_t board_start_index = 0;

    // create animaiton in center
    // first row (3 boards)
    calculate_step__sun_spiral_center3(board_start_index);
    board_start_index += 3;


    // // second row (4 boards)
    calculate_step__spiral2(board_start_index, true);
    board_start_index += 2;
    calculate_step__spiral2(board_start_index, true);
    board_start_index += 2;

    // thirt row (4 boards)
    calculate_step__spiral2(board_start_index, true);
    board_start_index += 2;
    calculate_step__spiral2(board_start_index, true);
    board_start_index += 2;

    // fourth row (3 boards)
    calculate_step__sun_spiral_center3(board_start_index);
    board_start_index += 3;


    // use spiral2 for arms
    calculate_step__spiral2(board_start_index);
    // copy to all arms
    map_to_nBoards(board_start_index, boards_count_sun_arms, 2);

    calculate_step__spiral2_next();

}

void calculate_step_mounting_sun_lines() {
    // Serial.print("calculate_step: ");

    uint8_t board_start_index = 0;

    // create animaiton in center
    // first row (3 boards)
    calculate_step__line_center3(board_start_index, false);
    board_start_index += 3;

    // second row (4 boards)
    // thirt row (4 boards)
    calculate_step__line_center2x4(board_start_index);
    board_start_index += 4;
    board_start_index += 4;

    // fourth row (3 boards)
    calculate_step__line_center3(board_start_index, true);
    board_start_index += 3;

    // use spiral2 for arms
    calculate_step__line4(board_start_index);
    // copy to all arms
    map_to_nBoards(board_start_index, boards_count_sun_arms, 4);

    calculate_step__line4_next();

}



void calculate_step_singleboard() {
    // Serial.print("calculate_step: ");

    // we use the a part of our global memory.
    // uint16_t values_dualboard[colorchannels_per_board];
    // init array with 0
    // memset(, 0, colorchannels_per_board);

    // deside what sequencer we want to run

    switch (sequencer_mode) {
        case sequencer_OFF: {
            // 1;
        } break;
        case sequencer_CHANNELCHECK: {
            calculate_step__channelcheck();
        } break;
        case sequencer_HORIZONTAL: {
            calculate_step__horizontal();
        } break;
        case sequencer_SPIRAL: {
            calculate_step__spiral();
        } break;
        case sequencer_HPLINE: {
            calculate_step__hpline();
        } break;
    }

    // debug out print array:
    // Serial.print("values_global: ");
    // slight_DebugMenu::print_uint16_array(
    //     Serial,
    //     values_global,
    //     colorchannels_per_board
    // );
    // Serial.println();

    // now map to all tlc chips and write theme to output
    map_to_allBoards();
}

void calculate_step_dualboard() {
    // Serial.print("calculate_step: ");
    // deside what sequencer we want to run
    switch (sequencer_mode) {
        case sequencer_OFF: {
            // 1;
        } break;
        case sequencer_SPIRAL2: {
            calculate_step__spiral2();
            calculate_step__spiral2_next();
        } break;
        case sequencer_HORIZONTAL4: {
            calculate_step__line4();
        } break;
    }
    // now map to all tlc chips and write theme to output
    map_to_nBoards(0, boards_count, 2);
}


void calculate_step() {
    // Serial.println("calculate_step ");
    switch (sequencer_mode) {
        case sequencer_OFF: {
            // 1;
        } break;
        case sequencer_CHANNELCHECK:
        case sequencer_HORIZONTAL:
        case sequencer_SPIRAL:
        case sequencer_HPLINE: {
            calculate_step_singleboard();
        } break;
        case sequencer_SPIRAL2:
        case sequencer_HORIZONTAL4: {
            calculate_step_dualboard();
        } break;
        case sequencer_SUN_SPIRAL: {
            calculate_step_mounting_sun_spiral();
        } break;
        case sequencer_SUN_LINES: {
          calculate_step_mounting_sun_lines();
        } break;
    }
    if (sequencer_mode > sequencer_OFF) {
        // write data to chips
        tlc.write();
    }

}




void map_to_allBoards() {
    if (output_enabled) {
        // set all channels (mapping)
        for (
            size_t channel_index = 0;
            channel_index < colorchannels_per_board;
            channel_index++
        ) {
            // uint8_t mapped_channel = mapping_single_board[i];
            // Serial.print("mapping: ");
            // Serial.print(i);
            // Serial.print("-->");
            // Serial.print(mapped_channel);
            // Serial.println();
            for (size_t board_index = 0; board_index < boards_count; board_index++) {
                tlc.setChannel(
                    channel_index + (tlc_channels_per_board * board_index),
                    tlc.getChannel(channel_index)
                );
            }
        }
    }
}

void map_to_nBoards(
  uint8_t board_start_index,
  uint8_t boards_count_local,
  uint8_t boards_per_copy
) {
    if (output_enabled) {
        // set all channels (mapping)
        for (
            size_t board_index = board_start_index;
            board_index < (board_start_index + boards_count_local);
            board_index += boards_per_copy
        ) {
            // Serial.print("bi: ");
            // Serial.print(board_index);
            // Serial.println();
            // copy channels for both boards
            for (
                size_t channel_index = 0;
                channel_index < colorchannels_per_board*boards_per_copy;
                channel_index++
            ) {
                // uint8_t mapped_channel = mapping_single_board[i];
                // Serial.print("mapping: ");
                // Serial.print(i);
                // Serial.print("-->");
                // Serial.print(mapped_channel);
                // Serial.println();

                // get channel value
                uint16_t temp_value = tlc.getChannel(
                  channel_index + (tlc_channels_per_board * (board_start_index))
                );
                // set channel value
                tlc.setChannel(
                    channel_index + (tlc_channels_per_board * (board_index)),
                    temp_value
                );
            }
        }
    }
}



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// FaderLin

void setup_myFaderRGB(Print &out) {
    out.println(F("setup myFaderRGB:"));

    out.println(F("\t myFaderRGB.begin();"));
    myFaderRGB.begin();

    out.println(F("\t myFaderRGB welcome fade"));
    // myFaderRGB.startFadeTo( 1000, memList_A[memList_A__Full]);
    myFaderRGB_fadeTo(1000, 60000, 60000, 0);
    // run fading
    // while ( myFaderRGB.getLastEvent() == slight_FaderLin::event_fading_Finished) {
    //     myFaderRGB.update();
    // }
    while ( myFaderRGB.update() == slight_FaderLin::state_Fading) {
        // nothing to do.
    }
    myFaderRGB.update();

    // myFaderRGB.startFadeTo( 1000, memList_A[memList_A__Off]);
    myFaderRGB_fadeTo(1000, 1000, 500, 1);
    // run fading
    // while ( myFaderRGB.getLastEvent() != slight_FaderLin::event_fading_Finished) {
    //     myFaderRGB.update();
    // }
    while ( myFaderRGB.update() == slight_FaderLin::state_Fading) {
        // nothing to do.
    }
    myFaderRGB.update();

    // myFaderRGB.startFadeTo( 1000, memList_A[memList_A__Off]);
    myFaderRGB_fadeTo(1000, 0, 0, 1);
    // run fading
    // while ( myFaderRGB.getLastEvent() != slight_FaderLin::event_fading_Finished) {
    //     myFaderRGB.update();
    // }
    while ( myFaderRGB.update() == slight_FaderLin::state_Fading) {
        // nothing to do.
    }
    myFaderRGB.update();

    out.println(F("\t finished."));
}

void myFaderRGB_callback_OutputChanged(uint8_t id, uint16_t *values, uint8_t count) {

    // if (bDebugOut_myFaderRGB_Output_Enable) {
    //     Serial.print(F("OutputValue: "));
    //     printArray_uint16(Serial, wValues, bCount);
    //     Serial.println();
    // }

    // for (size_t channel_index = 0; channel_index < count; channel_index++) {
    //     tlc.setChannel(channel_index, values[channel_index]);
    // }

    if (output_enabled) {
        tlc.setRGB(values[0], values[1], values[2]);
        tlc.write();
    }

}

void myFaderRGB_fadeTo(uint16_t duration, uint16_t r, uint16_t g, uint16_t b) {
    uint16_t values[myFaderRGB__channel_count];
    // init array with 0
    values[0] = r;
    values[1] = g;
    values[2] = b;
    myFaderRGB.startFadeTo(duration, values);
}

void myCallback_onEvent(slight_FaderLin *pInstance, byte event) {


    // Serial.print(F("Instance ID:"));
    // Serial.println((*pInstance).getID());
    //
    // Serial.print(F("Event: "));
    // (*pInstance).printEvent(Serial, event);
    // Serial.println();


    // react on events:
    switch (event) {
        case slight_FaderLin::event_StateChanged : {
            // Serial.print(F("slight_FaderLin "));
            // Serial.print((*pInstance).getID());
            // Serial.println(F(" : "));
            // Serial.print(F("\t state: "));
            // (*pInstance).printState(Serial);
            // Serial.println();

            // switch (state) {
            //     case slight_FaderLin::state_Standby : {
            //             //
            //         } break;
            //     case slight_FaderLin::state_Fading : {
            //             //
            //         } break;
            //     case slight_FaderLin::state_Finished : {
            //             //
            //         } break;
            // } //end switch


        } break;

        case slight_FaderLin::event_fading_Finished : {
            // Serial.print(F("\t fading Finished."));
        } break;
    } //end switch

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// button callbacks

boolean button_getInput(uint8_t id, uint8_t pin) {
    // read input invert reading - button closes to GND.
    // check HWB
    // return ! (PINE & B00000100);
    return ! digitalRead(pin);
}

void button_onEvent(slight_ButtonInput *pInstance, byte bEvent) {
    // Serial.print(F("FRL button:"));
    // Serial.println((*pInstance).getID());
    //
    // Serial.print(F("Event: "));
    // Serial.print(bEvent);
    // // (*pInstance).printEvent(Serial, bEvent);
    // Serial.println();

    // uint8_t button_id = (*pInstance).getID();

    // show event additional infos:
    switch (bEvent) {
        // case slight_ButtonInput::event_StateChanged : {
        //     Serial.println(F("\t state: "));
        //     (*pInstance).printlnState(Serial);
        //     Serial.println();
        // } break;
        case slight_ButtonInput::event_Down : {
            // Serial.println(F("FRL down"));
        } break;
        case slight_ButtonInput::event_HoldingDown : {
            uint32_t duration = (*pInstance).getDurationActive();
            Serial.println(F("duration active: "));
            Serial.println(duration);
            if (duration <= 2000) {
                myFaderRGB_fadeTo(500, 10000, 0, 0);
            }
            else if (duration <= 3000) {
                myFaderRGB_fadeTo(500, 0, 10000, 0);
            }
            else if (duration <= 4000) {
                myFaderRGB_fadeTo(500, 0, 0, 10000);
            }
            else if (duration <= 6000) {
                myFaderRGB_fadeTo(500, 0, 65000, 65000);
            }
            else if (duration <= 7000) {
                myFaderRGB_fadeTo(500, 65000, 0, 65000);
            }
            else if (duration <= 8000) {
                myFaderRGB_fadeTo(500, 65535, 65535, 0);
            }
            else if (duration <= 9000) {
                myFaderRGB_fadeTo(1000, 65535, 65535, 65535);
            }
            else {
                myFaderRGB_fadeTo(1000, 65535, 65535, 65535);
            }

        } break;
        case slight_ButtonInput::event_Up : {
            Serial.println(F("up"));
            myFaderRGB_fadeTo(1000, 0, 0, 1);
        } break;
        case slight_ButtonInput::event_Click : {
            // Serial.println(F("FRL click"));
            if (sequencer_mode == sequencer_OFF) {
                sequencer_mode = sequencer_CHANNELCHECK;
                sequencer_interval = 500;
                Serial.print(F("\t sequencer_mode: CHANNELCHECK\n"));
            }
            else {
                sequencer_off();
                sequencer_mode = sequencer_OFF;
                Serial.print(F("\t sequencer_mode: OFF\n"));
            }

        } break;
        case slight_ButtonInput::event_ClickLong : {
            // Serial.println(F("click long"));
        } break;
        case slight_ButtonInput::event_ClickDouble : {
            // Serial.println(F("click double"));
            sequencer_mode = sequencer_HORIZONTAL;
            sequencer_interval = 1000;
            Serial.print(F("\t sequencer_mode: HORIZONTAL\n"));
        } break;
        case slight_ButtonInput::event_ClickTriple : {
            sequencer_mode = sequencer_SPIRAL;
            sequencer_interval = 100;
            Serial.print(F("\t sequencer_mode: SPIRAL\n"));
            // Serial.println(F("click triple"));
        } break;
        case slight_ButtonInput::event_ClickMulti : {
            Serial.print(F("click count: "));
            Serial.println((*pInstance).getClickCount());
            switch ((*pInstance).getClickCount()) {
                case 4 : {
                    sequencer_mode = sequencer_SPIRAL2;
                    sequencer_interval = 50;
                    Serial.print(F("\t sequencer_mode: SPIRAL 2boards\n"));
                } break;
                case 5 : {
                    sequencer_mode = sequencer_HPLINE;
                    sequencer_interval = 50;
                    Serial.print(F("\t sequencer_mode: High Power Line\n"));
                } break;
                case 6 : {
                    sequencer_mode = sequencer_SUN_SPIRAL;
                    sequencer_interval = 100;
                    Serial.print(F("\t sequencer_mode: SUN SPIRAL\n"));
                } break;
            }
        } break;
    }  // end switch
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// lowbat

void lowbat_check() {
    // handle lowbat
    if(
        (millis() - lowbat_timestamp_last) > lowbat_interval
    ) {
        lowbat_timestamp_last =  millis();
        uint16_t bat_input_raw = analogRead(bat_voltage_pin);
        uint16_t bat_voltage_raw = map(
            bat_input_raw,
            0, 1023,
            0, 500
        );
        bat_voltage = bat_voltage_raw + 13;
        // bat_voltage = map(
        //     bat_voltage_raw,
        //     0, 50,
        //     2, 52
        // );

        // Serial.print(F("bat input raw: "));
        // Serial.print(bat_input_raw);
        // Serial.println();

        // Serial.print(F("bat votlage raw: "));
        // Serial.print(bat_voltage_raw);
        // Serial.println();

        // Serial.print(F("bat votlage: "));
        // // Serial.print(bat_voltage);
        // // Serial.print(F(" --> "));
        // Serial.print(bat_voltage/100.0);
        // Serial.println(F("V"));

        if (bat_voltage > 340) {
            output_enabled = true;
            digitalWrite(lowbat_warning_pin, HIGH);
            // Serial.println(F("--> output enabled"));
        } else if (bat_voltage <= 310) {
            // force off
            output_enabled = false;
            digitalWrite(lowbat_warning_pin, LOW);
            tlc.setRGB(0, 0, 0);
            tlc.write();
            Serial.println(F("--> output disabled"));
        }
    }
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// other things..




//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// setup
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void setup() {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // initialise PINs

        //LiveSign
        pinMode(infoled_pin, OUTPUT);
        digitalWrite(infoled_pin, HIGH);

        pinMode(lowbat_warning_pin, OUTPUT);
        digitalWrite(lowbat_warning_pin, HIGH);

        // as of arduino 1.0.1 you can use INPUT_PULLUP

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // initialise serial

        // for ATmega32U4 devices:
        #if defined (__AVR_ATmega32U4__)
            // wait for arduino IDE to release all serial ports after upload.
            // delay(2000);
        #endif

        Serial.begin(115200);

        // for ATmega32U4 devices:
        #if defined (__AVR_ATmega32U4__)
            // Wait for Serial Connection to be Opend from Host or
            // timeout after 6second
            uint32_t timeStamp_Start = millis();
            while( (! Serial) && ( (millis() - timeStamp_Start) < 1000 ) ) {
                // nothing to do
            }
        #endif

        Print &out = Serial;
        out.println();

        out.print(F("# Free RAM = "));
        out.println(freeRam());

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // print welcome

        sketchinfo_print(out);

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // setup LEDBoard

        setup_Boards(out);

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // setup Fader

    out.print(F("# Free RAM = "));
    out.println(freeRam());

    setup_myFaderRGB(out);

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // setup button

    out.print(F("# Free RAM = "));
    out.println(freeRam());

    out.println(F("setup button:")); {
        out.println(F("\t set button pin"));
        pinMode(button_pin, INPUT_PULLUP);
        out.println(F("\t button begin"));
        button.begin();
    }
    out.println(F("\t finished."));

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // setup XXX1

        // out.print(F("# Free RAM = "));
        // out.println(freeRam());
        //
        // out.println(F("setup XXX1:")); {
        //
        //     out.println(F("\t sub action"));
        // }
        // out.println(F("\t finished."));

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // show serial commands

        // myDebugMenu.set_user_EOC_char(';');
        myDebugMenu.set_callback(handleMenu_Main);
        myDebugMenu.begin();

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // go

        out.println(F("Loop:"));

} /** setup **/

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// main loop
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void loop() {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // menu input
        myDebugMenu.update();

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // update sub parts

        myFaderRGB.update();
        button.update();

        // lowbat_check();

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // timed things

        if (sequencer_mode != sequencer_OFF) {
            if(
                (millis() - sequencer_timestamp_last) > sequencer_interval
            ) {
                sequencer_timestamp_last =  millis();
                calculate_step();
            }
        }



    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // debug output

        if (
            (millis() - debugOut_LiveSign_TimeStamp_LastAction) >
            debugOut_LiveSign_UpdateInterval
        ) {
            debugOut_LiveSign_TimeStamp_LastAction = millis();

            if ( debugOut_LiveSign_Serial_Enabled ) {
                Serial.print(millis());
                Serial.print(F("ms;"));
                Serial.print(F("  free RAM = "));
                Serial.print(freeRam());
                Serial.print(F("; bat votlage: "));
                Serial.print(bat_voltage/100.0);
                Serial.print(F("V"));
                Serial.println();
            }

            if ( debugOut_LiveSign_LED_Enabled ) {
                infoled_state = ! infoled_state;
                if (infoled_state) {
                    //set LED to HIGH
                    digitalWrite(infoled_pin, HIGH);
                } else {
                    //set LED to LOW
                    digitalWrite(infoled_pin, LOW);
                }
            }

        }

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // other things

} /** loop **/

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// THE END
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
