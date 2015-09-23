#include "apu_type.h"
#include "nes_type.h"
APU apu_init(void);
void apu_step(NES nes);
void apu_destroy(APU apu);