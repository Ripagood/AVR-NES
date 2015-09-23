#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "nes.h"
#define DEFAULT_RESOLUTION_WIDTH 256
#define DEFAULT_RESOLUTION_HEIGHT 240
int main(int argc, char *argv[]) {
  assert(argc >= 2);
  char *filename = NULL;
  int width = DEFAULT_RESOLUTION_WIDTH;
  int height = DEFAULT_RESOLUTION_HEIGHT;
  if (argc == 2) {
    filename = argv[1];
  } else if (argc == 4) {
    width = atoi(argv[1]);
    height = atoi(argv[2]);
    filename = argv[3];
  }
  NES nes = nes_init(filename, width, height);
  nes_run(nes);
  nes_destroy(nes);
  return 0;
}