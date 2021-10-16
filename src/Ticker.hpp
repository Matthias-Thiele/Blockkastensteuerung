#ifndef _TICKER__
#define _TICKER__
#include <Arduino.h>

class Ticker {
  public:
    Ticker(uint8_t port, int stateDurationMillis, int totalDurationMillis, int x);
    void start();
    void end();
    void tick(unsigned long now);
    bool isRunning();

  private:
    uint8_t m_port;
    int m_stateDurationMillis;
    int m_totalDurationMillis;

    volatile uint8_t m_count;
    unsigned long m_tickerDone;
    unsigned long m_nextChange;
    bool m_actState;
};


#endif

