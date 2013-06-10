#include "station_states.h"
#include "morse.h"

// IDLE state.  We wait here for the local phone to go off-hook or for the station to be called.
void 
idle_enter(struct Station_Info *station)
{
  // Set variables for IDLE state
  pinMode(station->pin_called, INPUT_PULLUP);
  pinMode(station->pin_answered, INPUT_PULLUP);
  pinMode(station->pin_buzzer, OUTPUT);
  digitalWrite(station->pin_buzzer, BUZZER_OFF);
}

void 
idle_state(struct Station_Info *station)
{
  if (station->called())
  {
    for (int ii = 0; ii < num_stations; ii++)
    {
      if (stations[ii].state == RING_PLAYING) 
      {
        goto_state(station, RING_WAITING);
        return;
      }
    }

    // No other station currently playing, so play the ring
    goto_state(station, RING_PLAYING);
  }
  else if (station->answered())
  {
    goto_state(station, TALKING);
  }
}

void 
idle_exit(struct Station_Info *station)
{
}

void 
ring_waiting_enter(struct Station_Info *station)
{
  if (station->state == IDLE)
  {
    // Make us the oldest station waiting
    station->ring_priority = -1;
    for (int ii = 0; ii < num_stations; ii++)
    {
      if(stations[ii].ring_priority >= station->ring_priority)
        station->ring_priority = stations[ii].ring_priority + 1;
    }
  }
  else
  {
    // Make every other station that is currently RING_WAITING be one step older and pick the oldest
    for (int ii = 0; ii < num_stations; ii++)
    {
      if ((stations[ii].state == RING_WAITING) && (stations[ii].ring_priority >= 0))
      {
        stations[ii].ring_priority += 1;
      }
    }

    // Then make this station be the newest one waiting
    station->ring_priority = 0;
  }
}

void 
ring_waiting_state(struct Station_Info *station)
{
  // When we are in RING_WAITING, the station should be being CALLED, but not yet ANSWERED
  if (!station->called())
  {
    // Caller hung up -- go back to IDLE
    goto_state(station, IDLE);
  }
  else if (station->answered())
  {
    goto_state(station, TALKING);
  }
}

void 
ring_waiting_exit(struct Station_Info *station)
{
}

MorseBuzzer morse;

void 
ring_playing_enter(struct Station_Info *station)
{
  morse.start(station->pin_buzzer, station->station_code);
}

void 
ring_playing_state(struct Station_Info *station)
{
  // Should this exit the RING_PLAYING state immediately if the station goes off-hook?
  if (!station->called())
  {
    // Caller hung up -- go back to IDLE
    goto_state(station, IDLE);
    return;
  }

  if (station->answered())
  {
    goto_state(station, TALKING);
    return;
  }

  if (morse.still_playing())
    return;
    
  goto_state(station, RING_WAITING);
}

void 
ring_playing_exit(struct Station_Info *station)
{
  // Clean up anything allocated in ring_playing_enter
  morse.cancel();
}

void 
talking_enter(struct Station_Info *station)
{
}

void 
talking_state(struct Station_Info *station)
{
  if ((!station->called()) || (!station->answered()))
    goto_state(station, HANGUP_WAIT);
}

void 
talking_exit(struct Station_Info *station)
{
}

void 
hangup_wait_enter(struct Station_Info *station)
{

}

void 
hangup_wait_state(struct Station_Info *station)
{
  if ((!station->called()) && (!station->answered()))
    goto_state(station, IDLE);
}

void 
hangup_wait_exit(struct Station_Info *station)
{
}

struct State_Callback_Table callback_table[] = {
  { IDLE,         idle_enter,         idle_state,         idle_exit                 }
  ,
  { RING_WAITING, ring_waiting_enter, ring_waiting_state, ring_waiting_exit         }
  ,
  { RING_PLAYING, ring_playing_enter, ring_playing_state, ring_playing_exit         }
  ,
  { TALKING,      talking_enter,      talking_state,      talking_exit              }
  ,
  { HANGUP_WAIT,  hangup_wait_enter,  hangup_wait_state,  hangup_wait_exit          }
};

void 
goto_state(struct Station_Info *station, Station_States next_state)
{
  if (station->state != next_state)
  {
    Serial.print(millis(),DEC);
    Serial.print(" ");
    Serial.print(station->station_code);
    Serial.print(" changes state ");
    Serial.print(station->state,DEC);
    Serial.print("->");
    Serial.println(next_state,DEC);
    
    
    // Does current state have an exit_callback?
    Exit_Callback exit_cb = callback_table[station->state].exit_callback;
    if (exit_cb != 0)
      (*exit_cb)(station);

    // Does next state have an enter_callback?  If so, call it before
    // we change the station->state in case there is code in the enter
    // callback that is sensitive to the state we are coming from.
    Enter_Callback enter_cb = callback_table[next_state].enter_callback;
    if (enter_cb != 0)
      (*enter_cb)(station);

    // Finally change state
    station->state = next_state;
  }
}

void
do_current_state(Station_Info *station)
{
  delay(10);
  State_Callback state_cb = callback_table[station->state].state_callback;
  (*state_cb)(station);
}


void
check_for_ringers()
{
  // Look for oldest station waiting to ring.
  int ring_priority = -1;
  int ring_idx = -1;
  for (int ii = 0; ii < num_stations; ii++)
  {
    if (stations[ii].state == RING_PLAYING)
      return;
      
    if (stations[ii].state == RING_WAITING)
    {
      if (stations[ii].ring_priority > ring_priority)
      {
        // This one has been waiting longest
        ring_idx = ii;
        ring_priority = stations[ii].ring_priority;
      }
    }
  }


  if (ring_idx >= 0)
    goto_state(&stations[ring_idx], RING_PLAYING);
}
