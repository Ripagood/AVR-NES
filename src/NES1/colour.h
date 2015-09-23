#include "colour_type.h"
#include "globals.h"
void colour_destroy(Colour colour);
void colour_setRed(Colour colour, Byte red);
void colour_setGreen(Colour colour, Byte green);
void colour_setBlue(Colour colour, Byte blue);
Byte colour_getRed(Colour colour);
Byte colour_getGreen(Colour colour);
Byte colour_getBlue(Colour colour);
Colour colour_init(Byte red, Byte green, Byte blue);
void colour_setColour(Colour colour, Byte red, Byte green, Byte blue);