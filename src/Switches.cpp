#include "Switches.hpp"
#include "Wire.h"

#define WAIT_TIME 200
#define MCP23017_I2CADD_DEFAULT 0x20

/**
 * Liest die Fahrstraßenschalter über einen Port
 * Expander ein. Das Einlesen erfolgt asynchron,
 * der Wert wird lokal zwischengespeichert.
 **/
Switches::Switches(Ticker *ticker) {
  m_ticker = ticker;
  m_nextTick = 0ul;
  m_lastValue = 0xfffffu;
  state = 3;

  init_MCP(MCP23017_I2CADD_DEFAULT);
  init_MCP(MCP23017_I2CADD_DEFAULT | 4);
}

/**
 * Liefert den aktuellen Zustand der Fahrstraßen-
 * schalter aus dem letzten Einlesevorgang.
 **/
unsigned long Switches::getActValue() {
  return m_lastValue;
}

/**
 * Weiterleitung aus der Arduino loop Funktion, wird
 * für die asynchronen Vorgänge benötigt und muss
 * regelmäßig aufgerufen werden.
 **/
void Switches::tick(unsigned long now) {
  if ((now < m_nextTick) || m_ticker->isRunning()) {
    // Wenn gerade ein Block umgeschaltet wird, gibt es viele
    // Störungen auf der Leitung welche dazu führen könnnen,
    // das die Arduino Wire Library hängen bleibt. Deshalb 
    // wird während dieser Zeit nichts eingelesen.
    return;
  }

  uint8_t actStateValue = ((m_lastValue >> (state << 2)) & 0xf) | ((state + 8) << 4);
  Serial1.write(actStateValue);

  state++;
  if (state == 5) {
    m_lastValue = read();
    state = 0;
  }

  m_nextTick = now + WAIT_TIME;
}

/**
 * Initialisiert die Port Expander. Alle Ports werden
 * zu Eingängen mit PullUp Widerstand gesetzt.
 **/
void Switches::init_MCP(uint8_t i2CAddr) {
  Serial.println("Init MCP 0");
  // all input
  sendI2C(i2CAddr, 0, 0xff);
  sendI2C(i2CAddr, 1, 0xff); 
  sendI2C(i2CAddr, 2, 0xff);
  sendI2C(i2CAddr, 3, 0xff); 

  // activate pull up
  sendI2C(i2CAddr, 0xc, 0xff);
  sendI2C(i2CAddr, 0xd, 0xff);
  Serial.println("Init done.");
}

/**
 * Liest den aktuellen Zustand der vier 8 Bit
 * Port Expander Register ein.
 **/
unsigned long Switches::read() {
  uint8_t addr = MCP23017_I2CADD_DEFAULT;
  uint8_t reg1 = readRegister(addr, 0x12);
  uint8_t reg2 = readRegister(addr, 0x13);
  uint8_t reg3 = readRegister(addr | 4, 0x13);
  //Serial.print("Reg 1: "); Serial.print(reg1, HEX); Serial.print(", Reg 2: "); Serial.print(reg2, HEX); Serial.print(", Reg 3: "); Serial.print(reg3, HEX);

  unsigned long result = 0;
  fillup(result, mapper1, reg1 >> 2, 6);
  fillup(result, mapper2, reg2, 6);
  fillup(result, mapper3, reg3, 8);

  //Serial.print(", Result: "); Serial.println(result, HEX);
  return ~result;
}

/**
 * Überträgt den 8 Bit Teilwert in ein long Wert an
 * der entsprechenden Stelle ein. Der mapper Wert
 * übergibt die Umrechnung aus der Schalternummer
 * auf die Fahrstraßennummer.
 **/
void Switches::fillup(unsigned long &result, unsigned long *mapper, uint8_t source, uint8_t size) {
  for (uint8_t i = 0; i < size; i++) {
    if (source & 1) {
      result |= mapper[i];
    }

    source >>= 1;
  }
}

/**
 * Sendet ein Kommando zum Port Expander
 **/
void Switches::sendI2C(uint8_t addr, uint8_t cmd, uint8_t value) {
    Wire.beginTransmission(addr);
    Wire.write(cmd);
    Wire.write(value);
    Wire.endTransmission();
}

/**
 * Liest ein Port Expander Register ein.
 **/
uint8_t Switches::readRegister(uint8_t addr, uint8_t regNo) {
    digitalWrite(LED_BUILTIN, true);
    Wire.beginTransmission(addr);
    Wire.write(regNo);
    Wire.endTransmission();
    Wire.requestFrom(addr, (uint8_t)1);
    short b1 = Wire.read();
    digitalWrite(LED_BUILTIN, false);
    return b1;
}

