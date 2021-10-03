#ifndef _INPUTSTATE__
#define _INPUTSTATE__
#include <Arduino.h>

class InputState {
  public:
    InputState();
    void tick(unsigned long now);
    void print();

    bool ZustimmungA1;
    bool ZustimmungA2;
    bool ZustimmungA4;
    bool ZustimmungB2;
    bool ZustimmungB4;
    bool ZustimmungF;

    bool ZustimmungAllA;
    bool ZustimmungAllB;

    bool Sb_AnfLB;
    bool Sb_ErlLB;
    bool Sb_EndLB;

    bool Sb_AnfAH;
    bool Sb_ErlAH;
    bool Sb_EndAH;
    
  private:
    void readAllInputs();
    void calcCompounds();
};
#endif

