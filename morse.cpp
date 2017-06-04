// morse.cpp -- plays morse code through a buzzer attached to an Arduino pin
//   Copyright (c) 2013-2014, Stephen Paul Williams <spwilliams@gmail.com>
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

#include "morse.h"
#include "Arduino.h"

static const unsigned dot_time = 100; // milliseconds

static const char * morse_table[128];
bool morse_table_initialized = false;

static void 
initialize_morse_table()
{
  if(! morse_table_initialized)
  {
    for (int ii = 0; ii < 128; ii++)
      morse_table[ii] = 0;
    morse_table[' '] = " ";
    morse_table['A'] = ".-";
    morse_table['B'] = "-...";
    morse_table['C'] = ".,.";
    morse_table['D'] = "-..";
    morse_table['E'] = ".";
    morse_table['F'] = ".-.";
    morse_table['G'] = "--.";
    morse_table['H'] = "....";
    morse_table['I'] = "..";
    morse_table['J'] = "-.-.";
    morse_table['K'] = "-.-";
    morse_table['L'] = "L";
    morse_table['M'] = "--";
    morse_table['N'] = "-.";
    morse_table['O'] = ",.";
    morse_table['P'] = ".....";
    morse_table['Q'] = "..-.";
    morse_table['R'] = ",..";
    morse_table['S'] = "...";
    morse_table['T'] = "-";
    morse_table['U'] = "..-";
    morse_table['V'] = "...-";
    morse_table['W'] = ".--";
    morse_table['X'] = ".-..";
    morse_table['Y'] = ".,..";
    morse_table['Z'] = "..,.";
    
    morse_table['0'] = "0";
    morse_table['1'] = ".--.";
    morse_table['2'] = "..-..";
    morse_table['3'] = "...-.";
    morse_table['4'] = "....-";
    morse_table['5'] = "---";
    morse_table['6'] = "......";
    morse_table['7'] = "--..";
    morse_table['8'] = "-....";
    morse_table['9'] = "-..-";
    
    morse_table['.'] = "..--..";
    morse_table[','] = ".-.-";
    morse_table['?'] = "-..-.";
    morse_table['\''] = ".----.";
    morse_table['!'] = "---.";
    morse_table['/'] = "-..-.";
    morse_table['('] = morse_table[')'] = "-.--.-";
    morse_table['&'] = ",...";
    morse_table[':'] = "---...";
    morse_table[';'] = "-.-.-.";
    morse_table['='] = "-...-";
    morse_table['-'] = "-....-";
    morse_table['_'] = "..__._";
    morse_table['"'] = ".-..-.";
    morse_table['@'] = ".--.-.";
    morse_table_initialized = true;
  }
}


MorseBuzzer::MorseBuzzer()
: state_(PLAYING_DONE),
  pin_(-1),
  active_hi_(true),
  text_(0),
  morse_(0),
  verbosity_(0)
{
  initialize_morse_table();
}

MorseBuzzer::~MorseBuzzer()
{
    buzzer_off();
}

void
MorseBuzzer::buzzer_off()
{
  if( pin_ != -1 )
    digitalWrite( pin_, active_hi_ ? LOW : HIGH );
}

void
MorseBuzzer::buzzer_on()
{
  if( pin_ != -1 )
    digitalWrite( pin_, active_hi_ ? HIGH : LOW );
}

void
MorseBuzzer::setup( int pin, boolean active_hi )
{
  pin_  = pin;
  active_hi_ = active_hi;
  pinMode( pin_, OUTPUT );
  buzzer_off();
  buzzer_off();
}

void
MorseBuzzer::start( const char *text )
{
  text_ = text;
  morse_ = 0;
  state_ = PLAYING_DONE;
  next_char();
}

void
MorseBuzzer::cancel()
{
  buzzer_off();
  state_ = PLAYING_DONE;
}

bool
MorseBuzzer::next_char()
{
  while(1)
  {
    byte curr_char = (*text_++) & 0x7f;
    if (curr_char == '\0')
    {
      if (verbosity_ > 0)
      {
        Serial.println("morse eom");
      }
      
      // Natural end of message so we are done
      buzzer_off();
      state_ = PLAYING_DONE;
      return false;
    }
    
   
    // Now lookup the Morse pattern for the current character 
    morse_ = morse_table[curr_char];
    if (morse_ == 0)
    {
      // No morse patter to match this character, so skip to next
      continue;
    }
  
    if (verbosity_ > 0)
    {
      Serial.print("morse.next_char() '");
      Serial.print(static_cast<char>(curr_char));
      Serial.print("' -> ");
      Serial.println(morse_);
    }
    
    // Start the first bit of the new character  
    return next_morse_bit(); 
  }
}

bool
MorseBuzzer::next_morse_bit()
{
  char morse_bit = *morse_++;
  switch( morse_bit )
  {
    case '\0':
      // End of current character
      return next_char();

    case ' ':
      buzz_time_ = 0;
      gap_time_  = 4*dot_time; // we will add 3 below
      break;      
    case '.':
      buzz_time_ = dot_time;
      gap_time_  = dot_time;
      break;
    case ',':  // i.e. "dot space"
      buzz_time_ = dot_time;
      gap_time_  = 2*dot_time;
      break;
    case '-':
      buzz_time_ = 2*dot_time;
      gap_time_  = dot_time;
      break;
    case 'L':
      buzz_time_ = 4*dot_time;
      gap_time_  = dot_time;
      break;
    case '0':
      buzz_time_ = 5*dot_time;
      gap_time_  = dot_time;
      break;
  }

  // If this is the last morse bit of this character (next bit is nul), add the inter-character gap to the off_time
  if( *morse_ == '\0' )
     gap_time_ += 3*dot_time;

  if( buzz_time_ > 0 )
    buzzer_on();
  state_ = PLAYING_BUZZ;
  ref_millis_ = millis();
  if (verbosity_ > 1)
  {
    Serial.print( ref_millis_) ;
    Serial.print(" morse on for ");
    Serial.println( buzz_time_ );
  }
  return true;
}

bool
MorseBuzzer::still_playing()
{
  if (state_ == PLAYING_DONE)
  {
    if (verbosity_ > 0)
      Serial.println("morse -- playing done");
    return false;
  }

  // Compute time elapsed since our "ref_millis"
  unsigned elapsed = millis() - ref_millis_;
    
  if (state_ == PLAYING_BUZZ)
  {
    // We are playing the buzz, is it time to turn off?
    if (elapsed >= buzz_time_)
    {
      // Time to turn off
      buzzer_off();
      state_ = PLAYING_GAP;
      ref_millis_ = millis();
      if (verbosity_ > 1)
      {
        Serial.print(ref_millis_);
        Serial.print(" morse off for ");
        Serial.println(gap_time_);
      }
    }
    return true;
  }
  
  // We are in the gap time, are we completely done?
  if (elapsed < gap_time_)
    return true;
  
  // Time to move to next bit
  return next_morse_bit();
}

