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
        ~ DMXSerial
            Copyright (c) 2005-2012 by Matthias Hertel,
            http://www.mathertel.de
            license:
                See http://www.mathertel.de/License.aspx
                Software License Agreement (BSD License)

    written by stefan krueger (s-light),
        github@s-light.eu, http://s-light.eu, https://github.com/s-light/

    changelog / history
        check git commit messages

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

#include "LEDBoard.h"
#include "effect_engine.h"
#include "dmx_handling.h"

// create alias for namespace
namespace effe = effect_engine;

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

Print &DebugOut = Serial;
Stream &DebugIn = Serial;
// attention: in setup_DebugOut 'Serial' is hardcoded used for initialisation


boolean infoled_state = 0;
const byte infoled_pin = 1;

unsigned long debugOut_LiveSign_TimeStamp_LastAction = 0;
const uint16_t debugOut_LiveSign_UpdateInterval = 1000; //ms

boolean debugOut_LiveSign_Serial_Enabled = 0;
boolean debugOut_LiveSign_LED_Enabled = 1;

unsigned long debugOut_dmx_TimeStamp_LastAction = 0;
const uint16_t debugOut_dmx_UpdateInterval = 500; //ms

boolean debugOut_dmx_Enabled = 0;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Menu

// slight_DebugMenu(Stream &in_ref, Print &out_ref, uint8_t input_length_new);
slight_DebugMenu myDebugMenu(Serial, Serial, 15);

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
// FaderLin

