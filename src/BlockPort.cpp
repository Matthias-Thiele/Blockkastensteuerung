#include "BlockPort.hpp"

/**
 * Ein BlockPort verwaltet einen Wechselspannungsport
 * aus dem Blockkasten. Die Software kennt nicht den
 * aktuellen Stand und kann auch nur an zulässigen
 * Zeitpunkten den Zustand ändern. Andernfalls wird
 * die Änderung durch die Stellwerkstechnik verhindert.
 * Die Aufrufende Stelle muss selber prüfen, ob die
 * gewünschte Umschaltung zu diesem Zeitpunkt zulässig ist.
 **/
BlockPort::BlockPort(Ticker *ticker, uint8_t outPort, uint16_t blockDuration) {
  m_outPort = outPort;
  m_ticker = ticker;
  m_blockDuration = blockDuration;
  m_endBlock = 0;
  m_isInvers = false;
  pinMode(m_outPort, OUTPUT);
  digitalWrite(m_outPort, true);
}

/**
 * Startet eine Umschaltung dieses Strecken- oder Bahnhofsblocks.
 * Der Aufruf kommt sofort zurück, die notwendige Zahl der
 * Wechselstromzyklen wird intern durchgeführt. Es dürfen
 * mehrere Blöcke gleichzeitig bewegt werden.
 **/
void BlockPort::block() {
  Serial.println("Block started.");
  m_endBlock = millis() + m_blockDuration;
  m_ticker->start();
  digitalWrite(m_outPort, m_isInvers);
}

// vier Relais mit umgekehrter Polung im Streckenblock Althengstett
void BlockPort::setInvers() {
  m_isInvers = true;
  digitalWrite(m_outPort, false);
}

/**
 * Weiterleitung aus der Arduino loop Funktion, wird
 * für die asynchronen Vorgänge benötigt und muss
 * regelmäßig aufgerufen werden.
 **/
void BlockPort::tick(unsigned long now) {
  if (m_endBlock > 0) {
    if (now > m_endBlock) {
      m_ticker->end();
      digitalWrite(m_outPort, !m_isInvers);
      Serial.print("Block end: "); Serial.println(m_endBlock);
      m_endBlock = 0;
    }
  }
}
