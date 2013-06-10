#include "station_info.h"
#include "station_states.h"

const char version[] = "v1.0";
int value = 0;
unsigned long last_flip = 0;

void flip(int pin)
{
  if ((millis() - last_flip) > 1000)
  {
    value = 1 - value;
    digitalWrite(pin, value);
    last_flip = millis();
  }
}

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
  pinMode(13,OUTPUT);
  digitalWrite(13,value);

}

void loop()
{
  delay(10);
  if (0)
  {
    Serial.print(millis());
    Serial.println(" top of loop");
  }
  for (int ii = 0; ii < num_stations; ii++)
  {
    Station_Info *station = &stations[ii];
    if (0)
    {
      Serial.print(millis());
      Serial.print(" ");
      Serial.print(station->station_code);
      Serial.print(" st:");
      Serial.print(station->state);
      Serial.print(" called:");
      Serial.print(station->called());
      Serial.print(" answered:");
      Serial.print(station->answered());
      Serial.print(" buzz:");
      Serial.println(digitalRead(station->pin_buzzer));
    }

    do_current_state(station);
  }
  
  check_for_ringers();

  flip(13);  
}

