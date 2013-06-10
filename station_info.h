#ifndef INCLUDED_station_info
#define INCLUDED_station_info

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
  Station_States state;
  byte pin_called;
  byte pin_answered;
  byte pin_buzzer;
  const char * const station_code;
  int  ring_priority;
  
  bool called()   { return digitalRead(pin_called)   == STATION_CALLED;   }
  bool answered() { return digitalRead(pin_answered) == STATION_ANSWERED; }
};

extern struct Station_Info stations[];
extern const int num_stations;

#endif
