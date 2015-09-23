#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "joypad.h"
#include "gui.h"
#include "nes.h"
struct joypad {
  int joypadNumber;
  Button currentButton;
};
Joypad joypad_init(int joypadNumber) {
  Joypad joypad = (Joypad) malloc(sizeof(struct joypad));
  //assert(joypad != NULL);
  joypad->joypadNumber = joypadNumber;
  joypad->currentButton = 0;
  return joypad;
}
void joypad_resetCurrentButton(Joypad joypad) {
  //assert(joypad != NULL);
  joypad->currentButton = 0;
}
Byte joypad_readByte(NES nes, Joypad joypad) {
  //assert(nes != NULL);
 // assert(joypad != NULL);
  GUI gui = nes_getGUI(nes);
 // assert(gui != NULL);
  Byte data = 0;
  if (joypad->currentButton < JOYPAD_NUM_BUTTONS) {
    if (gui_isButtonPressed(gui, joypad->joypadNumber, joypad->currentButton) == TRUE) {
      data = 1;
    }
  } else if (joypad->currentButton < (JOYPAD_NUM_BUTTONS * 2)) {
    data = 0; // player 0/2 1/3
  } else {
    data = 1;
  }
  joypad->currentButton++;
  return data;
}
void joypad_destroy(Joypad joypad) {
  //assert(joypad != NULL);
  free(joypad);
}