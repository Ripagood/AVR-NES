#include "globals.h"
#include "nes_type.h"
#include "joypad_type.h"
#define JOYPAD_NUM_BUTTONS 8
Joypad joypad_init(int joypadNumber);
void joypad_destroy(Joypad joypad);
Byte joypad_readByte(NES nes, Joypad joypad);
void joypad_resetCurrentButton(Joypad joypad);
joypad_type.h
#ifndef JOYPAD_TYPE_H
  #define JOYPAD_TYPE_H
  typedef struct joypad *Joypad;
  typedef enum {
    JOYPAD_BUTTON_A = 0,
    JOYPAD_BUTTON_B = 1,
    JOYPAD_BUTTON_SELECT = 2,
    JOYPAD_BUTTON_START = 3,
    JOYPAD_BUTTON_UP = 4,
    JOYPAD_BUTTON_DOWN = 5,
    JOYPAD_BUTTON_LEFT = 6,
    JOYPAD_BUTTON_RIGHT = 7
  } Button;
#endif