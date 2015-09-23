#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "globals.h"
#include "gui.h"
#include <SDL/SDL.h>
#include <string.h>
#include "joypad.h"
#define GUI_MAX_JOYPADS 4
#define GUI_IMAGE_SCALE 2
struct gui {
  SDL_Surface *screen;
  int width;
  int height;
  int currentFrame;
  Boolean buttonState[GUI_MAX_JOYPADS][JOYPAD_NUM_BUTTONS];
  Boolean receivedTerminationRequest;
};
int gui_getWidth(GUI gui) {
  assert(gui != NULL);
  return gui->width;
}
int gui_getHeight(GUI gui) {
  assert(gui != NULL);
  return gui->height;
}
void gui_drawPixel(GUI gui, Byte x, Byte y, Byte red, Byte green, Byte blue) {
  assert(gui != NULL);
  assert(gui->screen != NULL);
  assert(x < gui->width);    // Byte type is unsigned
  assert(y < gui->height);   // Byte type is unsigned
  Uint32 color = SDL_MapRGB(gui->screen->format, red, green, blue);
  assert(gui->screen->format->BytesPerPixel == 4);
  Uint32 *bufp;
  // hack, hardcoded scale of 2
  Word actualX = GUI_IMAGE_SCALE * x;
  Word actualY = GUI_IMAGE_SCALE * y;
  bufp = (Uint32 *)gui->screen->pixels + ((actualY)*gui->screen->pitch/4) + (actualX);
  *bufp = color;
  bufp = (Uint32 *)gui->screen->pixels + ((actualY)*gui->screen->pitch/4) + (actualX) +
1;
  *bufp = color;
  bufp = (Uint32 *)gui->screen->pixels + (((actualY)+1)*gui->screen->pitch/4) +
(actualX);
  *bufp = color;
  bufp = (Uint32 *)gui->screen->pixels + (((actualY)+1)*gui->screen->pitch/4) +
(actualX) + 1;
  *bufp = color;
}
void gui_refresh(GUI gui) {
  assert(gui != NULL);
  assert(gui->screen != NULL);
  if (gui->currentFrame % 200 == 0) {
    char filename[100];
    sprintf(filename, "capture%03d.bmp", gui->currentFrame);
    SDL_SaveBMP(gui->screen,filename);
  }
  gui->currentFrame++;
  SDL_Flip(gui->screen);
        //SDL_Delay(1000/60);
}
static void gui_resetButtonState(GUI gui) {
  assert(gui != NULL);
  int joypad;
  for (joypad=0; joypad < GUI_MAX_JOYPADS; joypad++) {
    int button;
    for (button=0; button < JOYPAD_NUM_BUTTONS; button++) {
      gui->buttonState[joypad][button] = 0;
    }
  }
}
GUI gui_init(int width, int height) {
  GUI gui = (GUI) malloc(sizeof(struct gui));
  assert(gui != NULL);
                  //SDL_INIT_AUDIO
  assert(SDL_Init(SDL_INIT_VIDEO) >= 0);
  SDL_WM_SetCaption( "1337NES", 0 );
  atexit(SDL_Quit);
  gui->screen=SDL_SetVideoMode(width * GUI_IMAGE_SCALE,height *
GUI_IMAGE_SCALE,32,SDL_HWSURFACE|SDL_DOUBLEBUF); //|SDL_FULLSCREEN);
  assert(gui->screen != NULL);
  gui->width = width;
  gui->height = height;
  gui->currentFrame = 0;
  gui->receivedTerminationRequest = FALSE;
  gui_resetButtonState(gui);
  return gui;
}
Boolean gui_receivedTerminationRequest(GUI gui) {
  assert(gui != NULL);
  return gui->receivedTerminationRequest;
}
void gui_destroy(GUI gui) {
  assert(gui != NULL);
  free(gui);
}
void gui_queryInput(GUI gui) {
  assert(gui != NULL);
  //gui_resetButtonState(gui);
  SDL_Event event;
  while ( SDL_PollEvent(&event) ) {
if ( event.type == SDL_QUIT) {
  gui->receivedTerminationRequest = TRUE;
} else if ( event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
  switch(event.key.keysym.sym) {
    case GUI_KEY_JOYPAD_0_BUTTON_A:
      gui->buttonState[0][JOYPAD_BUTTON_A] = (event.type == SDL_KEYDOWN);
      break;
    case GUI_KEY_JOYPAD_0_BUTTON_B:
      gui->buttonState[0][JOYPAD_BUTTON_B] = (event.type == SDL_KEYDOWN);
      break;
    case GUI_KEY_JOYPAD_0_BUTTON_START:
      gui->buttonState[0][JOYPAD_BUTTON_START] = (event.type == SDL_KEYDOWN);
      break;
    case GUI_KEY_JOYPAD_0_BUTTON_SELECT:
      gui->buttonState[0][JOYPAD_BUTTON_SELECT] = (event.type == SDL_KEYDOWN);
      break;
    case GUI_KEY_JOYPAD_0_BUTTON_UP:
      gui->buttonState[0][JOYPAD_BUTTON_UP] = (event.type == SDL_KEYDOWN);
      break;
    case GUI_KEY_JOYPAD_0_BUTTON_DOWN:
      gui->buttonState[0][JOYPAD_BUTTON_DOWN] = (event.type == SDL_KEYDOWN);
      break;
    case GUI_KEY_JOYPAD_0_BUTTON_LEFT:
      gui->buttonState[0][JOYPAD_BUTTON_LEFT] = (event.type == SDL_KEYDOWN);
      break;
    case GUI_KEY_JOYPAD_0_BUTTON_RIGHT:
      gui->buttonState[0][JOYPAD_BUTTON_RIGHT] = (event.type == SDL_KEYDOWN);
      break;
    case GUI_KEY_JOYPAD_1_BUTTON_A:
      gui->buttonState[1][JOYPAD_BUTTON_A] = (event.type == SDL_KEYDOWN);
      break;
    case GUI_KEY_JOYPAD_1_BUTTON_B:
      gui->buttonState[1][JOYPAD_BUTTON_B] = (event.type == SDL_KEYDOWN);
      break;
    case GUI_KEY_JOYPAD_1_BUTTON_START:
      gui->buttonState[1][JOYPAD_BUTTON_START] = (event.type == SDL_KEYDOWN);
      break;
    case GUI_KEY_JOYPAD_1_BUTTON_SELECT:
      gui->buttonState[1][JOYPAD_BUTTON_SELECT] = (event.type == SDL_KEYDOWN);
      break;
    case GUI_KEY_JOYPAD_1_BUTTON_UP:
      gui->buttonState[1][JOYPAD_BUTTON_UP] = (event.type == SDL_KEYDOWN);
      break;
    case GUI_KEY_JOYPAD_1_BUTTON_DOWN:
      gui->buttonState[1][JOYPAD_BUTTON_DOWN] = (event.type == SDL_KEYDOWN);
      break;
    case GUI_KEY_JOYPAD_1_BUTTON_LEFT:
      gui->buttonState[1][JOYPAD_BUTTON_LEFT] = (event.type == SDL_KEYDOWN);
      break;
    case GUI_KEY_JOYPAD_1_BUTTON_RIGHT:
          gui->buttonState[1][JOYPAD_BUTTON_RIGHT] = (event.type == SDL_KEYDOWN);
          break;
        case SDLK_ESCAPE:
          gui->receivedTerminationRequest = TRUE;
          break;
        default:
          printf("Unmapped key press: %d\n", event.key.keysym.sym);
          break;
      }
    }
  }
}
Boolean gui_isButtonPressed(GUI gui, int joypad, Button button) {
  assert(gui != NULL);
  assert(joypad >= 0);
  assert(joypad <= GUI_MAX_JOYPADS);
  assert(button >= 0);
  assert(button <= JOYPAD_NUM_BUTTONS);
  return gui->buttonState[joypad][button];
}