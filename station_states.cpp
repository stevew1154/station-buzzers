// station_state.cpp -- state machine and transition functions or station_buzzers
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

#include <limits.h>
#include "station_states.h"
#include "station_info.h"

// Function callback types for the enter / state / exit conditions of each state
typedef void (*Enter_Callback)(Station_Info *station);
typedef void (*State_Callback)(Station_Info *station);
typedef void (*Exit_Callback)(Station_Info *station);

// The structure definition for the state table itself.
struct State_Callback_Table {
  Station_States state;   // to verify correctness of table
  Enter_Callback enter_callback;
  State_Callback state_callback;
  Exit_Callback  exit_callback;
};

// current_ringer: holds a pointer to the station that is currently ringing, if any.
static Station_Info *current_ringer = 0;

// last_ring_time: the mills() value when the most recent ringing station completed ringing
static unsigned long last_ring_millis = 0;

// ring_silence_interval: the minimum interval between completion (or interruption) of one ring
// and starting the next.
static const unsigned ring_silence_interval = 2000;
static const unsigned ambience_silence_interval = 10000;

// Forward declaration for the state transition function.
static void goto_state(Station_Info *station, enum Station_States next_state);


// IDLE state.  We wait here for the local phone to go off-hook or for the station to be called.
void
idle_enter(struct Station_Info *station)
{
  // Set variables for IDLE state
  station->enter_idle();
}

void
idle_state(struct Station_Info *station)
{
  if (station->called())
  {
    goto_state(station, RING_WAITING);
  }
  else if (station->off_hook())
  {
    goto_state(station, TALKING);
  }
}

void
idle_exit(struct Station_Info *station)
{
}


// RING_WAITING state: here we have been called and not yet answered, but some other station is playing
void
ring_waiting_enter(struct Station_Info *station)
{
  station->enter_ring_waiting();
}

void
ring_waiting_state(struct Station_Info *station)
{
  // When we are in RING_WAITING, the station should be being CALLED, but not yet OFF_HOOK
  if (!station->called())
  {
    // Caller hung up -- go back to IDLE
    goto_state (station, IDLE);
  }
  else if (station->off_hook())
  {
    goto_state (station, TALKING);
  }
}

void
ring_waiting_exit(struct Station_Info *station)
{
}

void
ring_playing_enter(struct Station_Info *station)
{
  current_ringer = station;
  station->enter_ring_playing();
}

void
ring_playing_state(struct Station_Info *station)
{
  if (!station->called())
  {
    // Caller hung up -- go back to IDLE
    // N.B. this aborts he playing of the station code
    goto_state(station, IDLE);
    return;
  }

  if (station->off_hook())
  {
    // Called station answered
    goto_state(station, TALKING);
    return;
  }

  if (!station->still_playing())
    goto_state(station, station->is_ambience() ? IDLE : RING_WAITING);
}

void
ring_playing_exit(struct Station_Info *station)
{
  current_ringer = 0;
  last_ring_millis = millis();
}

void
talking_enter(struct Station_Info *station)
{
  station->enter_talking();
}

void
talking_state(struct Station_Info *station)
{
  if ((!station->called()) || (!station->off_hook()))
    goto_state(station, HANGUP_WAIT);
}

void
talking_exit(struct Station_Info *station)
{
}

void
hangup_wait_enter(struct Station_Info *station)
{
  station->enter_hangup_wait();
}

void
hangup_wait_state(struct Station_Info *station)
{
  if ((!station->called()) && (!station->off_hook()))
    goto_state(station, IDLE);
}

void
hangup_wait_exit(struct Station_Info *station)
{
}

struct State_Callback_Table callback_table[] = {
  { IDLE,         idle_enter,         idle_state,         idle_exit                 },
  { RING_WAITING, ring_waiting_enter, ring_waiting_state, ring_waiting_exit         },
  { RING_PLAYING, ring_playing_enter, ring_playing_state, ring_playing_exit         },
  { TALKING,      talking_enter,      talking_state,      talking_exit              },
  { HANGUP_WAIT,  hangup_wait_enter,  hangup_wait_state,  hangup_wait_exit          }
};

