#ifndef INCLUDED_station_states
#define INCLDUED_stations_states

#ifndef INCLUDED_station_info
#include "station_info.h"
#endif

typedef void (*Enter_Callback)(Station_Info *station);
typedef void (*State_Callback)(Station_Info *station);
typedef void (*Exit_Callback)(Station_Info *station);

struct State_Callback_Table {
  Station_States state;   // to verify correctness of table
  Enter_Callback enter_callback;
  State_Callback state_callback;
  Exit_Callback  exit_callback;
};

void 
goto_state(Station_Info *station, Station_States next_state);

void
do_current_state(Station_Info *station);

void
check_for_ringers();

#endif
