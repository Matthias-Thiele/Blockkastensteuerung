#include "Switches.hpp"
#include "Wire.h"

#define WAIT_TIME 200
#define MCP23017_I2CADD_DEFAULT 0x20

Switches::Switches() {
  Serial1.begin(9600);
  Wire.begin();

  m_nextTick = 0ul;
  m_lastValue = 0xfffffu;
  state = 3;

  init_MCP(MCP23017_I2CADD_DEFAULT);
  init_MCP(MCP23017_I2CADD_DEFAULT | 4);
}

void Switches::tick(unsigned long now) {
  if (now < m_nextTick) {
    return;
  }

  uint8_t actStateValue = ((m_lastValue >> (state << 2)) & 0xf) | ((state + 8) << 4);
  Serial.print("Switch value: "); Serial.println(actStateValue, HEX);
  Serial2.write(actStateValue);

  state++;
  if (state == 5) {
    m_lastValue = read();
    Serial.print("Value read: "); Serial.println(m_lastValue, HEX);
    state = 0;
  }

  m_nextTick = now + WAIT_TIME;
}

unsigned long Switches::getActValue() {
  return m_lastValue;
}

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

unsigned long Switches::read() {
  uint8_t addr = MCP23017_I2CADD_DEFAULT;
  uint8_t reg1 = readRegister(addr, 0x12);
  uint8_t reg2 = readRegister(addr, 0x13);
  uint8_t reg3 = readRegister(addr | 4, 0x13);
  Serial.print("Reg 1: "); Serial.print(reg1, HEX); Serial.print(", Reg 2: "); Serial.print(reg2, HEX); Serial.print(", Reg 3: "); Serial.print(reg3, HEX);

  unsigned long result = 0;
  fillup(result, mapper1, reg1 >> 2, 6);
  fillup(result, mapper2, reg2, 6);
  fillup(result, mapper3, reg3, 8);

  Serial.print(", Result: "); Serial.println(result, HEX);
  return ~result;
}

void Switches::fillup(unsigned long &result, unsigned long *mapper, uint8_t source, uint8_t size) {
  for (uint8_t i = 0; i < size; i++) {
    if (source & 1) {
      result |= mapper[i];
    }

    source >>= 1;
  }
}

void Switches::sendI2C(uint8_t addr, uint8_t cmd, uint8_t value) {
    Wire.beginTransmission(addr);
    Wire.write(cmd);
    Wire.write(value);
    Wire.endTransmission();
}

uint8_t Switches::readRegister(uint8_t addr, uint8_t regNo) {
    Wire.beginTransmission(addr);
    Wire.write(regNo);
    Wire.endTransmission();

    Wire.requestFrom(addr, (uint8_t)1);
    short b1 = Wire.read();
    return b1;
}

