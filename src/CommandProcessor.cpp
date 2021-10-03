#include "PortDefinitions.hpp"
#include "InputState.hpp"
#include "CommandProcessor.hpp"

CommandProcessor::CommandProcessor() {
  ticker = new Ticker(OUT_BLINK, STATE_DURATION, BLOCK_DURATION << 1);
  state = new InputState();

  for (uint8_t i = 0; i < NUMBER_OF_ZUSTIMMUNGEN; i++) {
    Zustimmung[i] = new BlockPort(ticker, magnetports[i], BLOCK_DURATION);
  }

  for (uint8_t i = 0; i < NUMBER_OF_STRECKENBLOCK; i++) {
    Streckenblock[i] = new BlockPort(ticker, magnetports[i + NUMBER_OF_ZUSTIMMUNGEN], BLOCK_DURATION);
  }
}

void CommandProcessor::execute(uint8_t command) {
  Serial.print("Process command "); Serial.println(command);
  state->print();

  switch (command) {
    case CMD_ZUST_A1:
      if (state->ZustimmungAllA && !ticker->isRunning() && !state->Sb_EndLB) {
        Zustimmung[0]->block();
      }
      break;

    case CMD_ZUST_A2:
      if (state->ZustimmungAllA && state->ZustimmungAllB && state->ZustimmungF && !ticker->isRunning() && !state->Sb_EndLB) {
        Zustimmung[1]->block();
      }
      break;

    case CMD_ZUST_A4:
      if (state->ZustimmungAllA && state->ZustimmungAllB && state->ZustimmungF && !ticker->isRunning() && !state->Sb_EndLB) {
        Zustimmung[2]->block();
      }
      break;

    case CMD_ZUST_B2:
      if (state->ZustimmungA2 && state->ZustimmungA4 && state->ZustimmungAllB && state->ZustimmungF && !ticker->isRunning() && !state->Sb_EndAH) {
        Zustimmung[3]->block();
      }
      break;

    case CMD_ZUST_B4:
      if (state->ZustimmungA2 && state->ZustimmungAllB && state->ZustimmungF && !ticker->isRunning() && !state->Sb_EndAH) {
        Zustimmung[4]->block();
      }
      break;

    case CMD_ZUST_F:
      if (state->ZustimmungA2 && state->ZustimmungAllB && state->ZustimmungF && !ticker->isRunning()) {
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

void CommandProcessor::tick(unsigned long now) {
  ticker->tick(now);
  state->tick(now);

  for (uint8_t i = 0; i < NUMBER_OF_ZUSTIMMUNGEN; i++) {
    Zustimmung[i]->tick(now);
  }

  for (uint8_t i = 0; i < NUMBER_OF_STRECKENBLOCK; i++) {
    Streckenblock[i]->tick(now);
  }
}