#include "nes_type.h"
#include "globals.h"
#include "memory_type.h"
#include "cpu_type.h"
#include "ppu_type.h"
#include "cartridge_type.h"
#include "interrupts_type.h"
#include "apu_type.h"
#include "gui.h"
#define NES_NUM_JOYPADS 2
NES nes_init(char *filename, int width, int height);
void nes_destroy(NES nes);
Memory nes_getCPUMemory(NES nes);
Memory nes_getPPUMemory(NES nes);
Memory nes_getObjectAttributeMemory(NES nes);
CPU nes_getCPU(NES nes);
PPU nes_getPPU(NES nes);
APU nes_getAPU(NES nes);
Cartridge nes_getCartridge(NES nes);
Interrupts nes_getInterrupts(NES nes);
void nes_run(NES nes);
Byte nes_readCPUMemory(NES nes, Address address);
void nes_writeCPUMemory(NES nes, Address address, Byte data);
Byte nes_readPPUMemory(NES nes, Address address);
void nes_writePPUMemory(NES nes, Address address, Byte data);
Byte nes_readObjectAttributeMemory(NES nes, Address address);
void nes_writeObjectAttributeMemory(NES nes, Address address, Byte data);
void nes_cpuCycled(NES nes);
void nes_generateNMI(NES nes);
GUI nes_getGUI(NES nes);
Byte nes_readJoypad(NES nes, int joypadNumber);
void nes_writeJoypad(NES nes, int joypadNumber, Byte data);