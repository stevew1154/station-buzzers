// Station Buzzers -- Arduino sketch to control station buzzers on a model railroad
//
// Copyright (c) 2013, Stephen Paul Williams
//
// This software is released under the terms of the GNU General Public License, version 2
// 
//
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
// through the sequence of states associated with a call, ensuring that only one station is ringing
// at any given time.

#include "station_info.h"
#include "station_states.h"

const char version[] = "v1.0";

class PinFlipper {
  int           pin;
  unsigned      interval;
  int           value;
  unsigned long last_flip;

public:
  PinFlipper(int pin, unsigned interval)
  : pin(pin),
    interval(interval),
    value(0),
    last_flip(0)
  {
    pinMode(pin, OUTPUT);
    flip();
  }
  void flip()
  {
    if ((millis() - last_flip) > interval)
    {
      value = 1 - value;
      digitalWrite(pin, value);
      last_flip = millis();
    }
  }
};

PinFlipper status_led(13, 1000);

void setup()
{
  Serial.begin(9600);
  delay(500);
  Serial.print("Station Buzzers ");
  Serial.println(version);

  for (int ii = 0 ; ii < num_stations; ii++)
  {
    stations[ii].state = LAST_UNUSED_STATE;
    goto_state(&stations[ii], IDLE);
  }
}

void loop()
{
  status_led.flip();  
  delay(10);
  for (int ii = 0; ii < num_stations; ii++)
  {
    Station_Info *station = &stations[ii];
    do_current_state(station);
  }
  
  check_for_ringers();

}

