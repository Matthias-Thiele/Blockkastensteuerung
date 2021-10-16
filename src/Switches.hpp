#ifndef _SWITCHES__
#define _SWITCHES__

#include <Arduino.h>
#include "Ticker.hpp"

class Switches {
  public:
    Switches(Ticker *ticker);
    void tick(unsigned long now);
    unsigned long getActValue();

  private:
    Ticker *m_ticker;
    unsigned long m_nextTick;
    unsigned long m_lastValue;
    uint8_t state;
    unsigned long mapper1[6] = {0x4000, 0x1000, 0x400, 0x10, 4, 1};
    unsigned long mapper2[6] = {0x40, 0x200, 0x80, 0x80000, 0x20000, 0x10000};
    unsigned long mapper3[8] = {2, 8, 0x20, 0x800, 0x2000, 0x8000, 0x40000, 0x100};

    void sendI2C(uint8_t addr, uint8_t cmd, uint8_t value);
    void init_MCP(uint8_t i2CAddr);
    unsigned long read();
    uint8_t readRegister(uint8_t addr, uint8_t regNo);
    void fillup(unsigned long &result, unsigned long *mapper, uint8_t source, uint8_t size);
};

#endif