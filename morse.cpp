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
: state(PLAYING_DONE),
  pin(-1),
  text(0),
  morse(0),
  verbosity(0)
{
  pinMode(pin, OUTPUT);
  digitalWrite(pin, BUZZER_OFF);
  initialize_morse_table();
}

MorseBuzzer::~MorseBuzzer()
{
  digitalWrite(pin, BUZZER_OFF);
}

void
MorseBuzzer::start(int pin, const char *text)
{
  this->pin  = pin;
  this->text = text;
  this->morse = 0;
  this->state = PLAYING_DONE;
  next_char();
}

void
MorseBuzzer::cancel()
{
  digitalWrite(pin, BUZZER_OFF);
  state = PLAYING_DONE;
}

bool
MorseBuzzer::next_char()
{
  while(1)
  {
    byte curr_char = (*text++) & 0x7f;
    if (curr_char == '\0')
    {
      if (verbosity > 0)
      {
        Serial.println("morse eom");
      }
      
      // Natural end of message so we are done
      digitalWrite(pin, BUZZER_OFF);
      state = PLAYING_DONE;
      return false;
    }
    
   
    // Now lookup the Morse pattern for the current character 
    morse = morse_table[curr_char];
    if (morse == 0)
    {
      // No morse patter to match this character, so skip to next
      continue;
    }
  
    if (verbosity > 0)
    {
      Serial.print("morse.next_char() '");
      Serial.print(static_cast<char>(curr_char));
      Serial.print("' -> ");
      Serial.println(morse);
    }
    
    // Start the first bit of the new character  
    return next_morse_bit(); 
  }
}

bool
MorseBuzzer::next_morse_bit()
{
  char morse_bit = *morse++;
  switch(morse_bit)
  {
    case '\0':
      // End of current character
      return next_char();
      break;
      
    case '.':
      buzz_time = dot_time;
      gap_time  = dot_time;
      break;
    case ',':  // i.e. "dot space"
      buzz_time = dot_time;
      gap_time  = 2*dot_time;
      break;
    case '-':
      buzz_time = 3*dot_time;
      gap_time  = dot_time;
      break;
    case 'L':
      buzz_time = 9*dot_time;
      gap_time  = dot_time;
      break;
    case '0':
      buzz_time = 15*dot_time;
      gap_time  = dot_time;
      break;
  }

  // If this is the last morse bit of this character, add the inter-character gap to the off_time
  if (*morse == '\0')
  {  
    // If this is the last bit of the last character, add the inter-station delay
    if (*text == '\0')
      gap_time += 1000;
    else
      gap_time += 3*dot_time;
  }

  digitalWrite(pin, BUZZER_ON);
  state = PLAYING_BUZZ;
  ref_millis = millis();
  if (verbosity > 1)
  {
    Serial.print(ref_millis);
    Serial.print(" morse on for ");
    Serial.println(buzz_time);
  }
  return true;
}

bool
MorseBuzzer::still_playing()
{
  if (state == PLAYING_DONE)
  {
    if (verbosity > 0)
      Serial.println("morse -- playing done");
    return false;
  }

  // Compute time elapsed since our "ref_millis"
  unsigned elapsed = millis() - ref_millis;
    
  if (state == PLAYING_BUZZ)
  {
    // We are playing the buz, is it time to turn off?
    if (elapsed >= buzz_time)
    {
      // Time to turn off
      digitalWrite(pin, BUZZER_OFF);
      state = PLAYING_GAP;
      ref_millis = millis();
      if (verbosity > 1)
      {
        Serial.print(ref_millis);
        Serial.print(" morse off for ");
        Serial.println(gap_time);
      }
    }
    return true;
  }
  
  // We are in the gap time, are we completely done?
  if (elapsed < gap_time)
    return true;
  
  // Time to move to next bit
  bool result= next_morse_bit();
  return result;
}

