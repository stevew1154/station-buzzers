Station Buzzers -- Arduino sketch to control station buzzers on a model railroad

This sketch implements a state machine for model railroad telephone systems.
When a station is called, the corresponding station's buzzer will sound with
the American "Railroad Morse" code for the station's call sign.

This sketch was inspired to solve a problem on David Parks' "Cumberland West"
where the telephone buzzers could not be readily distinguished and as a result
the phones were going unanswered.

The initialization table to identify the pins used for the "called" and
"answered" lines, as well as the station designators is near the top of the
main sketch file "station_buzzers.ino"
