#include "PortDefinitions.hpp"
#include "InputState.hpp"
#include "CommandProcessor.hpp"

/**
 * Führt die Kommandos von der seriellen Schnittstelle zu
 * den Bahnhofs- und Streckenblöcken aus.
 **/
CommandProcessor::CommandProcessor(Ticker *ticker) {
  m_ticker = ticker;
  state = new InputState();

  for (uint8_t i = 0; i < NUMBER_OF_ZUSTIMMUNGEN; i++) {
    Zustimmung[i] = new BlockPort(ticker, magnetports[i], BLOCK_DURATION);
  }

  for (uint8_t i = 0; i < NUMBER_OF_STRECKENBLOCK; i++) {
    Streckenblock[i] = new BlockPort(ticker, magnetports[i + NUMBER_OF_ZUSTIMMUNGEN], BLOCK_DURATION);
  }

  // Sonderbehandlung für die vier Relais mit umgekehrter Polung
  Streckenblock[4]->setInvers();
  Streckenblock[5]->setInvers();
}

/**
 * Liefert den Zustand der Erlaubsnisfelder in den Bits 0 (LB)
 * und 1 (AH) zurück.
 **/
uint8_t CommandProcessor::getDirectionLB_AH() {
  uint8_t result = 0;
  if (state->Sb_ErlLB) {
    result |= 1;
  }

  if (state->Sb_ErlAH) {
    result |= 2;
  }

  return result;
}

/**
 * Liefert den Zustand der Anfangs- und Endfelder in den Bits 0 (LB)
 * und 1 (AH) zurück. Nur wenn beide Frei sind, wird das jeweilge
 * Frei Bit gesetzt.
 **/
uint8_t CommandProcessor::getFreeStateLB_AH() {
  //state->print();
  uint8_t result = 0;
  if (!state->Sb_AnfLB && state->Sb_EndLB) {
    result |= 1;
  }

  if (!state->Sb_AnfAH && state->Sb_EndAH) {
    result |= 2;
  }

  return result;
}

/**
 * Prüft den aktuellen Zustand und führt - falls erlaubt - die
 * Änderung des jeweiligen Blocks durch.
 **/
void CommandProcessor::execute(uint8_t command, bool hasGleisbild) {
  Serial.print("Process command "); Serial.println(command);
  state->print();

  switch (command) {
    case CMD_ZUST_A1:
      if (state->ZustimmungAllA && !m_ticker->isRunning() && (!state->Sb_EndLB || hasGleisbild)) {
        Zustimmung[0]->block();
      }
      break;

    case CMD_ZUST_A2:
      if (state->ZustimmungAllA && state->ZustimmungAllB && state->ZustimmungF && !m_ticker->isRunning() && (!state->Sb_EndLB || hasGleisbild)) {
        Zustimmung[1]->block();
      }
      break;

    case CMD_ZUST_A4:
      if (state->ZustimmungAllA && state->ZustimmungAllB && state->ZustimmungF && !m_ticker->isRunning() && (!state->Sb_EndLB || hasGleisbild)) {
        Zustimmung[2]->block();
      }
      break;

    case CMD_ZUST_B2:
      if (state->ZustimmungA2 && state->ZustimmungA4 && state->ZustimmungAllB && state->ZustimmungF && !m_ticker->isRunning() && (!state->Sb_EndAH || hasGleisbild)) {
        Zustimmung[3]->block();
      }
      break;

    case CMD_ZUST_B4:
      if (state->ZustimmungA2 && state->ZustimmungAllB && state->ZustimmungF && !m_ticker->isRunning() && (!state->Sb_EndAH || hasGleisbild)) {
        Zustimmung[4]->block();
      }
      break;

    case CMD_ZUST_F:
      if (state->ZustimmungA2 && state->ZustimmungAllB && state->ZustimmungF && !m_ticker->isRunning()) {
        Zustimmung[5]->block();
      }
      break;

    case CMD_STRB_LB_ANF:
      if (state->Sb_AnfLB && !state->Sb_ErlLB && state->Sb_EndLB) {
        Streckenblock[0]->block();
      }
      break;

    case CMD_STRB_LB_ERL:
      if (!state->Sb_AnfLB && state->Sb_ErlLB && state->Sb_EndLB) {
        Streckenblock[1]->block();
      }
      break;

    case CMD_STRB_LB_END:
      if (!state->Sb_AnfLB && state->Sb_ErlLB && state->Sb_EndLB) {
        Streckenblock[2]->block();
      }
      break;

    case CMD_STRB_AH_ANF:
      if (state->Sb_AnfAH && !state->Sb_ErlAH && state->Sb_EndAH) {
        Streckenblock[3]->block();
      }
      break;

    case CMD_STRB_AH_ERL:
      if (!state->Sb_AnfAH && state->Sb_ErlAH && state->Sb_EndAH) {
        Streckenblock[4]->block();
      }
      break;

    case CMD_STRB_AH_END:
      if (!state->Sb_AnfAH && state->Sb_ErlAH && state->Sb_EndAH) {
        Streckenblock[5]->block();
      }
      break;
  }
}

/**
 * Weiterleitung aus der Arduino loop Funktion, wird
 * für die asynchronen Vorgänge benötigt und muss
 * regelmäßig aufgerufen werden.
 **/
void CommandProcessor::tick(unsigned long now) {
  m_ticker->tick(now);
  state->tick(now);

  for (uint8_t i = 0; i < NUMBER_OF_ZUSTIMMUNGEN; i++) {
    Zustimmung[i]->tick(now);
  }

  for (uint8_t i = 0; i < NUMBER_OF_STRECKENBLOCK; i++) {
    Streckenblock[i]->tick(now);
  }
}