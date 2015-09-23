#ifndef COLOUR_TYPE_H
  #define COLOUR_TYPE_H
  typedef struct colour *Colour;
  // we break abstraction here so that ppu.c can declare an array of 'struct colour' for the global system palette
  // maybe find a better way to do this later
  struct colour {
    Byte red;
    Byte green;
    Byte blue;
  };
#endif