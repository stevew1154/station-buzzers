// Station Buzzers -- Arduino sketch to control station buzzers on a model railroad
//   Copyright (c) 2013-2017, Stephen Paul Williams <spwilliams@gmail.com>
//
// This program is free software; you can redistribute it and/or modify it under the terms of
// the GNU General Public License as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with this program;
// if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
// Boston, MA 02110-1301, USA.

///////////////////////////////////////////////////////////////////////////////////////////////
// This Arduino sketch monitors the "called" and "answered" lines associated with a set of
// telephone buzzers, and plays a unique station code for each station using American Railroad
// Morse code.
//
// The stations themselves are defined in the array "stations" in the file station_info.cpp and
// the program automatically re-schedules itself based on the number of stations in this array.
//
// The files morse.h and morse.cpp provide a C++ class MorseBuzzer which implements the Morse
// code for a buzzer attached to an Arduino output pin.  This class would easily support having
// multiple MorseBuzzer objects running simultaneously, but for this application, that is not
// desired.
//
// The files station_states.h/.cpp implement the main state machine which cycles each station
// through the sequence of states associated with a call, ensuring that only one station is
// ringing at any given time.
///////////////////////////////////////////////////////////////////////////////////////////////
#include "station_info.h"
#include "station_states.h"
#include "avr/pgmspace.h"
#include "DebugSerial.h"

const char version[] = "v2.0";

// stations
//
// This table defines the stations on the layout.
//
// Normal stations have level-sensitive "called" inputs, and will continue ringing until either
// answered or until the caller hangs up. To define a "normal" station, set the station_type to
// STATION_NORMAL, and define the "buzzer", "called" and "off_hook" pins. The timeout value is
// ignored.
//
// Momentary stations are defined by setting station_type to STATION_MOMENTARY, set the buzzer,
// called, and answered pins. The timeout value should specify the timeout in seconds. If set to
// zero, the station will ring forever or until answered.
//
// To define an "ambience" station, set the "station_type" to STATION_AMBIENT, and the"called" and
// "off_hook" pins to -1. The timeout will then define a "base" time between random ambience
// messages. The sketch will pick a random interval between 2/3 and 4/3 of this base time. It really
// doesn't matter what you set the "station_code" to, as that field will only show up in debug
// messages.
//
// Use of Arduino "analog" pins
// ============================
//
// On most Arduino variants, the analog input pins are A0-A5 also fully capable of being digital
// input or output pins. However, the A6 and A7 pins are a mixed bag. On the Leonardo and other
// ATMEGA 32u4 based boards, A6 and A7 are muxed with other digital input pins so you probably 
// don't want to use them by the names A6/A7. 
//
// On the Uno, Mini, Nano and probably most other ATMEGA-328P variants A6 and A7 are distinct
// pins, but lack any sort of digital hardware, and therefore can only be used as analog
// inputs. This sketch will allow you to use A6 and A7 of the 328-based Arduinos as either "called"
// or "off_hook" inputs, but you must specify "active" as ANALOG_LOW or ANALOG_HIGH instead of LOW
// or HIGH. Also, your circuit must provide an external pull-up (ANALOG_LOW) or pull-down
// (ANALOG_HIGH) resistor. I.e. if your called switch connects pin A6 to ground, specify it as
// A6,ANALOG_LOW in the table and provide an external pull-up resistor (1K Ohm should work well).

// MRCS Buzzer Board rev 2
//
// The second-generation buzzer board supports 7 stations plus an ambience buzzer
#define MRCS_REV2_TABLE
#ifdef MMRCS_REV2_TABLE
const struct Station_Info stations[] = {
  //                         buzzer       called      off_hook     timeout   station
  //  station_type,        pin,active,  pin,active,  pin,active,   seconds,   code
  {   STATION_MOMENTARY,    13, HIGH,      A0, LOW,       0, LOW,      0,       "AA"  },
  {   STATION_MOMENTARY,    12, HIGH,      A1, LOW,       1, LOW,      0,       "BB"  }, 
  {   STATION_MOMENTARY,    11, HIGH,      A2, LOW,       2, LOW,      0,       "CC"  },
  {   STATION_MOMENTARY,    10, HIGH,      A3, LOW,       3, LOW,      0,       "DD"  },
  {   STATION_MOMENTARY,     9, HIGH,      A4, LOW,       4, LOW,      0,       "EE"  },
  {   STATION_MOMENTARY,     8, HIGH,      A5, LOW,       5, LOW,      0,       "FF"  },
  {   STATION_MOMENTARY,     7, HIGH,      A6, LOW,      A7, LOW,      0,       "GG"  },

  // This demonstrates an "ambiance" station which will buzz it's code at a random interval between
  // 2/3 and 4/3 of the "timeout_sec". This station doesn't need "answered" or "called" pins so they
  // are set to -1.
  {   STATION_AMBIENCE,      6, HIGH,      -1, LOW,     -1, LOW,      60,       "MM"  },
};
#endif