const char * const state_names[] = {
  [IDLE]         = "IDLE",
  [RING_WAITING] = "RING_WAITING",
  [RING_PLAYING] = "RING_PLAYING",
  [TALKING]      = "TALKING",
  [HANGUP_WAIT]  = "HANGUP_WAIT",
};

void
goto_state(struct Station_Info *station, Station_States next_state)
{
  Station_States curr_state = station->state();
  if (curr_state != next_state)
  {
    Serial.print(millis(),DEC);
    Serial.print(" ");
    Serial.print(station->station_code());
    Serial.print(" changes state ");
    Serial.print(state_names[curr_state]);
    Serial.print("->");
    Serial.println(state_names[next_state]);


    // Does current state have an exit_callback?
    Exit_Callback exit_cb = callback_table[curr_state].exit_callback;
    if (exit_cb != 0)
      (*exit_cb)(station);

    // Does next state have an enter_callback?  If so, call it before
    // we change the station->state in case there is code in the enter
    // callback that is sensitive to the state we are coming from.
    Enter_Callback enter_cb = callback_table[next_state].enter_callback;
    if (enter_cb != 0)
      (*enter_cb)(station);
  }
}

void
choose_next_ringer()
{
  const unsigned long now_millis = millis();
  const signed long since_last_ring = now_millis - last_ring_millis;


  // Has it been long enough since the last ring?
  bool long_enough = ((ring_silence_interval < since_last_ring) && (since_last_ring < LONG_MAX));
  if (!long_enough)
    return;

  // Scan for "normal" (non ambience) stations that could ring
  unsigned next_age = 0;
  Station_Info *next_ringer = 0;
  bool all_normal_stations_idle = true;
  for (int ii = 0; ii < num_stations; ii++)
  {
    Station_Info * const station = &stations[ii];
    if (station->is_ambience()) continue;
    if (station->state() != IDLE) all_normal_stations_idle = false;
    if (station->state() != RING_WAITING) continue;

    const unsigned station_age = station->waiting_msec();
    if (station_age > next_age)
    {
      // This one is a candidate for next_ringer
      next_ringer = station;
      next_age = station_age;
    }
  }

  long_enough = ((ambience_silence_interval < since_last_ring) && (since_last_ring < LONG_MAX));
  if (false && stations[5].state() == RING_WAITING) {
    Serial.print("next_ringer="); Serial.print(reinterpret_cast<uintptr_t>(next_ringer));
    Serial.print(" next_age="); Serial.print(next_age);
    Serial.print(" since_last_ring="); Serial.print(since_last_ring);
    Serial.print(" age="); Serial.print(stations[5].waiting_msec());
    Serial.print(" test="); Serial.print(long_enough);
    Serial.println();
  }
  if (all_normal_stations_idle  && long_enough)
  {
    for (int ii = 0; ii < num_stations; ii++)
    {
      Station_Info * const station = &stations[ii];
      if (!station->is_ambience()) continue;
      if (station->state() != RING_WAITING) continue;
  
      const unsigned station_age = station->waiting_msec();
      if (station_age > next_age)
      {
        // This one is a candidate for next_ringer
        next_ringer = station;
        next_age = station_age;
      }
    }
  }
  
  if (next_ringer)
  {
    Serial.print("station "); Serial.print(next_ringer->station_code()); Serial.print(" will ring next"); Serial.println();
    goto_state(next_ringer, RING_PLAYING);
  }
}

void
init_station_states()
{
  for (int ii = 0 ; ii < num_stations; ii++)
  {
    Station_Info *station = &stations[ii];
    idle_enter(station);
  }
}

void
run_station_states()
{
  // Run each station through its state machine
  for (int ii = 0; ii < num_stations; ii++)
  {
    Station_Info *station = &stations[ii];
    State_Callback state_cb = callback_table[station->state()].state_callback;
    (*state_cb)(station);
  }

  if (!current_ringer)
    choose_next_ringer();
}
