#ifndef __STRECKENBLOCK_
#define __STRECKENBLOCK_
#include <Arduino.h>
#include "DebouncedPin.hpp"


class Streckenblock {
public:
  Streckenblock(uint8_t anfangsfeld, uint8_t erlaubnisfeld, uint8_t endfeld, uint8_t auftrag, uint8_t meldelampe, uint8_t befehlVonWB, uint8_t befehlNachWB);
  void tick(unsigned long now);
  void release();
  void aquire();
  void setAnfang();
  void setEnd();
  void resetAnfang();
  void resetEnd();
  bool hasErlaubnis();
  bool isFree();

private:
  bool m_hasErlaubnis;
  bool m_AnfangSet;
  bool m_EndSet;
  unsigned long m_nextCheck;
  unsigned long m_releaseTime;

  uint8_t m_anfang, m_erlaubnis, m_end;
  uint8_t m_meldelampe, m_auftrag;
  uint8_t m_befehlVonWB, m_befehlNachWB;
  DebouncedPin m_auftragsPin;
  DebouncedPin m_befehlVonPin;
  DebouncedPin m_befehlNachPin;

};

#endif