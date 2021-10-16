#ifndef _WAERTERINPUT__
#define _WAERTERINPUT__
#include <Arduino.h>
#include "Ticker.hpp"
#include "Switches.hpp"
#include "BlockPort.hpp"
#include "DebouncedPin.hpp"
#include "Streckenblock.hpp"

enum WaerterState {WS_IDLE, WS_TRIGGERD, WS_WAITMWT, WS_UNBLOCK, WS_WAITRELEASE};


class WaerterInput {

public:
  WaerterInput(Streckenblock *streckenblock, Ticker *ticker, Switches *switches, BlockPort *block, unsigned long fahrstrassenMaske, DebouncedPin *triggerInput, DebouncedPin *mwtInput, uint8_t signalRelais);
  void tick(unsigned long now);
  void startRueckblocken(unsigned long now, bool waitForMwT);

private:
  Streckenblock *m_streckenblock;
  Ticker *m_ticker;
  Switches *m_switches;
  uint8_t m_signalRelais;
  WaerterState m_state;
  unsigned long m_fahrstrassenMaske;

  DebouncedPin *m_triggerInput;
  DebouncedPin *m_mwtInput;
  BlockPort *m_blockPort;
  bool m_isAusfahrt;
  bool m_withGleisbild;
  unsigned long m_activateSignal;
  unsigned long m_deactivateSignal;
  unsigned long m_unblock;
};

#endif