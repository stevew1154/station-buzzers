// morse.h -- plays railroad morse code through a buzzer attached to an Arduino pin
//   Copyright (c) 2013, Stephen Paul Williams <spwilliams@gmail.com>
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

#ifndef INCLUDED_morse
#define INLUDED_morse

#define BUZZER_ON   0
#define BUZZER_OFF  1

class MorseBuzzer {
  public:
    MorseBuzzer();
    ~MorseBuzzer();
    void start(int pin, const char *text);
    void cancel();
    bool still_playing();

  private:
    bool next_char();
    bool next_morse_bit();
    
    enum {
      PLAYING_DONE,
      PLAYING_BUZZ,
      PLAYING_GAP
    } state;
    int pin;
    const char *text;
    const char *morse;
    
    unsigned long ref_millis;
    unsigned buzz_time;
    unsigned gap_time;
    unsigned verbosity;
};

#endif
