// station_info.h -- structures for managing station information in station_buzzers
//   Copyright (c) 2013-2014, Stephen Paul Williams <spwilliams@gmail.com>
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
#include "morse.h"

#define STATION_CALLED    LOW   // Value on pin_called when station is called
#define STATION_ANSWERED  LOW   // Value on pin_aswered when station has answered

enum Station_States {
  IDLE,
  RING_WAITING, // Station has played ring tone once, waiting to do it again
  RING_PLAYING, // Station is playing its ring tone
  TALKING,
  HANGUP_WAIT,  // Waiting for both parties to hangup
  LAST_UNUSED_STATE
};


struct Station_Info {
  Station_States     state_;
  bool               pulsed_called_;
  byte               pin_called_;
  byte               pin_answered_;
  byte               pin_buzzer_;
  byte               pin_active_;
  unsigned           random_period_sec_;
  const char * const station_code_;
  unsigned long      wait_enter_millis_;
  unsigned long      next_call_millis_;
  MorseBuzzer        morse_;
  bool               is_called_;
  bool               is_answered_;
  unsigned long      called_millis_;
  unsigned long      answered_millis_;

  const char * const station_code() { return station_code_; }
  bool is_ambience() { return random_period_sec_ != 0; }
  byte pin_buzzer() { return pin_buzzer_; }
  Station_States state() { return state_; }
  bool called();
  bool answered();
  bool still_playing() { return morse_.still_playing(); }
  void enter_idle();
  void enter_ring_waiting();
  void enter_ring_playing();
  void enter_talking();
  void enter_hangup_wait();
  unsigned waiting_msec() { return millis() - wait_enter_millis_; }
private:
  void buzzer_off() { digitalWrite( pin_buzzer_, pin_active_ == HIGH ? LOW : HIGH ); }
};

extern struct Station_Info stations[];
extern const int num_stations;

#endif
