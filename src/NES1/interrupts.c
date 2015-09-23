#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "globals.h"
#include "interrupts.h"
struct interrupts {
  Bool IRQ;
  Bool NMI;
  Bool RESET;
};
Interrupts interrupts_init(void) {
  Interrupts interrupts = (Interrupts) malloc(sizeof(struct interrupts));
  //assert(interrupts != NULL);
  interrupts->IRQ = FALSE;
  interrupts->NMI = FALSE;
  interrupts->RESET = FALSE;
  return interrupts;
}
void interrupts_destroy(Interrupts interrupts) {
  //assert(interrupts != NULL);
  free(interrupts);
}
Bool interrupts_getIRQ(Interrupts interrupts) {
  //assert(interrupts != NULL);
  return interrupts->IRQ;
}
Bool interrupts_getNMI(Interrupts interrupts) {
  //assert(interrupts != NULL);
  return interrupts->NMI;
}
Bool interrupts_getRESET(Interrupts interrupts) {
  //assert(interrupts != NULL);
  return interrupts->RESET;
}
void interrupts_setIRQ(Interrupts interrupts, Bool IRQ) {
  //assert(interrupts != NULL);
  interrupts->IRQ = IRQ;
}
void interrupts_setNMI(Interrupts interrupts, Bool NMI) {
  //assert(interrupts != NULL);
  interrupts->NMI = NMI;
}
void interrupts_setRESET(Interrupts interrupts, Bool RESET) {
  //assert(interrupts != NULL);
  interrupts->RESET = RESET;
}