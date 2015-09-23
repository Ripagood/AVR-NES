#include "gui_type.h"
#include "globals.h"
#include "joypad_type.h"
#define GUI_KEY_JOYPAD_0_BUTTON_A       SDLK_q
#define GUI_KEY_JOYPAD_0_BUTTON_B       SDLK_e
#define GUI_KEY_JOYPAD_0_BUTTON_SELECT  SDLK_c
#define GUI_KEY_JOYPAD_0_BUTTON_START   SDLK_z
#define GUI_KEY_JOYPAD_0_BUTTON_UP      SDLK_w
#define GUI_KEY_JOYPAD_0_BUTTON_DOWN    SDLK_s
#define GUI_KEY_JOYPAD_0_BUTTON_LEFT    SDLK_a
#define GUI_KEY_JOYPAD_0_BUTTON_RIGHT   SDLK_d
// must turn NUM-LOCK on
#define GUI_KEY_JOYPAD_1_BUTTON_A       SDLK_KP7
#define GUI_KEY_JOYPAD_1_BUTTON_B       SDLK_KP9
#define GUI_KEY_JOYPAD_1_BUTTON_SELECT  SDLK_KP3
#define GUI_KEY_JOYPAD_1_BUTTON_START   SDLK_KP1
#define GUI_KEY_JOYPAD_1_BUTTON_UP      SDLK_KP8
#define GUI_KEY_JOYPAD_1_BUTTON_DOWN    SDLK_KP5
#define GUI_KEY_JOYPAD_1_BUTTON_LEFT    SDLK_KP4
#define GUI_KEY_JOYPAD_1_BUTTON_RIGHT   SDLK_KP6
void gui_destroy(GUI gui);
int gui_getWidth(GUI gui);
int gui_getHeight(GUI gui);
void gui_drawPixel(GUI gui, Byte x, Byte y, Byte red, Byte green, Byte blue);
void gui_refresh(GUI gui);
GUI gui_init(int width, int height);
void gui_queryInput(GUI gui);
Boolean gui_isButtonPressed(GUI gui, int joypad, Button button);
Boolean gui_receivedTerminationRequest(GUI gui);