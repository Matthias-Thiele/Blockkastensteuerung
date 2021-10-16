#ifndef _COMMANDPROCESSOR__
#define _COMMANDPROCESSOR__

#include <Arduino.h>
#include "Ticker.hpp"
#include "BlockPort.hpp"
#include "InputState.hpp"

#define STATE_DURATION 150
#define BLOCK_DURATION 6000

#define NUMBER_OF_ZUSTIMMUNGEN 6
#define CMD_ZUST_A1 0
#define CMD_ZUST_A2 1
#define CMD_ZUST_A4 2
#define CMD_ZUST_B2 3
#define CMD_ZUST_B4 4
#define CMD_ZUST_F 5

#define NUMBER_OF_STRECKENBLOCK 6
#define CMD_STRB_LB_ANF 6
#define CMD_STRB_LB_ERL 7
#define CMD_STRB_LB_END 8
#define CMD_STRB_AH_ANF 9
#define CMD_STRB_AH_ERL 10
#define CMD_STRB_AH_END 11

class CommandProcessor {
  public:
    CommandProcessor(Ticker *ticker);
    void execute(uint8_t command, bool hasGleisbild);
    void tick(unsigned long now);
    uint8_t getDirectionLB_AH();
    uint8_t getFreeStateLB_AH();
    
  private:
    Ticker *m_ticker;
    InputState *state;
    unsigned long m_gleisbildWatchdog = 0;

    BlockPort *Zustimmung[NUMBER_OF_ZUSTIMMUNGEN];
    BlockPort *Streckenblock[NUMBER_OF_STRECKENBLOCK];
};

#endif