#include "globals.h"
#include "nes_type.h"
#include "joypad_type.h"
#define JOYPAD_NUM_BUTTONS 8
Joypad joypad_init(int joypadNumber);
void joypad_destroy(Joypad joypad);
Byte joypad_readByte(NES nes, Joypad joypad);
void joypad_resetCurrentButton(Joypad joypad);