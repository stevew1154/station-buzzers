#ifndef INCLUDED_morse
#define INLUDED_morse

#define BUZZER_OFF   0  // set for active high to turn an NPN transistor on
#define BUZZER_ON  1    // reverse if driving something directly 

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
};

#endif
