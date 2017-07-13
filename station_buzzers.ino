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

const char version[] = "v2.0";

struct Station_Info stations[] = {
  //                       buzzer       called       off_hook     timeout
  //  station_type,        pin,active,  pin,active,  pin,active,  seconds,   station_code_
  {   STATION_NORMAL,      8,HIGH,        A0,LOW,       2,LOW,      0,       "ND" }, // Viaduct
  {   STATION_NORMAL,      9,HIGH,        A1,LOW,       3,LOW,      0,       "GE" }, // Evitts
  {   STATION_NORMAL,     10,HIGH,        A2,LOW,       4,LOW,      0,       "KY" }, // Keyser
  {   STATION_MOMENTARY,  11,HIGH,        A3,LOW,       5,LOW,     30,       "CO" }, // McKenxie
  {   STATION_NORMAL,     12,HIGH,        A5,LOW,       6,LOW,      0,       "P"  }, // Piedmont

  // This demonstrates an "ambiance" station which will buzz it's code at a random interval
  // between 2/3 and 4/3 of the "random_period_sec". This station doesn't need "answered" or "called" pins
  {   STATION_AMBIENCE,   13,HIGH,        -1,LOW,     -1,LOW,      30,       "DS" }, // Dispatcher
};
const int num_stations = sizeof(stations)/sizeof(stations[0]);

void setup()
{
  Serial.begin(9600);
  delay(5000);
  Serial.print("Station Buzzers ");
  Serial.println(version);

  init_station_states();
}

void loop()
{
  run_station_states();
}
