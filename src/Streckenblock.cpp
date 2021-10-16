#include "Streckenblock.hpp"

#define WAIT_RELEASE 3000;

/**
 * Simuliert den fehlenden Streckenblock aus dem
 * Wärterstellwerk in Richtung Wildberg.
 **/
Streckenblock::Streckenblock(uint8_t anfangsfeld, uint8_t erlaubnisfeld, uint8_t endfeld, uint8_t auftrag, uint8_t meldelampe, uint8_t befehlVonWB, uint8_t befehlNachWB) {
    m_anfang = anfangsfeld;
    m_erlaubnis = erlaubnisfeld;
    m_end = endfeld;
    m_auftrag = auftrag;
    m_meldelampe = meldelampe;
    m_befehlNachWB = befehlNachWB;
    m_befehlVonWB = befehlVonWB;
    m_nextCheck = 0;
    m_releaseTime = UINT32_MAX;
    m_hasErlaubnis = true;
    m_AnfangSet = true;
    m_EndSet = true;

    pinMode(m_anfang, OUTPUT);
    pinMode(m_erlaubnis, OUTPUT);
    pinMode(m_end, OUTPUT);
    pinMode(m_meldelampe, OUTPUT);
    pinMode(m_auftrag, INPUT);

    m_auftragsPin.init(m_auftrag, 50);
    m_befehlVonPin.init(m_befehlVonWB, 50);
    m_befehlNachPin.init(m_befehlNachWB, 50);

    digitalWrite(m_anfang, true);
    digitalWrite(m_erlaubnis, true);
    digitalWrite(m_end, true);
    digitalWrite(m_meldelampe, true);
}

/**
 * Meldet zurück, ob Calw die Erlaubnis für den
 * Streckenblock besitzt (entspricht Erlaubnisfeld
 * ist weiß).
 **/
bool Streckenblock::hasErlaubnis() {
    //Serial.print("Check has Erlaubnis"); Serial.println(m_hasErlaubnis);
    return m_hasErlaubnis;
}

/**
 * Meldet zurück, ob die Strecke frei ist.
 * m_Anfang/EndSet ist active low!
 **/
bool Streckenblock::isFree() {
    return m_AnfangSet && m_EndSet;
}

/**
 * Gibt den Streckenblock an den Fahrdienstleiter
 * Wildberg ab. Der Besitz kann nur geänder werden,
 * wenn die Strecke gerade nicht befahren wird.
 **/
void Streckenblock::release() {
    if (m_AnfangSet && m_EndSet) {
        m_releaseTime = millis() + WAIT_RELEASE;
        Serial.println("Has Erlaubnis false: "); Serial.println(m_hasErlaubnis);
    } else {
        Serial.println("Strecke noch belegt, Aktion abgebrochen.");
    }
}

/**
 * Bekommt den Streckenblock vom Fahrdienstleiter
 * Wildberg. Der Besitz kann nur geändert werden, wenn
 * die Strecke gerade nicht befahren wird.
 **/
void Streckenblock::aquire() {
    Serial.print("Aquire a: "); Serial.print(m_AnfangSet); Serial.print(", e: "); Serial.println(m_EndSet);
    if (m_AnfangSet && m_EndSet) {
        m_hasErlaubnis = true;
        digitalWrite(m_erlaubnis, m_hasErlaubnis);
        Serial.println("Has Erlaubnis true"); Serial.println(m_hasErlaubnis);
    } else {
        Serial.println("Strecke noch belegt, Aktion abgebrochen.");
    }
}

/**
 * Setzt das Anfangsfeld auf rot. Die Strecke ist
 * nun durch einen Zug aus Calw belegt.
 **/
void Streckenblock::setAnfang() {
    Serial.println("set anfang");
    m_AnfangSet = false;
    digitalWrite(m_anfang, m_AnfangSet);
}

/**
 * Setzt das Endfeld auf rot. Die Strecke ist nun
 * durch einen Zug aus Wildberg belegt.
 **/
void Streckenblock::setEnd() {
    Serial.println("set end");
    m_EndSet = false;
    digitalWrite(m_end, m_EndSet);
}

/**
 * Setzt das Anfangsfeld auf weiß. Die Strecke
 * ist nun frei.
 **/
void Streckenblock::resetAnfang() {
    Serial.println("reset anfang");
    m_AnfangSet = true;
    digitalWrite(m_anfang, m_AnfangSet);
}

/**
 * Setzt das Endfeld auf weiß. Die Strecke ist
 * nun frei.
 **/
void Streckenblock::resetEnd() {
    Serial.println("reset end");
    m_EndSet = true;
    digitalWrite(m_end, m_EndSet);
}

/**
 * Weiterleitung aus der Arduino loop Funktion, wird
 * für die asynchronen Vorgänge benötigt und muss
 * regelmäßig aufgerufen werden.
 **/
void Streckenblock::tick(unsigned long now) {
  m_auftragsPin.tick(now);
  if (now > m_nextCheck) {
    //Serial.print("Streckenblock "); Serial.print(m_auftragsPin.read()); Serial.print(", Port "); Serial.println(digitalRead(m_auftrag));
    m_nextCheck = now + 300;
    if (m_auftragsPin.triggered() && m_AnfangSet && m_EndSet) {
        release();
        digitalWrite(m_meldelampe, false);
    }

    if (now > m_releaseTime) {
        digitalWrite(m_erlaubnis, false);
        digitalWrite(m_meldelampe, true);
        m_releaseTime = UINT32_MAX;
        m_hasErlaubnis = false;
        Serial.print("Has Erlaubnis false: "); Serial.println(m_hasErlaubnis);
    }
  }
}