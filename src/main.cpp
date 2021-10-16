#include <Arduino.h>
#include <avr/wdt.h>
#include "PortDefinitions.hpp"
#include "Ticker.hpp"
#include "BlockPort.hpp"
#include "IRremote.h"
#include "CommandProcessor.hpp"
#include "Switches.hpp"
#include "WaerterInput.hpp"
#include "DebouncedPin.hpp"
#include "Streckenblock.hpp"
#include "Wire.h"



// IR Fernsteuerung Calw
#define IR_CMD_A1 16769055 // 7
#define IR_CMD_A2 16754775 // 8
#define IR_CMD_A4 16748655 // 9
#define IR_CMD_B2 16738455 // *
#define IR_CMD_B4 16750695 // 0
#define IR_CMD_F 16756815 // #
#define IR_CMD_LB_ANF 16753245 // 1
#define IR_CMD_LB_ERL 16736925 // 2
#define IR_CMD_LB_END 16769565 // 3
#define IR_CMD_AH_ANF 16720605 // 4
#define IR_CMD_AH_ERL 16712445 // 5
#define IR_CMD_AH_END 16761405 // 6
#define IR_CMD_WB_ANF 0xFF10EF // Links
#define IR_CMD_WB_ERL 0xFF4AB5 // Unten
#define IR_CMD_WB_END 0xFF5AA5 // Rechts
#define IR_CMD_PAUSE 0xFF38C7 // OK

#define IR_CMD_RES_1 0xFF18E7 // Oben


#define BEFEHLSABGABE_EINFAHRT 46
#define BEFEHLSABGABE_AUSFAHRT 47
#define SIGNALMELDER_H 49
#define SIGNALMELDER_M 48
#define START_BEFEHLSABGABE_EINFAHRT 43
#define START_BEFEHLSABGABE_AUSFAHRT 44
#define MWT_FAHRDIENSTLEITER_INPUT 50

#define WB_ANFANG 37
#define WB_ERLAUBNIS 39
#define WB_END 38
#define WB_AUFTRAG 41
#define WB_MELDELAMPE 40
#define WB_BEFEHL_VON 43
#define WB_BEFEHL_NACH 44

// Aktualisierungsintervall für die Abfrage der Fahrstraßenschalter
#define DELTA_FAHRSTRASSEN_SEND 200

Ticker *ticker;
decode_results results;
int RECV_PIN1 = 2;          
IRrecv irrecv(RECV_PIN1);  
CommandProcessor *cmdProc;
unsigned long nextSend;
unsigned long testStart;
Switches *switches;

// Wärtersteuerung
DebouncedPin triggerWaerterEinfahrt;
DebouncedPin triggerWaerterAusfahrt;
DebouncedPin mwtFahrdienstleiter;
BlockPort *waerterEinfahrt;
BlockPort *waerterAusfahrt;
WaerterInput *waerterSteuerung[4];
WaerterInput *gleisbildsteuerungEinfahrt;
WaerterInput *gleisbildsteuerungAusfahrt;
Streckenblock *wildbergStreckenblock;

bool hasGleisbild = false;
unsigned long watchdocGBCommand = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("Sketch started.");

  Serial1.begin(9600);
  Wire.begin();
  Wire.setClock(20000);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, false);
  
  pinMode(SIGNALMELDER_M, OUTPUT);
  digitalWrite(SIGNALMELDER_M, true);
  pinMode(SIGNALMELDER_H, OUTPUT);
  digitalWrite(SIGNALMELDER_H, true);

  ticker = new Ticker(OUT_BLINK, 100, 8000,1);
  cmdProc = new CommandProcessor(ticker);
  switches = new Switches(ticker);

  triggerWaerterEinfahrt.init(START_BEFEHLSABGABE_EINFAHRT, 50);
  triggerWaerterAusfahrt.init(START_BEFEHLSABGABE_AUSFAHRT, 50);
  mwtFahrdienstleiter.init(MWT_FAHRDIENSTLEITER_INPUT, 50);

  wildbergStreckenblock = new Streckenblock(WB_ANFANG, WB_ERLAUBNIS, WB_END, WB_AUFTRAG, WB_MELDELAMPE, WB_BEFEHL_VON, WB_BEFEHL_NACH);

  waerterEinfahrt = new BlockPort(ticker, BEFEHLSABGABE_EINFAHRT, 6000);
  waerterAusfahrt = new BlockPort(ticker, BEFEHLSABGABE_AUSFAHRT, 6000);
  waerterSteuerung[0] = new WaerterInput(wildbergStreckenblock, ticker, switches, waerterEinfahrt, 1ul << 7, &triggerWaerterEinfahrt, NULL, SIGNALMELDER_M);
  waerterSteuerung[1] = new WaerterInput(wildbergStreckenblock, ticker, switches, waerterEinfahrt, 3ul << 8, &triggerWaerterEinfahrt, &mwtFahrdienstleiter, SIGNALMELDER_M);
  waerterSteuerung[2] = new WaerterInput(wildbergStreckenblock, ticker, switches, waerterAusfahrt, 1ul << 17, &triggerWaerterAusfahrt, NULL, SIGNALMELDER_H);
  waerterSteuerung[3] = new WaerterInput(wildbergStreckenblock, ticker, switches, waerterAusfahrt, 3ul << 18, &triggerWaerterAusfahrt, NULL, 0);
  gleisbildsteuerungEinfahrt = new WaerterInput(wildbergStreckenblock, ticker, switches, waerterEinfahrt, 7ul << 7, NULL, &mwtFahrdienstleiter, 0);
  gleisbildsteuerungAusfahrt = new WaerterInput(wildbergStreckenblock, ticker, switches, waerterAusfahrt, 7ul << 17, NULL, NULL, 0);

  for (uint8_t ii = 0; ii < 4; ii++) {
    Serial.print("WI "); Serial.print(ii); Serial.print(" : "); Serial.println((unsigned long) waerterSteuerung[ii], HEX);
  }

  testStart = millis();
  nextSend = 0;

  irrecv.enableIRIn();
  wdt_enable(WDTO_8S);
}

