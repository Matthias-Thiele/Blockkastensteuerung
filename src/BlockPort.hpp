#ifndef _BLOCKPORT__
#define _BLOCKPORT__
#include <Arduino.h>
#include "Ticker.hpp"

class BlockPort {
  public:
    BlockPort(Ticker *ticker, uint8_t outPort, uint16_t blockDuration);
    void tick(unsigned long now);
    void block();
    void setInvers();

  private:
    Ticker *m_ticker;
    bool m_isInvers;
    uint8_t m_outPort;
    uint16_t m_blockDuration;
    unsigned long m_endBlock;
};


#endif