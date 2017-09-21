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
#include <limits.h>
#include "DebugSerial.h"

void Station_Info::enter_idle()
{
  called_latch_ = false;
  called_debounce_ = off_hook_debounce_ = false;
  called_millis_   = off_hook_millis_   = millis();
  morse_.setup(buzzer_pin_, buzzer_active_ == HIGH);
  if (is_ambience()) {
    // Make up the time that we will next play an ambience message
    const int ambience_idx = random(0, num_ambience_messages);
    ambience_message_ = reinterpret_cast<const __FlashStringHelper *>(pgm_read_word(&ambience_messages[ambience_idx]));
    
    next_call_millis_ = millis() + random(2000L * timeout_secs_ / 3, 4000L * timeout_secs_ / 3);
  } else {
    // Make our digital input pins inputs.
    if ((called_active_ & ANALOG_IN) == 0)
      pinMode(called_pin_, INPUT_PULLUP);
    if ((off_hook_active_ & ANALOG_IN) == 0)
      pinMode(off_hook_pin_, INPUT_PULLUP);
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
  if (is_ambience()) {
    morse_.start(ambience_message_.c_str());
    DebugSerial_println(ambience_message_);
  } else {
    morse_.start(station_code_);
  }
  state_ = RING_PLAYING;
}

void Station_Info::enter_talking()
{
  morse_.cancel();
  state_ = TALKING;
}

void Station_Info::enter_hangup_wait()
{
  // Stop our morse player (shouldn't be running).
  morse_.cancel();

  // Momentary stations become need to clear their latched called status
  // when they hang up
  called_latch_ = false;

  state_ = HANGUP_WAIT;

}

// read an input pin as either analog or digital
static  inline bool read_input_pin(uint8_t pin, uint8_t active)
{
  if (active & ANALOG_IN) {
    uint16_t value = analogRead(pin);
    return (value > 511) ? (active == ANALOG_HIGH) : (active == ANALOG_LOW);
  } else {
    return digitalRead(pin) == active;
  }
}

bool Station_Info::called()
{
  const unsigned long now_millis = millis();

  // A "random ambience" station gets called at a random time selected
  // when the station enters IDLE state, and stays "called" until it
  // finishes ringing one time.  This is all managed in enter_ring_waiting()
  if (is_ambience()) {
    // Deal with unwrapping for those cases the layout stays on for more than 24.8 days
    signed long diff_ring = (now_millis - next_call_millis_);
    return (0 < diff_ring && diff_ring < LONG_MAX);
  }

  // Debounce the input, looking for when the pin goes active
  
  const signed long diff_called = (now_millis - called_millis_);
  bool is_called = read_input_pin(called_pin_, called_active_);
  bool called_changed = ((is_called != called_debounce_) && (20 < diff_called) && (diff_called < LONG_MAX));
  called_debounce_ = is_called;

  if (!is_momentary()) {
    if (called_changed) {
      DebugSerial_print(station_code_); DebugSerial_print(F(" is "));
      DebugSerial_println(is_called ? F("called") : F("not called"));
      called_millis_ = now_millis;
    }
    return is_called;
  }

  // Special logic for stations which are "momentary".

  // First, if we have been called for too long, we timeout.
  if (called_latch_ && (state_ != RING_PLAYING) && (timeout_secs_ != 0)) 
  {
    if ((1000L * timeout_secs_ <= diff_called) && (diff_called < LONG_MAX)) {
      DebugSerial_print(F("Station "));
      DebugSerial_print(station_code_);
      DebugSerial_println(F(" timed out"));
      called_latch_ = false;
      return false;
    }
  }

  // Second, we only want to act on called becoming active when we are in state IDLE
  if (called_changed && is_called && (state_ == IDLE)) {
    DebugSerial_print(F("Station "));
    DebugSerial_print(station_code_);
    DebugSerial_println(F(" is called"));
    called_millis_ = now_millis;
    called_latch_ = true;
  }
  // Return our debounced is_called_state
  return called_latch_;
}

bool Station_Info::off_hook()
{
  // The "random" ambience stations never go off_hook
  if (is_ambience())
    return false;

  // Debounce state changes on off_hook_pin_
  const bool was_off_hook = off_hook_debounce_;
  const bool is_off_hook = read_input_pin(off_hook_pin_, off_hook_active_);
  const unsigned long now_millis = millis();
  const signed long diff_off_hook = (now_millis - off_hook_millis_);
  off_hook_debounce_ = is_off_hook;

  if ((is_off_hook != was_off_hook) && (20 <= diff_off_hook) && (diff_off_hook < LONG_MAX)) {
    // React to change on "off_hook"
    off_hook_millis_ = now_millis;
    DebugSerial_print(station_code_); DebugSerial_print(F(" goes "));
    DebugSerial_print(is_off_hook ? F("off") : F("on"));
    DebugSerial_println(F(" hook"));
  }

  // Return the the debounced hook state variable
  return is_off_hook;
}
