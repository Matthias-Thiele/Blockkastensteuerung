#include "WaerterInput.hpp"

#define DELTA_ACTIVATE_SIGNAL 6000
#define DELTA_KEEP_SIGNAL 6000
#define DELTA_UNBLOCK 6000

/**
 * Verwaltet eine oder mehrere Fahrstraßen aus dem
 * simulierten Wärterstellwerk. Fahrstraßen, die
 * gleichartige Aktionen auslösen, können in ein
 * WaerterInput Objekt zusammengfasst werden.
 **/
WaerterInput::WaerterInput(Streckenblock *streckenblock, Ticker *ticker, Switches *switches, BlockPort *block, unsigned long fahrstrassenMaske, DebouncedPin *triggerInput, DebouncedPin *mwtInput, uint8_t signalRelais) {
  m_streckenblock = streckenblock;
  m_ticker = ticker;
  m_switches = switches;
  m_triggerInput = triggerInput;
  m_fahrstrassenMaske = fahrstrassenMaske;
  m_mwtInput = mwtInput;
  m_isAusfahrt = fahrstrassenMaske > 0x8000ul;
  m_withGleisbild = m_triggerInput == NULL;
  m_blockPort = block;

  pinMode(signalRelais, OUTPUT);

  m_signalRelais = signalRelais;
  m_state = WS_IDLE;

  m_activateSignal = UINT32_MAX;
  m_deactivateSignal = UINT32_MAX;
  m_unblock = UINT32_MAX;

  if (m_signalRelais > 0) {
    digitalWrite(m_signalRelais, true);
  }

  Serial.print("Mask "); Serial.println((unsigned long) m_fahrstrassenMaske, HEX);
}

/**
 * Wenn das Wärterstellwerk über das Gleisbild gesteuert wird,
 * erfolgt das Rückblocken nicht zeitgesteuert, sondern über
 * ein Kommando vom Gleisbild.
 **/
void WaerterInput::startRueckblocken(unsigned long now, bool waitForMwT) {
  if (m_mwtInput && waitForMwT) {
    Serial.println("GB: Wait for MWt.");
    m_state = WS_WAITMWT;
  } else {
    m_state = WS_UNBLOCK;
    m_unblock = now + DELTA_UNBLOCK;
    Serial.print("GB: Direct unlock at "); Serial.println(m_unblock);
  }
}

/**
 * Weiterleitung aus der Arduino loop Funktion, wird
 * für die asynchronen Vorgänge benötigt und muss
 * regelmäßig aufgerufen werden.
 **/
void WaerterInput::tick(unsigned long now) {
    //Serial.print("Mask "); Serial.print((unsigned long) m_fahrstrassenMaske, HEX); Serial.print(", FS: ");  Serial.print(~m_switches->getActValue(), HEX); Serial.print(", this: "); Serial.println((unsigned long) this, HEX);
    if ((~m_switches->getActValue() & m_fahrstrassenMaske) == 0) {
      // Es ist keine Fahrstraße aktiv die zu diesem WaerterInput gehören.
      return;
    }

    m_blockPort->tick(now);
    if (m_mwtInput) {
      m_mwtInput->tick(now);
    }

    static unsigned long nextSend2 = 0;
    if (now > nextSend2) {
      //Serial.print("Now: "); Serial.print(now); Serial.print(", unlock: "); Serial.println(m_unblock);
      nextSend2 = now + 500ul;
    }

    if (m_triggerInput) {
      m_triggerInput->tick(now);
    }

    switch (m_state) {
      case WS_IDLE: // Keine Aktion aktiv, wartet auf Kommando
        if (!m_triggerInput) {
          // Bei einer Steuerung über das Gleisbild, werden die
          // Eingangsschalter nicht eingelesen. Die Aktivierung
          // erfolgt nur über ein serielles Kommando.
          break;
        }

        if (m_triggerInput->read()) {
          // Signalschalter wurde aktiviert

          if (!m_streckenblock->isFree()) {
            Serial.println("Strecke ist noch belegt.");
            break;
          }

          if (m_isAusfahrt != m_streckenblock->hasErlaubnis()) {
            Serial.print("Keine Erlaubnis"); Serial.print(m_isAusfahrt); Serial.print(" - "); Serial.println(m_streckenblock->hasErlaubnis());
            break;
          }

          Serial.print("Triggered. "); Serial.println((unsigned long) this, HEX);
          m_state = WS_TRIGGERD;
          // Zeitpunkte für die Änderung der Signalanzeiger festlegen.
          m_activateSignal = now + DELTA_ACTIVATE_SIGNAL;
          m_deactivateSignal = m_activateSignal + DELTA_KEEP_SIGNAL;
        }
        break;

      case WS_WAITMWT: // Wartet auf die Betätigung der Fahrdienstleiter Mitwirktaste.
        if (!m_mwtInput->read()) {
          // MWT wurde betätigt, schaltet nach 0.
          Serial.println("MWT activated.");
          m_state = WS_UNBLOCK;
          m_unblock = now + DELTA_UNBLOCK;
        }
        break;

      case WS_WAITRELEASE: // Wartet auf die Rücknahme der Befehlsabgabe nach Abschluss der Aktion.
        if (!m_triggerInput || !m_triggerInput->read()) {
          // Es gibt keinen Hebel (serielles Kommando) oder Hebel wurde zurückgestellt
          Serial.println("Released.");
          if (!m_withGleisbild) {
            if (m_isAusfahrt) {
              m_streckenblock->setAnfang();
            } else {
              m_streckenblock->resetEnd();
            }
          }
          
          m_state = WS_IDLE;
        }
        break;

      default:
        // do nothing
        break;
    }

    if (now > m_activateSignal) {
      // Signalanzeiger setzen wenn zu diesem Objekt ein Anzeiger definiert wurde.
      Serial.print("Signal set.");Serial.println(m_signalRelais);
      if (m_signalRelais > 0) {
        digitalWrite(m_signalRelais, false);
      }
      m_activateSignal = UINT32_MAX;
    }

      // Signalanzeiger zurücksetzen wenn zu diesem Objekt ein Anzeiger definiert wurde.
    if (now > m_deactivateSignal) {
      Serial.println("Signal cleared.");
      if (m_signalRelais > 0) {
        digitalWrite(m_signalRelais, true);
      }
      m_deactivateSignal = UINT32_MAX;

      if (m_mwtInput) {
        // wenn das Objekt eine MWT definiert hat, auf die Betätigung warten.
        Serial.println("Wait for MWt.");
        m_state = WS_WAITMWT;
      } else {
        // andernfalls direkt das Rückblocken anstoßen.
        Serial.println("Direct unlock.");
        m_state = WS_UNBLOCK;
        m_unblock = now + DELTA_UNBLOCK;
      }
    }

    if (now > m_unblock) {
      // Zeitpunkt für Rückblocken erreicht. Rückblocken starten
      // auf Rücknahme des Befehlsabgabe-Hebels warten.
      Serial.println("Wait for release.");
      m_blockPort->block();
      m_state = WS_WAITRELEASE;
      m_unblock = UINT32_MAX;
    }
}