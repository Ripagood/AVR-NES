#include "interrupts_type.h"
#include "globals.h"
Interrupts interrupts_init(void);
void interrupts_destroy(Interrupts interrupts);
Boolean interrupts_getIRQ(Interrupts interrupts);
Boolean interrupts_getNMI(Interrupts interrupts);
Boolean interrupts_getRESET(Interrupts interrupts);
void interrupts_setIRQ(Interrupts interrupts, Boolean IRQ);
void interrupts_setNMI(Interrupts interrupts, Boolean NMI);
void interrupts_setRESET(Interrupts interrupts, Boolean RESET);