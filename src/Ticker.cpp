#include "Ticker.hpp"

/**
 * Erzeugt die Wechselspannung für die Blockumschaltung.
 * Hierzu werden zwei Umschalt-Relais betätigt, welche
 * Plus und Minus Pol gegenseitig geschaltet haben.
 * 
 * Der Ticker kann mehrfach überlappend aufgerufen werden.
 * Er läuft dann so lange, dass der letzte Aufruf komplett
 * abgearbeitet werden kann.
 **/
Ticker::Ticker(uint8_t port, int stateDurationMillis, int totalDurationMillis, int x) {
  Serial.print("Create Ticker "); Serial.println((long)this, HEX);
  delay(1000);
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

/**
 * Meldet zurück, ob der Ticker gerade aktiv ist.
 **/
bool Ticker::isRunning() {
  return m_count != 0;
}

/**
 * Startet den Ticker für einen Umschaltvorgang. Der
 * Aufruf kehrt sofort zurück, die Abarbeitung erfolgt
 * asynchron in der tick Methode.
 **/
void Ticker::start() {
  unsigned long now = millis();

  if (m_count == 0) {
    m_nextChange = now + m_stateDurationMillis;
  }

  m_count++;
  Serial.print("Starting "); Serial.println(m_count);
  m_tickerDone = now + m_totalDurationMillis;
}

/**
 * Beendet einen Ticker-Aufruf. Falls mehrere Aufrufe
 * aktiv sind, wird nur der Counter herunter gezählt.
 * Sobald der letzte Tickeraufruf beendet wurde, wird
 * der asynchrone Ticker gestoppt.
 **/
void Ticker::end() {
  m_count--;

  if (m_count == 0) {
    Serial.println("All ticker stopped.");
    m_tickerDone = 0;
    m_actState = true;
    digitalWrite(m_port, m_actState);
  }
}

/**
 * Weiterleitung aus der Arduino loop Funktion, wird
 * für die asynchronen Vorgänge benötigt und muss
 * regelmäßig aufgerufen werden.
 **/
void Ticker::tick(unsigned long now) {
  // Falls ein end Kommando fehlt, wird über diesen
  // Vorgang die maximale Tickerlaufzeit geprüft und
  // bei Bedarf der Ticker nach einem Timeout gestoppt.
  if ((m_tickerDone != 0) && (now > m_tickerDone)) {
    Serial.println("Watchdog ticker stopped.");
    m_tickerDone = 0;
    m_count = 0;
    m_actState = true;
    digitalWrite(m_port, m_actState);
  }

  if (m_count > 0) {
    // Solange mindestens ein Tickervorgang aktiv ist,
    // wird alle m_stateDurationMillis der Relais-
    // ausgang umgeschaltet.
    if (now > m_nextChange) {
      m_actState = !m_actState;
      digitalWrite(m_port, m_actState);
      m_nextChange = now + + m_stateDurationMillis;
    }
  }
}