const uint8_t myFaderRGB__channel_count = LEDBoard::colors_per_led;
slight_FaderLin myFaderRGB(
    0, // byte cbID_New
    myFaderRGB__channel_count, // byte cbChannelCount_New
    myFaderRGB_callback_OutputChanged, // tCbfuncValuesChanged cbfuncValuesChanged_New
    myCallback_onEvent // tCbfuncStateChanged cbfuncStateChanged_New
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
// DebugOut

// freeRam found at
// http://forum.arduino.cc/index.php?topic=183790.msg1362282#msg1362282
// posted by mrburnette
int freeRam () {
    extern int __heap_start, *__brkval;
    int v;
    return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

void setup_DebugOut(Print &out) {
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


    out.println();

    out.print(F("# Free RAM = "));
    out.println(freeRam());
}

void handle_debugout(Print &out) {
    if (
        (millis() - debugOut_LiveSign_TimeStamp_LastAction) >
        debugOut_LiveSign_UpdateInterval
    ) {
        debugOut_LiveSign_TimeStamp_LastAction = millis();

        if ( debugOut_LiveSign_Serial_Enabled ) {
            out.print(millis());
            out.print(F("ms;"));
            out.print(F("  free RAM = "));
            out.print(freeRam());
            // out.print(F("; bat votlage: "));
            // out.print(bat_voltage/100.0);
            // out.print(F("V"));
            out.println();
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
}

void handle_debugout_dmx(Print &out) {

    if ( debugOut_dmx_Enabled ) {
        if (
            (millis() - debugOut_dmx_TimeStamp_LastAction) >
            debugOut_dmx_UpdateInterval
        ) {
            debugOut_dmx_TimeStamp_LastAction = millis();
            dmx_handling::print_values(out);
            out.println();
            // dmx_handling::print_raw(out);
            // out.println();
        }
    }
}



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Menu System

void setup_DebugMenu(Print &out) {
    out.print(F("# Free RAM = "));
    out.println(freeRam());

    out.println(F("setup DebugMenu:")); {
        // myDebugMenu.set_user_EOC_char(';');
        myDebugMenu.set_callback(handleMenu_Main);
        myDebugMenu.begin();
    }
    out.println(F("\t finished."));
}

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
            out.println(F("\t 'X': test timings"));
            out.println();
            // out.println(F("\t 'A': Show 'HelloWorld' "));
            out.print(F("\t current sequencer: '"));
            out.print(effect_engine::sequencer_mode);
            out.println(F("'"));
            out.print(F("\t 'z': toggle sequencer_running ("));
            out.print(effect_engine::sequencer_running);
            out.println(F(")"));
            out.print(F("\t 'Z': toggle sequencer_direction ("));
            effect_engine::print_sequencer_direction(out);
            out.println(F(")"));
            out.println(F("\t 'o': sequencer off"));
            out.println(F("\t 'a': toggle CHANNELCHECK"));
            out.println(F("\t 'A': toggle SPIRAL"));
            out.println(F("\t 'b': toggle SPIRAL2"));
            out.println(F("\t 'B': toggle SUN SPIRAL"));
            out.println(F("\t 'c': toggle HORIZONTAL"));
            out.println(F("\t 'C': toggle SUN WAVE blue"));
            out.println(F("\t 'd': toggle SUN WAVE orange"));
            out.println();
            out.print(F("\t 'r': toggle effect_control ("));
            out.print(dmx_handling::effect_control);
            out.println(F(")"));
            out.print(F("\t 'R': toggle debugOut_dmx_Enabled ("));
            out.print(debugOut_dmx_Enabled);
            out.println(F(")"));
            out.print(F("\t 'e': toggle dmx serial out ("));
            out.print(dmx_handling::serial_out_enabled);
            out.println(F(")"));
            out.print(F("\t 'E': set dmx serial out interval 'i65535' ("));
            out.print(dmx_handling::serial_out_interval);
            out.println(F(")"));
            // ----------
            out.println();
            out.println(F("\t 's': sequencer single step 's'"));
            out.print(F("\t 'u': set sequencer interval 'u65535' ("));
            out.print(effe::sequencer_interval);
            out.println(F(")"));
            out.print(F("\t 'U': set calculate interval 'U65535' ("));
            out.print(effe::calculate_interval);
            out.println(F(")"));
            out.println();
            out.print(F("\t 'v': set effect value_low 'v65535' ("));
            out.print(effe::value_low);
            out.println(F(")"));
            out.print(F("\t 'V': set effect value_high 'V65535' ("));
            out.print(effe::value_high);
            out.println(F(")"));
            out.println();
            out.println();
            out.println(F("\t 'S': set channel 'S1:65535'"));
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
        case 'X': {
            effect_engine::test_update(out);
        } break;
        //---------------------------------------------------------------------
        // case 'A': {
        //     out.println(F("\t Hello World! :-)"));
        // } break;
        case 'z': {
          out.print(F("\t toggle sequencer_running: "));
            effect_engine::sequencer_running = !effect_engine::sequencer_running;
            out.print(effect_engine::sequencer_running);
            out.print(F("\n"));
        } break;
        case 'Z': {
          out.print(F("\t toggle sequencer_direction: "));
            effect_engine::toggle_sequencer_direction(out);
        } break;
        case 'o': {
          out.print(F("\t sequencer_mode: OFF\n"));
            effe::toggle_sequencer(out, effe::sequencer_OFF, 1000);
        } break;
        case 'a': {
            out.println(F("\t toggle CHANNELCHECK:"));
            effe::toggle_sequencer(out, effe::sequencer_CHANNELCHECK, 1000);
        } break;
        case 'A': {
            out.println(F("\t toggle SPIRAL:"));
            effe::toggle_sequencer(out, effe::sequencer_SPIRAL, 1000);
        } break;
        case 'b': {
            out.println(F("\t toggle SPIRAL2:"));
            effe::toggle_sequencer(out, effe::sequencer_SPIRAL2, 100);
        } break;
        case 'B': {
            out.println(F("\t toggle SUN SPIRAL:"));
            effe::toggle_sequencer(out, effe::sequencer_SUN_SPIRAL, 100);
        } break;
        case 'c': {
            out.println(F("\t toggle HORIZONTAL:"));
            effe::toggle_sequencer(out, effe::sequencer_HORIZONTAL, 200);
        } break;
        case 'C': {
            out.println(F("\t toggle SUN_WAVE:"));
            effe::toggle_sequencer(out, effe::sequencer_SUN_WAVE_BLUE, 100);
        } break;
        case 'd': {
            out.println(F("\t toggle SUN_WAVE2:"));
            effe::toggle_sequencer(out, effe::sequencer_SUN_WAVE_ORANGE, 100);
        } break;
        // ------------------------------------------
        case 'r': {
            out.print(F("\t toggle effect_control:"));
            dmx_handling::effect_control = !dmx_handling::effect_control;
            out.print(dmx_handling::effect_control);
            out.println();
        } break;
        case 'R': {
            out.print(F("\t debugOut_dmx_Enabled:"));
            debugOut_dmx_Enabled = !debugOut_dmx_Enabled;
            out.print(debugOut_dmx_Enabled);
            out.println();
        } break;
        case 'e': {
            out.println(F("\t toggle dmx serial out"));
            dmx_handling::serial_out_enabled =
                !dmx_handling::serial_out_enabled;
        } break;
        case 'E': {
            out.print(F("\t set dmx serial out interval "));
            // convert part of string to int
            // (up to first char that is not a number)
            uint8_t command_offset = 1;
            uint16_t value = atoi(&command[command_offset]);
            out.print(value);
            out.println();
            dmx_handling::serial_out_interval = value;
        } break;
        // ------------------------------------------
        case 's': {
            out.println(F("\t sequencer single step  "));
            effect_engine::calculate_step();
            effect_engine::calculate_step_next();
        } break;
        case 'u': {
            out.print(F("\t set sequencer interval "));
            // convert part of string to int
            // (up to first char that is not a number)
            uint8_t command_offset = 1;
            uint16_t value = atoi(&command[command_offset]);
            out.print(value);
            out.println();
            effe::sequencer_interval = value;
        } break;
        case 'U': {
            out.print(F("\t set calculate interval "));
            uint8_t command_offset = 1;
            uint16_t value = atoi(&command[command_offset]);
            out.print(value);
            out.println();
            effe::calculate_interval = value;
        } break;
        case 'v': {
            out.print(F("\t set effect value_low"));
            // convert part of string to int
            // (up to first char that is not a number)
            uint8_t command_offset = 1;
            uint16_t value = atoi(&command[command_offset]);
            out.print(value);
            out.println();
            effe::value_low = value;
        } break;
        case 'V': {
            out.print(F("\t set effect value_high"));
            // convert part of string to int
            // (up to first char that is not a number)
            uint8_t command_offset = 1;
            uint16_t value = atoi(&command[command_offset]);
            out.print(value);
            out.println();
            effe::value_high = value;
        } break;
        // ------------------------------------------
        case 'S': {
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

            if (LEDBoard::output_enabled) {
                LEDBoard::tlc.setChannel(ch, value);
                LEDBoard::tlc.write();
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
        //     LEDBoard::tlc.setChannel()
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
// button callbacks

void setup_buttons(Print &out) {
    out.print(F("# Free RAM = "));
    out.println(freeRam());

    out.println(F("setup button:")); {
        out.println(F("\t set button pin"));
        pinMode(button_pin, INPUT_PULLUP);
        out.println(F("\t button begin"));
        button.begin();
        // for (size_t index = 0; index < buttons_COUNT; index++) {
        //     pinMode(buttons[index].getPin(), INPUT_PULLUP);
        //     buttons[index].begin();
        // }
    }
    out.println(F("\t finished."));
}

void buttons_update() {
    button.update();
    // for (size_t index = 0; index < buttons_COUNT; index++) {
    //     buttons[index].update();
    // }
}


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
            if (effe::sequencer_mode == effe::sequencer_OFF) {
                effe::sequencer_mode = effe::sequencer_CHANNELCHECK;
                effe::sequencer_interval = 500;
                dmx_handling::effect_control = true;
                Serial.print(F("\t sequencer_mode: CHANNELCHECK\n"));
            } else {
                effe::sequencer_off();
                effe::sequencer_mode = effe::sequencer_OFF;
                Serial.print(F("\t sequencer_mode: OFF\n"));
            }

        } break;
        case slight_ButtonInput::event_ClickLong : {
            // Serial.println(F("click long"));
        } break;
        case slight_ButtonInput::event_ClickDouble : {
            // Serial.println(F("click double"));
            effe::sequencer_mode = effe::sequencer_SPIRAL;
            effe::sequencer_interval = 100;
            dmx_handling::effect_control = true;
            Serial.print(F("\t sequencer_mode: SPIRAL\n"));
        } break;
        case slight_ButtonInput::event_ClickTriple : {
            // Serial.println(F("click triple"));
            effe::sequencer_mode = effe::sequencer_SUN_SPIRAL;
            effe::sequencer_interval = 100;
            dmx_handling::effect_control = false;
            Serial.print(F("\t sequencer_mode: SUN SPIRAL\n"));
        } break;
        case slight_ButtonInput::event_ClickMulti : {
            Serial.print(F("click count: "));
            Serial.println((*pInstance).getClickCount());
            switch ((*pInstance).getClickCount()) {
                case 4 : {
                  effe::sequencer_mode = effe::sequencer_SUN_WAVE_BLUE;
                  effe::sequencer_interval = 100;
                  dmx_handling::effect_control = false;
                  Serial.print(F("\t sequencer_mode: sequencer_SUN_WAVE_BLUE\n"));
                } break;
                case 5 : {
                    effe::sequencer_mode = effe::sequencer_SUN_WAVE_ORANGE;
                    effe::sequencer_interval = 50;
                    dmx_handling::effect_control = false;
                    Serial.print(F("\t sequencer_mode: sequencer_SUN_WAVE_ORANGE\n"));
                } break;
                case 6 : {
                    effe::sequencer_mode = effe::sequencer_HPLINE;
                    effe::sequencer_interval = 50;
                    dmx_handling::effect_control = false;
                    Serial.print(F("\t sequencer_mode: High Power Line\n"));
                } break;
                case 7 : {
                    effe::sequencer_mode = effe::sequencer_HORIZONTAL;
                    effe::sequencer_interval = 1000;
                    dmx_handling::effect_control = true;
                    Serial.print(F("\t sequencer_mode: HORIZONTA = STATIC \n"));
                } break;
            }
        } break;
    }  // end switch
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
    //     LEDBoard::tlc.setChannel(channel_index, values[channel_index]);
    // }

    if (LEDBoard::output_enabled) {
        LEDBoard::tlc.setRGB(values[0], values[1], values[2]);
        LEDBoard::tlc.write();
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
            LEDBoard::output_enabled = true;
            digitalWrite(lowbat_warning_pin, HIGH);
            // Serial.println(F("--> output enabled"));
        } else if (bat_voltage <= 310) {
            // force off
            LEDBoard::output_enabled = false;
            digitalWrite(lowbat_warning_pin, LOW);
            LEDBoard::tlc.setRGB(0, 0, 0);
            LEDBoard::tlc.write();
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

    setup_DebugOut(DebugOut);

    setup_DebugMenu(DebugOut);

    sketchinfo_print(DebugOut);

    setup_buttons(DebugOut);

    LEDBoard::setup(DebugOut);

    DebugOut.print(F("# Free RAM = "));
    DebugOut.println(freeRam());
    setup_myFaderRGB(DebugOut);

    dmx_handling::setup(DebugOut);


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // go
    DebugOut.println(F("Loop:"));

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// main loop
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void loop() {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // menu input
    myDebugMenu.update();

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // update sub parts
    buttons_update();
    myFaderRGB.update();

    // lowbat_check();

    effect_engine::update();
    dmx_handling::update(DebugOut);

    handle_debugout(DebugOut);
    handle_debugout_dmx(DebugOut);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// THE END
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
