#include "BlockPort.hpp"

BlockPort::BlockPort(Ticker *ticker, uint8_t outPort, uint16_t blockDuration) {
  m_outPort = outPort;
  m_ticker = ticker;
  m_blockDuration = blockDuration;
  m_endBlock = 0;
  pinMode(m_outPort, OUTPUT);
  digitalWrite(m_outPort, true);
}

void BlockPort::block() {
  Serial.println("Block started.");
  m_endBlock = millis() + m_blockDuration;
  m_ticker->start();
  digitalWrite(m_outPort, false);
}

void BlockPort::tick(unsigned long now) {
  if (m_endBlock > 0) {
    if (now > m_endBlock) {
      m_ticker->end();
      digitalWrite(m_outPort, true);
      m_endBlock = 0;
      Serial.println("Block end.");
    }
  }
}
