#include "InputState.hpp"
#include "PortDefinitions.hpp"

/**
 * Liest den aktuellen Zustand der Bahnhofs- und
 * Streckenblöcke ein und meldet diesen an das
 * CommandProcessor Modul zurück zur Aktivierung
 * der jeweiligen Blöcke.
 **/
InputState::InputState() {
  for (uint8_t i = 0; i < 6; i++) {
    pinMode(zustimmungsports[i], INPUT_PULLUP);
  }

  for (uint8_t i = 0; i < 6; i++) {
    pinMode(streckenblockports[i], INPUT_PULLUP);
  }
}

/**
 * Weiterleitung aus der Arduino loop Funktion, wird
 * für die asynchronen Vorgänge benötigt und muss
 * regelmäßig aufgerufen werden.
 **/
void InputState::tick(unsigned long now) {
  readAllInputs();
  calcCompounds();
}

/**
 * Liest die Arduino Input-Pins ein und
 * trägt den Status in Variablen ein.
 **/
void InputState::readAllInputs() {
  ZustimmungA1 = digitalRead(IN_A1);
  ZustimmungA2 = digitalRead(IN_A2);
  ZustimmungA4 = digitalRead(IN_A4);
  ZustimmungB2 = digitalRead(IN_B2);
  ZustimmungB4 = digitalRead(IN_B4);
  ZustimmungF = digitalRead(IN_F);

  Sb_AnfLB = digitalRead(IN_LB_ANF);
  Sb_ErlLB = digitalRead(IN_LB_ERL);
  Sb_EndLB = digitalRead(IN_LB_END);

  Sb_AnfAH = digitalRead(IN_AH_ANF);
  Sb_ErlAH = digitalRead(IN_AH_ERL);
  Sb_EndAH = digitalRead(IN_AH_END);

}

/**
 * Gruppierung einiger Zustimmungen die an
 * einigen Stellen gemeinsam benötigt werden.
 **/
void InputState::calcCompounds() {
  ZustimmungAllA = ZustimmungA1 && ZustimmungA2 && ZustimmungA4;
  ZustimmungAllB = ZustimmungB2 && ZustimmungB4;
}

/**
 * Testausgabe aller internen Zustände.
 **/
void InputState::print() {
  Serial.print("A1: "), Serial.print( ZustimmungA1 ); 
  Serial.print(", A2: "), Serial.print( ZustimmungA2 ); 
  Serial.print(", A4: "), Serial.print( ZustimmungA4 ); 
  Serial.print(", B2: "), Serial.print( ZustimmungB2 ); 
  Serial.print(", B4: "), Serial.print( ZustimmungB4 ); 
  Serial.print(", F: "), Serial.println( ZustimmungF ); 

  Serial.print("Anf LB: "), Serial.print( Sb_AnfLB ); 
  Serial.print(", Erl LB: "), Serial.print( Sb_ErlLB ); 
  Serial.print(", End LB: "), Serial.print( Sb_EndLB ); 
  Serial.print(", Anf AH: "), Serial.print( Sb_AnfAH ); 
  Serial.print(", Erl AH: "), Serial.print( Sb_ErlAH ); 
  Serial.print(", End AH: "), Serial.println( Sb_EndAH ); 
}
