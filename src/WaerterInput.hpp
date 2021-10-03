#ifndef _WAERTERINPUT__
#define _WAERTERINPUT__
#include <Arduino.h>
#include "Ticker.hpp"
#include "Switches.hpp"
#include "BlockPort.hpp"
#include "DebouncedPin.hpp"

enum WaerterState {WS_IDLE, WS_TRIGGERD, WS_WAITMWT, WS_UNBLOCK, WS_WAITRELEASE};


class WaerterInput {

public:
  WaerterInput(Ticker *ticker, Switches *switches, BlockPort *block, uint8_t triggerInput, uint8_t mwtInput, uint8_t activateRelais, uint8_t signalRelais);
  void tick(unsigned long now);

private:
  Ticker *m_ticker;
  Switches *m_switches;
  uint8_t m_signalRelais;
  WaerterState m_state;

  DebouncedPin m_triggerInput;
  DebouncedPin *m_mwtInput;
  BlockPort *m_blockPort;
  unsigned long m_activateSignal;
  unsigned long m_deactivateSignal;
  unsigned long m_unblock;
};

#endif