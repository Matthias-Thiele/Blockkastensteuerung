#include <Arduino.h>
#include "PortDefinitions.hpp"
#include "Ticker.hpp"
#include "BlockPort.hpp"
#include "IRremote.h"
#include "CommandProcessor.hpp"
#include "Switches.hpp"


/* 
// IR Fernsteuerung Calw
#define IR_CMD_A1 16769055
#define IR_CMD_A2 16754775
#define IR_CMD_A4 16748655
#define IR_CMD_B2 16738455
#define IR_CMD_B4 16750695
#define IR_CMD_F 16756815
#define IR_CMD_LB_ANF 16753245
#define IR_CMD_LB_ERL 16736925
#define IR_CMD_LB_END 16769565
#define IR_CMD_AH_ANF 16720605
#define IR_CMD_AH_ERL 16712445
#define IR_CMD_AH_END 16761405
*/

// IR Fernsteuerung WDS
#define IR_CMD_A1 0xe0e020df
#define IR_CMD_A2 0xe0e0a05f
#define IR_CMD_A4 0xe0e0609f
#define IR_CMD_B2 0xe0e010ef
#define IR_CMD_B4 0xe0e0906f
#define IR_CMD_F 0xe0e050af
#define IR_CMD_LB_ANF 0xe0e030cf
#define IR_CMD_LB_ERL 0xe0e0b04f
#define IR_CMD_LB_END 0xe0e0708f
#define IR_CMD_AH_ANF 0xe0e0c43b
#define IR_CMD_AH_ERL 0xe0e08877
#define IR_CMD_AH_END 0xe0e0c837

#define DELTA_FAHRSTRASSEN_SEND 200

Ticker *ticker;
BlockPort *testPort;
decode_results results;
int RECV_PIN1 = 2;          
IRrecv irrecv(RECV_PIN1);  
CommandProcessor cmdProc;
unsigned long nextSend;
unsigned long testStart;
Switches switches;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115000);

  ticker = new Ticker(OUT_BLINK, 150, 8000);
  testPort = new BlockPort(ticker, 8, 5000);

  testStart = millis();
  nextSend = 0;

  irrecv.enableIRIn();
}

uint8_t state = 0;
void tickerTest();
void blockTest();
void receiveIRCommand(unsigned long now);
void receiveSerialCommand();
void sendFahrstrassen();

void loop() {
  //tickerTest();
  //blockTest();
  unsigned long now = millis();

  receiveIRCommand(now);
  receiveSerialCommand();
  switches.tick(now);
}

void receiveSerialCommand() {
  if (Serial2.available()) {
    uint8_t cmd = Serial2.read();
    switch (cmd) {
      case 0xd2: // Anfangsfeld Liebenzell
        cmdProc.execute(CMD_STRB_LB_ANF);
        break;

      case 0xd3: // Endfeld Liebenzell
        cmdProc.execute(CMD_STRB_LB_END);
        break;

      case 0xd4: // Anfangsfeld Althengstett
        cmdProc.execute(CMD_STRB_AH_ANF);
        break;

      case 0xd5: // Endfeld Althengstett
        cmdProc.execute(CMD_STRB_AH_END);
        break;

      case 0xd6: // Anfangsfeld Wildberg, TODO
        //cmdProc.execute(CMD_STRB_WB_ANF);
        break;

      case 0xd7: // Endfeld Wildberg, TODO
        //cmdProc.execute(CMD_STRB_WB_END);
        break;
    }
  }
}

void receiveIRCommand(unsigned long now) {
  ticker->tick(now);
  cmdProc.tick(now);

  if (irrecv.decode(&results)) {
   //Serial.println(results.value, HEX);
   irrecv.resume();

   switch (results.value) {
     case IR_CMD_A1:
       cmdProc.execute(CMD_ZUST_A1);
       break;
       
     case IR_CMD_A2:
       cmdProc.execute(CMD_ZUST_A2);
       break;
       
     case IR_CMD_A4:
       cmdProc.execute(CMD_ZUST_A4);
       break;
       
     case IR_CMD_B2:
       cmdProc.execute(CMD_ZUST_B2);
       break;
       
     case IR_CMD_B4:
       cmdProc.execute(CMD_ZUST_B4);
       break;
       
     case IR_CMD_F:
       cmdProc.execute(CMD_ZUST_F);
       break;
       
     case IR_CMD_LB_ANF:
       cmdProc.execute(CMD_STRB_LB_ANF);
       break;
       
     case IR_CMD_LB_ERL:
       cmdProc.execute(CMD_STRB_LB_ERL);
       break;
       
     case IR_CMD_LB_END:
       cmdProc.execute(CMD_STRB_LB_END);
       break;
       
     case IR_CMD_AH_ANF:
       cmdProc.execute(CMD_STRB_AH_ANF);
       break;
       
     case IR_CMD_AH_ERL:
       cmdProc.execute(CMD_STRB_AH_ERL);
       break;
       
     case IR_CMD_AH_END:
       cmdProc.execute(CMD_STRB_AH_END);
       break;
       
   }
  }
}

void blockTest() {
  unsigned long now = millis();

  ticker->tick(now);
  testPort->tick(now);

  switch (state) {
    case 0:
      if (now > (testStart + 3000)) {
        testPort->block();
        state = 1;
        Serial.println("To 1");
      }
      break;

    case 1:
      if (now > (testStart + 15000)) {
        state = 0;
        testStart = now;
        Serial.println("To 0");
      }
      break;
  }
}

void tickerTest() {
  unsigned long now = millis();

  ticker->tick(now);

  switch (state) {
    case 0:
      if (now > (testStart + 3000)) {
        ticker->start();
        state = 1;
        Serial.println("To 1");
      }
      break;

    case 1:
      if (now > (testStart + 8000)) {
        ticker->start();
        state = 2;
        Serial.println("To 2");
      }
      break;

    case 2:
      if (now > (testStart + 9000)) {
        ticker->end();
        state = 3;
        Serial.println("To 3");
      }
      break;

    case 3:
      if (now > (testStart + 11000)) {
        ticker->end();
        state = 4;
        Serial.println("To 4");
      }
      break;

    case 4:
      if (now > (testStart + 11000)) {
        ticker->start();
        state = 5;
        Serial.println("To 5");
      }
      break;

    case 5:
      if (now > (testStart + 30000)) {
        testStart = now;
        state = 0;
        Serial.println("To 0");
      }
      break;
  }
}