uint8_t state = 0;
void receiveIRCommand();
void receiveSerialCommand(unsigned long now);
void sendStreckenblock(unsigned long now);

void loop() {
  wdt_reset();
  unsigned long now = millis();
  hasGleisbild = now < watchdocGBCommand;

  ticker->tick(now);
  cmdProc->tick(now);
  triggerWaerterEinfahrt.tick(now);
  triggerWaerterAusfahrt.tick(now);
  mwtFahrdienstleiter.tick(now);
  waerterEinfahrt->tick(now);
  waerterAusfahrt->tick(now);

  if (hasGleisbild) {
    // Wärtersteuerung über Gleisbild
    gleisbildsteuerungEinfahrt->tick(now);
    gleisbildsteuerungAusfahrt->tick(now);

    static unsigned long lastTriggerSent = 0ul;
    if (now > lastTriggerSent) {
      lastTriggerSent = now + 500;
      uint8_t cmd = 0;
      if (triggerWaerterAusfahrt.read()) {
        cmd |= 1;
      }

      if (triggerWaerterEinfahrt.read()) {
        cmd |= 2;
      }

      //Serial.print("Befehlsabgaben: "); Serial.println(cmd, HEX);
      Serial1.write(cmd);
    }
  } else {
    // Wärtersteuerung über Warteschleifen
    for (uint8_t i = 0; i < 4; i++) {
      waerterSteuerung[i]->tick(now);
    }
  }

  receiveIRCommand();
  receiveSerialCommand(now);
  switches->tick(now);
  wildbergStreckenblock->tick(now);

  if (hasGleisbild) {
    sendStreckenblock(now);
  }
}

/**
 * Sendet abwechselnd die Richtung der Streckenblöcke
 * und den Frei-Status.
 **/
void sendStreckenblock(unsigned long now) {
  static unsigned long nextAction = 0;
  static bool sendDirection = true;

  if (now > nextAction) {
    nextAction = now + 300;

    uint8_t result;
    if (sendDirection) {
      // sende die Richtung der Streckenblöcke
      result = cmdProc->getDirectionLB_AH();
      if (!wildbergStreckenblock->hasErlaubnis()) {
        result |= 4;
      }
    } else {
      // sende den Frei-Status der Streckenblöcke
      result = 8 | cmdProc->getFreeStateLB_AH();
      if (wildbergStreckenblock->isFree()){
        result |= 4;
      }
    }

    Serial1.write(0x40 | result);
    //Serial.print("Streckenblock: "); Serial.println(result, HEX);
    sendDirection = !sendDirection;
  }
}

/**
 * Prüft nach, ob ein Kommando von der seriellen
 * Schnittstelle empfangen wurde. Hier gibt es durch
 * Störungen manchmal Fehlmeldungen, diese werden einfach
 * ignoriert.
 **/
