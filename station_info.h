// station_info.h -- structures for managing station information in station_buzzers
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
#ifndef INCLUDED_station_info
#define INCLUDED_station_info

#include "Arduino.h"
#include "morse.h"

enum Station_States {
  IDLE,
  RING_WAITING, // Station has played ring tone once, waiting to do it again
  RING_PLAYING, // Station is playing its ring tone
  TALKING,
  HANGUP_WAIT,  // Waiting for both parties to hangup
  LAST_UNUSED_STATE
};

enum Station_Type {
  STATION_NORMAL,
  STATION_MOMENTARY,
  STATION_AMBIENCE
};

#define ANALOG_IN ((uint8_t) 0x80)
#define ANALOG_LOW (ANALOG_IN | LOW)
#define ANALOG_HIGH (ANALOG_IN | HIGH)

struct Station_Info {
  //////////////////////////////////////////////////////////////////////////
  // Fields initialized through the "stations" table in station_buzzers.ino
  //////////////////////////////////////////////////////////////////////////
  Station_Type       station_type_;

  byte               buzzer_pin_;       // Arduino pin the buzzer output is connected to
  byte               buzzer_active_;    // LOW or HIGH

  byte               called_pin_;       // Arduing pin for "called" input (-1 for an ambience station)
  byte               called_active_;    // LOW or HIGH
  

  byte               off_hook_pin_;     // Arduino pin for "off_hook" input (-1 for an ambience station)
  byte               off_hook_active_;  // LOW or HIGH

  uint16_t           timeout_secs_;     // For ambience stations, the basic time between messages
                                        // For momentary stations, the 

  const char * const station_code_;

  //////////////////////////////////////////////////////////////////////////////
  // Member fields below this point are not initialized in the table, but rather
  // when enter_idle() is first called
  //////////////////////////////////////////////////////////////////////////////
  Station_States     state_;
  unsigned long      wait_enter_millis_;
  unsigned long      next_call_millis_;
  MorseBuzzer        morse_;

  bool               called_latch_;
  bool               called_debounce_;
  unsigned long      called_millis_;
  
  bool               off_hook_debounce_;
  unsigned long      off_hook_millis_;

  String             ambience_message_;
  
  //////////////////////////////////////////////////////////////////////////
  // Member methods
  //////////////////////////////////////////////////////////////////////////
  Station_States state() { return state_; }
  const char * const station_code() { return station_code_; }
  bool is_ambience() { return (station_type_ == STATION_AMBIENCE); }
  bool is_momentary() { return (station_type_ == STATION_MOMENTARY); }
  bool still_playing() { return morse_.still_playing(); }

  bool called();
  bool off_hook();
  
  void enter_idle();
  void enter_ring_waiting();
  void enter_ring_playing();
  void enter_talking();
  void enter_hangup_wait();
  unsigned waiting_msec() { return millis() - wait_enter_millis_; }
 private:
  void buzzer_off() { digitalWrite(buzzer_pin_, (buzzer_active_ == HIGH) ? LOW : HIGH ); }
};

extern const struct Station_Info stations[];
extern const int num_stations;
extern const char * const ambience_messages[] PROGMEM;
extern const int num_ambience_messages;

#endif
