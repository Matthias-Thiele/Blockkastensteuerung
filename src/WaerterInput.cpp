#include "WaerterInput.hpp"

#define DELTA_ACTIVATE_SIGNAL 6000
#define DELTA_KEEP_SIGNAL 6000
#define DELTA_UNBLOCK 6000

WaerterInput::WaerterInput(Ticker *ticker, Switches *switches, BlockPort *block, uint8_t triggerInput, uint8_t mwtInput, uint8_t activateRelais, uint8_t signalRelais) {
  m_ticker = ticker;
  m_switches = switches;
  m_triggerInput.init(triggerInput, 50ul);
  if (mwtInput > 0) {
    m_mwtInput = new DebouncedPin();
    m_mwtInput->init(mwtInput, 50ul);
  } else {
    m_mwtInput = NULL;
  }

  m_blockPort = new BlockPort(ticker, activateRelais, 5000);

  m_signalRelais = signalRelais;
  m_state = WS_IDLE;

  m_activateSignal = UINT64_MAX;
  m_deactivateSignal = UINT64_MAX;
  m_unblock = UINT64_MAX;

  digitalWrite(m_signalRelais, true);
}

void WaerterInput::tick(unsigned long now) {
  m_blockPort->tick(now);
  if (m_mwtInput) {
    m_mwtInput->tick(now);
  }

  if (m_triggerInput.tick(now)) {
    switch (m_state) {
      case WS_IDLE:
        if (m_triggerInput.read()) {
          m_state = WS_TRIGGERD;
          m_activateSignal = now + DELTA_ACTIVATE_SIGNAL;
          m_deactivateSignal = m_activateSignal + DELTA_KEEP_SIGNAL;
        }
        break;

      case WS_WAITMWT:
        if (m_mwtInput->read()) {
          m_state = WS_UNBLOCK;
          m_unblock = now + DELTA_UNBLOCK;
        }
        break;

      case WS_WAITRELEASE:
        if (!m_triggerInput.read()) {
          m_state = WS_IDLE;
        }
        break;
    }

    if (now > m_activateSignal) {
      digitalWrite(m_signalRelais, false);
      m_activateSignal = UINT64_MAX;
    }

    if (now > m_deactivateSignal) {
      digitalWrite(m_signalRelais, true);
      m_deactivateSignal = UINT64_MAX;

      if (m_mwtInput) {
        m_state = WS_WAITMWT;
      } else {
        m_state = WS_UNBLOCK;
        m_unblock = now + DELTA_UNBLOCK;
      }
    }

    if (now > m_unblock) {
      m_blockPort->block();
      m_state = WS_WAITRELEASE;
      m_unblock = UINT64_MAX;
    }
  }
}