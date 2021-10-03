#ifndef _PORTDEFINITIONS__
#define _PORTDEFINITIONS__

#include <Arduino.h>

// Zustimmungsempfang
#define IN_A1 3
#define IN_A2 4
#define IN_A4 5
#define IN_B2 6
#define IN_B4 7
#define IN_F 8

extern uint8_t zustimmungsports[];

// Zustand Streckenblock Liebenzell
#define IN_LB_ANF 9
#define IN_LB_ERL 10
#define IN_LB_END 11

// Zustand Streckenblock Althengstett
#define IN_AH_ANF 34
#define IN_AH_ERL 35
#define IN_AH_END 36

extern uint8_t streckenblockports[];


// Wechselstromerzeugung
#define OUT_BLINK 12

// Block Port
#define OUT_ZUST_A1 30
#define OUT_ZUST_A2 22
#define OUT_ZUST_A4 23
#define OUT_ZUST_B2 24
#define OUT_ZUST_B4 25
#define OUT_ZUST_F 26

#define OUT_STRB_LB_ANF 27
#define OUT_STRB_LB_ERL 28
#define OUT_STRB_LB_END 29

#define OUT_STRB_AH_ANF 31
#define OUT_STRB_AH_ERL 33
#define OUT_STRB_AH_END 32

extern uint8_t magnetports[];

#endif
