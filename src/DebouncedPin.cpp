#include <Arduino.h>
#include "DebouncedPin.hpp"

#define SHIFT_OUT_BIT 0x40
#define ONE_THRESHOLD 3
#define SHIFT_COUNT 7

DebouncedPin::DebouncedPin() {
  m_pin = 0xff;
}

/**
 * Initializes the input pin and start state.
 **/
void DebouncedPin::init(uint8_t pin, unsigned long deltaReadMillis){
  m_pin = pin;
  m_deltaReadMillis = deltaReadMillis;
  m_nextRead = 0;

  pinMode(pin, INPUT_PULLUP);
  
  m_history = 0;
  m_count = 0;

  for (uint8_t i = 0; i < SHIFT_COUNT; i++) {
    update();
  }
}

/**
 * Reads the digital input pin and updates
 * the debouncer logic.
 **/
void DebouncedPin::update() {
  uint8_t actValue = digitalRead(m_pin);
  if (m_history & SHIFT_OUT_BIT) {
    m_count--;
  }

  if (actValue) {
    m_count++;
  }

  m_history = (m_history << 1) | (actValue & 1);
}

/**
 * Returns the actual state of the
 * debounced input pin.
 **/
bool DebouncedPin::read() {
  return m_count > ONE_THRESHOLD;
}

bool DebouncedPin::tick(unsigned long now) {
  if (now > m_nextRead) {
    update();
    m_nextRead = now + m_deltaReadMillis;
    return true;
  } else {
    return false;
  }
}