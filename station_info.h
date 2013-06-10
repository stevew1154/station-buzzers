// station_info.h -- structures for managing station information in station_buzzers
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
#ifndef INCLUDED_station_info
#define INCLUDED_station_info

#include "Arduino.h"

#define STATION_CALLED    0   // Value on pin_called when station is called
#define STATION_ANSWERED  0   // Value on pin_aswered when station has answered

enum Station_States {
  IDLE,
  RING_WAITING, // Station has played ring tone once, waiting to do it again
  RING_PLAYING, // Station is playing its ring tone
  TALKING,
  HANGUP_WAIT,  // Waiting for both parties to hangup
  LAST_UNUSED_STATE
};

struct Station_Info {
  Station_States     state;
  byte               pin_called;
  byte               pin_answered;
  byte               pin_buzzer;
  const char * const station_code;
  unsigned long      wait_enter_time;
  
  bool called()   { return digitalRead(pin_called)   == STATION_CALLED;   }
  bool answered() { return digitalRead(pin_answered) == STATION_ANSWERED; }
};

extern struct Station_Info stations[];
extern const int num_stations;

#endif
