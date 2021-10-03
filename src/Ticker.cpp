#include "Ticker.hpp"


Ticker::Ticker(uint8_t port, int stateDurationMillis, int totalDurationMillis) {
  m_port = port;
  m_count = 0;
  m_tickerDone = 0;
  m_nextChange = 0;
  m_actState = true;
  m_stateDurationMillis = stateDurationMillis;
  m_totalDurationMillis = totalDurationMillis;
  pinMode(m_port, OUTPUT);
  digitalWrite(m_port, m_actState);
}

bool Ticker::isRunning() {
  return m_count != 0;
}

void Ticker::start() {
  unsigned long now = millis();
  Serial.println("Ticker started.");

  if (m_count == 0) {
    m_nextChange = now + m_stateDurationMillis;
  }

  m_count++;
  m_tickerDone = now + m_totalDurationMillis;
}

void Ticker::end() {
  m_count--;
  Serial.print("Ticker stopped, count: "); Serial.println(m_count);

  if (m_count == 0) {
    Serial.println("All ticker stopped.");
    m_tickerDone = 0;
    m_actState = true;
    digitalWrite(m_port, m_actState);
  }
}

void Ticker::tick(unsigned long now) {
  // watchdog
  if ((m_tickerDone != 0) && (now > m_tickerDone)) {
    Serial.println("Watchdog ticker stopped.");
    m_tickerDone = 0;
    m_count = 0;
    m_actState = true;
    digitalWrite(m_port, m_actState);
  }

  if (m_count > 0) {
    if (now > m_nextChange) {
      m_actState = !m_actState;
      digitalWrite(m_port, m_actState);
      m_nextChange = now + + m_stateDurationMillis;
    }
  }
}