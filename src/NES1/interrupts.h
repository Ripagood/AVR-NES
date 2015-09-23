#include "interrupts_type.h"
#include "globals.h"
Interrupts interrupts_init(void);
void interrupts_destroy(Interrupts interrupts);
Bool interrupts_getIRQ(Interrupts interrupts);
Bool interrupts_getNMI(Interrupts interrupts);
Bool interrupts_getRESET(Interrupts interrupts);
void interrupts_setIRQ(Interrupts interrupts, Bool IRQ);
void interrupts_setNMI(Interrupts interrupts, Bool NMI);
void interrupts_setRESET(Interrupts interrupts, Bool RESET);