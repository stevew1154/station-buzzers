#include "station_info.h"
#include "Arduino.h"

struct Station_Info stations[] = {
  // state, called, answered, buzz, code  
  {   IDLE,     A0,        8,    2, "ND" }, // Viaduct
  {   IDLE,     A1,        9,    3, "GE" }, // Evitts
  {   IDLE,     A2,       10,    4, "KY" }, // Keyser
  {   IDLE,     A3,       11,    5, "CO" }, // McKenxie
  {   IDLE,     A4,       12,    6, "P"  }, // Piedmont
};
const int num_stations = sizeof(stations)/sizeof(stations[0]);


