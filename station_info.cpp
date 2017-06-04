// station_info.cpp -- station information table for stations_buzzers.ino
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
#include "station_info.h"
#include "Arduino.h"
#include "morse.h"

struct Station_Info stations[] = {
  // state, pulsed_call, called, answered, buzz, active, random_period_sec, code
  {   IDLE,     false,       A0,        8,    2,    LOW,                 0, "ND" }, // Viaduct
  {   IDLE,     true,        A1,        9,    3,    LOW,                 0, "GE" }, // Evitts
  {   IDLE,     false,       A2,       10,    4,    LOW,                 0, "KY" }, // Keyser
  {   IDLE,     false,       A3,       11,    5,    LOW,                 0, "CO" }, // McKenxie
  {   IDLE,     false,       A4,       12,    6,    LOW,                 0, "P"  }, // Piedmont

  // This demonstrates an "ambiance" station which will buzz it's code at a random interval
  // between 2/3 and 4/3 of the "random_period_sec". This station doesn't need "answered" or "called" pins
  //{   IDLE,     false,       -1,       -1,    7,    LOW,                 5, "DS" }, // Dispatcher
};
const int num_stations = sizeof(stations)/sizeof(stations[0]);

void Station_Info::enter_idle()
{
  is_called_ = is_answered_ = false;
  called_millis_ = answered_millis_ = millis();
  morse_.setup(pin_buzzer_, pin_active_==HIGH);
  if (is_ambience())
  {
    next_call_millis_ = millis() + random(2000L*random_period_sec_/3, 4000L*random_period_sec_/3);
  }
  else
  {
    pinMode(pin_called_, INPUT_PULLUP);
    pinMode(pin_answered_, INPUT_PULLUP);
  }
  state_ = IDLE;
}

void Station_Info::enter_ring_waiting()
{
  morse_.cancel();

  // Mark the time we entered RING_WAITING
  wait_enter_millis_ = millis();

  // If we are moving IDLE->RING_WAITING, make us look artificially old
  // so we will ring next.
  if (state_ == IDLE)
    wait_enter_millis_ -= (wait_enter_millis_ < 60000 ? wait_enter_millis_ : 60000);

  state_ = RING_WAITING;
}

void Station_Info::enter_ring_playing()
{
  state_ = RING_PLAYING;
  morse_.start(station_code_);
}

void Station_Info::enter_talking()
{
  morse_.cancel();
  state_ = TALKING;
}

void Station_Info::enter_hangup_wait()
{
  morse_.cancel();
  state_ = HANGUP_WAIT;
}

bool Station_Info::called()
{
  // A "random ambience" station gets called at a random time selected
  // when the station enters IDLE state, and stays "called" until it
  // finishes ringing one time.  This is all managed in enter_ring_waiting()
  if( is_ambience() )
    return millis() > next_call_millis_;

  // Debounce the input, looking for when the pin goes active

  const bool was_called = is_called_;
  const bool is_called = (digitalRead(pin_called_) == STATION_CALLED);
  const unsigned long now_millis = millis();
  bool called_changed = (is_called != was_called) && ((now_millis - called_millis_) > 20);

  // Special logic for stations which are "pulsed_called". For these stations, we want to
  // become called only when (state_ == STATE_IDLE)
  if (pulsed_called_ && called_changed && (state_ != IDLE))
  {
    // ignore the input transition
    called_changed = false;
  }

  if (called_changed)
  {
    called_millis_ = millis();
    is_called_ = is_called;
    Serial.print(station_code_); Serial.print(" is ");
    Serial.print(is_called_ ? "called" : "not called");
    Serial.println();
  }

  // Return our debounced is_called_state
  return is_called_;
}

bool Station_Info::answered()
{
  // The "random" ambiance stations never get answered
  if (is_ambience())
    return false;

  // Debounce state changes on pin_answered_
  const bool was_answered = is_answered_;
  const bool is_answered = (digitalRead(pin_answered_) == STATION_ANSWERED);
  if ((is_answered != was_answered) && ((millis() - answered_millis_) > 20))
  {
    // React to change on "answered"
    answered_millis_ = millis();
    is_answered_ = is_answered;
    Serial.print(station_code_); Serial.print(" goes ");
    Serial.print(is_answered ? "off" : "on");
    Serial.print(" hook"); Serial.println();
  }

  if (pulsed_called_ && is_answered_ && is_called_) {
    is_called_ = false;
    Serial.print(station_code_); Serial.print(" is not called");
    Serial.println();
  }
  // Return the value in the debounced is_answered_ state variable
  return is_answered_;
}
