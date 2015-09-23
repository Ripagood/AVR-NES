#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "globals.h"
#include "colour.h"
Colour colour_init(Byte red, Byte green, Byte blue) {
  Colour colour = (Colour) malloc(sizeof(struct colour));
  //assert(colour != NULL);
  colour_setColour(colour, red, green, blue);
  return colour;
}
void colour_destroy(Colour colour) {
  //assert(colour != NULL);
  free(colour);
}
inline void colour_setRed(Colour colour, Byte red) {
  //assert(colour != NULL);
  colour->red = red;
}
inline void colour_setGreen(Colour colour, Byte green) {
  //assert(colour != NULL);
  colour->green = green;
}
inline void colour_setBlue(Colour colour, Byte blue) {
  //assert(colour != NULL);
  colour->blue = blue;
}
inline Byte colour_getRed(Colour colour) {
 // assert(colour != NULL);
  return colour->red;
}
inline Byte colour_getGreen(Colour colour) {
 // assert(colour != NULL);
  return colour->green;
}
inline Byte colour_getBlue(Colour colour) {
 // assert(colour != NULL);
  return colour->blue;
}
inline void colour_setColour(Colour colour, Byte red, Byte green, Byte blue) {
  //assert(colour != NULL);
  colour->red = red;
  colour->green = green;
  colour->blue = blue;
}