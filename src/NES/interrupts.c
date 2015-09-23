#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "globals.h"
#include "interrupts.h"
struct interrupts {
  Boolean IRQ;
  Boolean NMI;
  Boolean RESET;
};
Interrupts interrupts_init(void) {
  Interrupts interrupts = (Interrupts) malloc(sizeof(struct interrupts));
  assert(interrupts != NULL);
  interrupts->IRQ = FALSE;
  interrupts->NMI = FALSE;
  interrupts->RESET = FALSE;
  return interrupts;
}
void interrupts_destroy(Interrupts interrupts) {
  assert(interrupts != NULL);
  free(interrupts);
}
Boolean interrupts_getIRQ(Interrupts interrupts) {
  assert(interrupts != NULL);
  return interrupts->IRQ;
}
Boolean interrupts_getNMI(Interrupts interrupts) {
  assert(interrupts != NULL);
  return interrupts->NMI;
}
Boolean interrupts_getRESET(Interrupts interrupts) {
  assert(interrupts != NULL);
  return interrupts->RESET;
}
void interrupts_setIRQ(Interrupts interrupts, Boolean IRQ) {
  assert(interrupts != NULL);
  interrupts->IRQ = IRQ;
}
void interrupts_setNMI(Interrupts interrupts, Boolean NMI) {
  assert(interrupts != NULL);
  interrupts->NMI = NMI;
}
void interrupts_setRESET(Interrupts interrupts, Boolean RESET) {
  assert(interrupts != NULL);
  interrupts->RESET = RESET;
}