// David Parks Cumberland West
//
// This layout has 5 "normal" stations that ring until answered or until the caller hangs up
#define DAVID_PARKS_TABLE
#ifdef DAVID_PARKS_TABLE
const struct Station_Info stations[] = {
  //                         buzzer       called      off_hook     timeout   station
  //  station_type,        pin,active,  pin,active,  pin,active,   seconds,   code
  {   STATION_NORMAL,      8, HIGH,      A0, LOW,       2, LOW,      0,       "ND" }, // Viaduct
  {   STATION_NORMAL,      9, HIGH,      A1, LOW,       3, LOW,      0,       "GE" }, // Evitts
  {   STATION_NORMAL,     10, HIGH,      A2, LOW,       4, LOW,      0,       "KY" }, // Keyser
  {   STATION_NORMAL,     11, HIGH,      A3, LOW,       5, LOW,      0,       "CO" }, // McKenxie
  {   STATION_NORMAL,     12, HIGH,      A4, LOW,       6, LOW,      0,       "P"  }, // Piedmont

  // This demonstrates an "ambiance" station which will buzz it's code at a random interval between
  // 2/3 and 4/3 of the "timeout_sec". This station doesn't need "answered" or "called" pins so they
  // are set to -1.
  {   STATION_AMBIENCE,   13, HIGH,        -1, LOW,     -1, LOW,      60,       "DS" }, // Dispatcher
};
#endif


// Dave Adams D&RGW
//
// This layout has 6 momentary stations, and no ambience buzzer. It is built on the MRCS Morse Buzzer 
// board v1.0, using an Arduino Pro Mini (Atmel 328P), and in order to use pin 13 as an input for "RO",
// Seth Neumann removed the built-in LED on D13.
//
// This sketch should work equally well on a Nano or a Leonardo with the same caveat: D13 is hard to
// use as an input without either removing the on-board LED circuit or adding a pull-up resistor.
#undef DAVE_ADAMS_TABLE
#ifdef DAVE_ADAMS_TABLE
const struct Station_Info stations[] = {
  //                         buzzer       called      off_hook     timeout   station
  //  station_type,        pin,active,  pin,active,  pin,active,   seconds,   code
  {   STATION_MOMENTARY,   11, HIGH,      A0, LOW,     2, LOW,       30,      "DW" }, // West Durango
  {   STATION_MOMENTARY,   10, HIGH,      A1, LOW,     4, LOW,       30,      "HF" }, // Hesperus
  {   STATION_MOMENTARY,    9, HIGH,      A2, LOW,     7, LOW,       30,      "CA" }, // Cima Summit
  {   STATION_MOMENTARY,    6, HIGH,      A3, LOW,     8, LOW,       30,      "MX" }, // Mancos
  {   STATION_MOMENTARY,    5, HIGH,      A4, LOW,    12, LOW,       30,      "DJ" }, // Dolores
  {   STATION_MOMENTARY,    3, HIGH,      A5, LOW,    13, LOW,       30,      "RO" }, // Rico
};
#endif
const int num_stations = sizeof(stations) / sizeof(stations[0]);

// The messages played by the ambience sations are defined here. We are playing Arduino AVR tricks
// here to place the strings themselves in the Arduino's larger program memory.
//
// You can add as many messages as you need, limited only by the code space in your Arduino.  Adding
// a message is a two-step process -- first, you need to add a declaration for the string itself
// like the declaration for "ambience_00" below. Simply copy/paste the last string and bump the
// number. Then you have to add the new string to the "ambience_messages" array.
const char ambience_00[] PROGMEM = "OS No 2 by ND at 14:45";
const char ambience_01[] PROGMEM = "DS GE Copy 3 West";
const char ambience_02[] PROGMEM = "Check box 13, the dispatcher is an idiot.";

const char * const ambience_messages[] PROGMEM = {
  ambience_00, ambience_01, ambience_02,
};
const int num_ambience_messages = sizeof(ambience_messages) / sizeof(ambience_messages[0]);


void setup()
{
  DebugSerial_begin(9600);
  delay(5000);
  DebugSerial_print(F("Station Buzzers "));
  DebugSerial_println(version);

  init_station_states();
}

void loop()
{
  run_station_states();
}
