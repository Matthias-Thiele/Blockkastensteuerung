#ifndef _DEBOUNCEDPIN__
#define _DEBOUNCEDPIN__
#include <Arduino.h>

class DebouncedPin {
  public:
    DebouncedPin();
    void init(uint8_t pin, unsigned long deltaReadMillis);
    bool tick(unsigned long now);
    void update();
    bool read();

  private:
    uint8_t m_pin;
    uint8_t m_history;
    uint8_t m_count;
    unsigned long m_deltaReadMillis;
    unsigned long m_nextRead;
};

#endif