void receiveSerialCommand(unsigned long now) {
  if (Serial1.available()) {
    uint8_t cmd = Serial1.read();
    //Serial.print("Serial command "); Serial.println(cmd, HEX);
    switch (cmd) {
      case 0xd0: // Watchdog reset vom Gleisbild
        watchdocGBCommand = now + 60000ul;
        break;

      case 0xd2: // Anfangsfeld Liebenzell
        cmdProc->execute(CMD_STRB_LB_ANF, hasGleisbild);
        break;

      case 0xd3: // Endfeld Liebenzell
        cmdProc->execute(CMD_STRB_LB_END, hasGleisbild);
        break;

      case 0xd4: // Anfangsfeld Althengstett
        cmdProc->execute(CMD_STRB_AH_ANF, hasGleisbild);
        break;

      case 0xd5: // Endfeld Althengstett
        cmdProc->execute(CMD_STRB_AH_END, hasGleisbild);
        break;

      case 0xd6: // Anfangsfeld Wildberg
        wildbergStreckenblock->setAnfang();
        break;

      case 0xd7: // Endfeld Wildberg
        wildbergStreckenblock->setEnd();
        break;

      case 0xd8: // Einfahrt Wildberg auf Gleis 1 abgeschlossen
        gleisbildsteuerungEinfahrt->startRueckblocken(now, false);
        break;

      case 0xd9: // Einfahrt Wildberg auf Gleis 2 oder 4 abgeschlossen
        gleisbildsteuerungEinfahrt->startRueckblocken(now, true);
        break;

      case 0xda: // Ausfahrt Wildberg von Gleis 1, 2, 4 abgeschlossen
        gleisbildsteuerungAusfahrt->startRueckblocken(now, false);
        break;

      case 0xdb: // Anfangsfeld Wildberg
        wildbergStreckenblock->resetAnfang();
        break;

      case 0xdc: // Endfeld Wildberg
        wildbergStreckenblock->resetEnd();
        break;

      case 0xe0: // Signalmelder M aus
        //Serial.println("Signalmelder M aus");
        digitalWrite(SIGNALMELDER_M, true);
        break;

      case 0xe8: // Signalmelder M ein
        //Serial.println("Signalmelder M ein");
        digitalWrite(SIGNALMELDER_M, false);
        break;

      case 0xe1: // Signalmelder H aus
        //Serial.println("Signalmelder H aus");
        digitalWrite(SIGNALMELDER_H, true);
        break;

      case 0xe9: // Signalmelder H ein
        //Serial.println("Signalmelder H ein");
        digitalWrite(SIGNALMELDER_H, false);
        break;
    }
  }
}


/**
 * Prüft nach, ob ein Kommando von der IR Schnittstelle
 * empfangen wurde. Hier gibt es durch Störungen und
 * Streulicht viele Fehlmeldungen, diese werden einfach
 * ignoriert.
 **/
void receiveIRCommand() {

  if (irrecv.decode(&results)) {
   //Serial.println(results.value, HEX);
   irrecv.resume();

   switch (results.value) {
     case IR_CMD_A1:
       cmdProc->execute(CMD_ZUST_A1, hasGleisbild);
       break;
       
     case IR_CMD_A2:
       cmdProc->execute(CMD_ZUST_A2, hasGleisbild);
       break;
       
     case IR_CMD_A4:
       cmdProc->execute(CMD_ZUST_A4, hasGleisbild);
       break;
       
     case IR_CMD_B2:
       cmdProc->execute(CMD_ZUST_B2, hasGleisbild);
       break;
       
     case IR_CMD_B4:
       cmdProc->execute(CMD_ZUST_B4, hasGleisbild);
       break;
       
     case IR_CMD_F:
       cmdProc->execute(CMD_ZUST_F, hasGleisbild);
       break;
       
     case IR_CMD_LB_ANF:
       cmdProc->execute(CMD_STRB_LB_ANF, hasGleisbild);
       break;
       
     case IR_CMD_LB_ERL:
       cmdProc->execute(CMD_STRB_LB_ERL, hasGleisbild);
       break;
       
     case IR_CMD_LB_END:
       cmdProc->execute(CMD_STRB_LB_END, hasGleisbild);
       break;
       
     case IR_CMD_AH_ANF:
       cmdProc->execute(CMD_STRB_AH_ANF, hasGleisbild);
       break;
       
     case IR_CMD_AH_ERL:
       cmdProc->execute(CMD_STRB_AH_ERL, hasGleisbild);
       break;
       
     case IR_CMD_AH_END:
       cmdProc->execute(CMD_STRB_AH_END, hasGleisbild);
       break;
       
     case IR_CMD_WB_ANF:
       wildbergStreckenblock->resetAnfang();
       break;

     case IR_CMD_WB_ERL:
        wildbergStreckenblock->aquire();
        break;

     case IR_CMD_WB_END:
        wildbergStreckenblock->resetEnd();
        break; 

     case IR_CMD_PAUSE:
        Serial1.write(0xdd);
        break;
   }
  }
}
