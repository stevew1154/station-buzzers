// station_info.cpp -- station information table for stations_buzzers.ino
//   Copyright (c) 2013, Stephen Paul Williams <spwilliams@gmail.com>
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
#include "station_info.h"
#include "Arduino.h"

struct Station_Info stations[] = {
  // state, called, answered, buzz, code  
  {   IDLE,     A0,        8,    2, "ND" }, // Viaduct
  {   IDLE,     A1,        9,    3, "GE" }, // Evitts
  {   IDLE,     A2,       10,    4, "KY" }, // Keyser
  {   IDLE,     A3,       11,    5, "CO" }, // McKenxie
  {   IDLE,     A4,       12,    6, "P"  }, // Piedmont
};
const int num_stations = sizeof(stations)/sizeof(stations[0]);


