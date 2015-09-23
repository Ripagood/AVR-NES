#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "globals.h"
#include "nes.h"
#include "memory.h"
#include "cartridge.h"
#include "cpu.h"
#include "ppu.h"
#include "interrupts.h"
#include "apu.h"
#include "cpu.h"
#include "ppu.h"
#include "cpuMemory.h"
#include "ppuMemory.h"
#include "objectAttributeMemory.h"
#include "mmu.h"
#include "gui.h"
#include "joypad.h"
#include "usart.h"
struct nes {
  Memory cpuMemory;
  Memory ppuMemory;
  Memory objectAttributeMemory;
  CPU cpu1;
  PPU ppu;
  APU apu;
  Cartridge cartridge;
  Interrupts interrupts;
  MMU mmu;
  Bool isRunning;
  GUI gui;
  Joypad joypads[NES_NUM_JOYPADS];
};
static void nes_init_interrupts(NES nes) {
  //assert(nes != NULL);
  //assert(nes->interrupts == NULL);
  nes->interrupts = interrupts_init();
  //assert(nes->interrupts != NULL);
  interrupts_setRESET(nes->interrupts, TRUE);
}

static void nes_init_cpu(NES nes) {
  //assert(nes != NULL);
  //assert(nes->cpu1 == NULL);
  nes->cpu1 = cpu_init();
  //assert(nes->cpu1 != NULL);
}
static void nes_init_ppu(NES nes) {
  //assert(nes != NULL);
  //assert(nes->ppu == NULL);
  nes->ppu = ppu_init();
  //assert(nes->ppu != NULL);
}
static void nes_init_apu(NES nes) {
  //assert(nes != NULL);
  //assert(nes->apu == NULL);
  nes->apu = apu_init();
  //assert(nes->apu != NULL);
}
static void nes_init_cartridge(NES nes, int filename) {
  //assert(nes != NULL);
  //assert(nes->cartridge == NULL);
  nes->cartridge = cartridge_init(filename);
  //assert(nes->cartridge != NULL);
}
static void nes_init_ppuMemory(NES nes) {
  ////assert(nes != NULL);
  ////assert(nes->ppuMemory == NULL);
  nes->ppuMemory = ppuMemory_init();
  ////assert(nes->ppuMemory != NULL);
}
static void nes_init_cpuMemory(NES nes) {
 //assert(nes != NULL);
 //assert(nes->cpuMemory == NULL);
  nes->cpuMemory = cpuMemory_init();
 //assert(nes->cpuMemory != NULL);
}
static void nes_init_mmu(NES nes) {
  //assert(nes != NULL);
  //assert(nes->mmu == NULL);
  nes->mmu = mmu_init(nes);
  //assert(nes->mmu != NULL);
}
static void nes_init_objectAttributeMemory(NES nes) {
  //assert(nes != NULL);
  //assert(nes->objectAttributeMemory == NULL);
  nes->objectAttributeMemory = objectAttributeMemory_init();
  //assert(nes->objectAttributeMemory != NULL);
}
Memory nes_getObjectAttributeMemory(NES nes) {
  //assert(nes != NULL);
  //assert(nes->objectAttributeMemory != NULL);
  return nes->objectAttributeMemory;
}
void nes_generateNMI(NES nes) {
  //assert(nes != NULL);
  //usart_write_line(&AVR32_USART0,"generateNMI\n");
  interrupts_setNMI(nes->interrupts, TRUE);
}
static void nes_init_gui(NES nes, int width, int height) {
  //assert(nes != NULL);
  //assert(nes->gui == NULL);
  nes->gui = gui_init(width, height);
  //assert(nes->gui != NULL);
}
static void nes_init_joypads(NES nes) {
  //assert(nes != NULL);
  int i;
  for (i=0; i < NES_NUM_JOYPADS; i++) {
    nes->joypads[i] = joypad_init(i);
    //assert(nes->joypads[i] != NULL);
  }
}
NES nes_init(int filename, int width, int height) {
  NES nes = (NES) malloc(sizeof(struct nes));
  //assert(nes != NULL);
  nes->cartridge = NULL;
  nes->ppuMemory = NULL;
  nes->cpuMemory = NULL;
  nes->ppu = NULL;
  nes->cpu1 = NULL;
  nes->apu = NULL;
  nes->interrupts = NULL;
  nes->mmu = NULL;
  nes->objectAttributeMemory = NULL;
  nes->gui = NULL;
  usart_write_line(&AVR32_USART0,"Initial cpuMemory1\n");
  nes_init_cpuMemory(nes);
   usart_write_line(&AVR32_USART0,"Initial cpuMemory2\n");
  nes_init_ppuMemory(nes);
   usart_write_line(&AVR32_USART0,"Initial cpuMemory3\n");
  nes_init_cartridge(nes, filename);
  //usart_write_line(&AVR32_USART0,"Initial cpuMemory4\n");
  nes_init_apu(nes);
  nes_init_cpu(nes);
  nes_init_ppu(nes);
  nes_init_interrupts(nes);
  nes_init_objectAttributeMemory(nes);
  nes_init_gui(nes, width, height);
  nes_init_joypads(nes);
  // MMU should be the last thing init, since it can create callbacks all over the place
  nes_init_mmu(nes);
  ////usart_write_line(&AVR32_USART0,"\n");
  //usart_write_line(&AVR32_USART0,"Initial cpuMemory00\n");
 // memory_print(nes, nes->cpuMemory);
  //usart_write_line(&AVR32_USART0,"\n");
  //usart_write_line(&AVR32_USART0,"Initial ppuMemory\n");
 // memory_print(nes, nes->ppuMemory);
  //usart_write_line(&AVR32_USART0,"\n");
  usart_write_line(&AVR32_USART0,"Initial objectAttributeMemory\n");
  //memory_print(nes, nes->objectAttributeMemory);
  nes->isRunning = TRUE;
  return nes;
}
Byte nes_readJoypad(NES nes, int joypadNumber) {
  //assert(nes != NULL);
  //assert(joypadNumber < NES_NUM_JOYPADS);
  return joypad_readByte(nes, nes->joypads[joypadNumber]);
}
void nes_writeJoypad(NES nes, int joypadNumber, Byte data) {
  //assert(nes != NULL);
  //assert(joypadNumber < NES_NUM_JOYPADS);
  if (joypadNumber == 0) {
    if ((data & MASK_BIT0) == MASK_BIT0) {
      gui_queryInput(nes->gui);
    } else {
      int i;
      for (i=0; i < NES_NUM_JOYPADS; i++) {
        joypad_resetCurrentButton(nes->joypads[i]);
      }
    }
  }
}
void nes_destroy(NES nes) {
  //assert(nes != NULL);
  memory_destroy(nes->cpuMemory);
  memory_destroy(nes->ppuMemory);
  memory_destroy(nes->objectAttributeMemory);
  cpu_destroy(nes->cpu1);
  ppu_destroy(nes->ppu);
  apu_destroy(nes->apu);
  cartridge_destroy(nes->cartridge);
  interrupts_destroy(nes->interrupts);
  mmu_destroy(nes->mmu);
  gui_destroy(nes->gui);
  int i;
  for (i=0; i < NES_NUM_JOYPADS; i++) {
    joypad_destroy(nes->joypads[i]);
  }
  free(nes);
}
static void nes_checkKeyboard(NES nes) {
  //assert(nes != NULL);
  nes->isRunning = !gui_receivedTerminationRequest(nes->gui);
}
void nes_run(NES nes) {
  ////assert(nes != NULL);
  while(nes->isRunning == TRUE) {
   // usart_write_line(&AVR32_USART0,"nes_run\n");
    if (interrupts_getRESET(nes->interrupts) == TRUE) {
     // usart_write_line(&AVR32_USART0,"RESET\n");
      interrupts_setRESET(nes->interrupts, FALSE);
      cpu_handleInterrupt(nes, CPU_RESET_VECTOR_LOWER_ADDRESS, FALSE);
    } else if (interrupts_getNMI(nes->interrupts) == TRUE) {
      //usart_write_line(&AVR32_USART0,"NMI\n");
      interrupts_setNMI(nes->interrupts, FALSE);
      cpu_handleInterrupt(nes, CPU_NMI_VECTOR_LOWER_ADDRESS, FALSE);
    } else if ((cpu_getInterruptDisable(nes->cpu1) == FALSE) && (interrupts_getIRQ(nes->interrupts) == TRUE)) {
      //usart_write_line(&AVR32_USART0,"IRQ\n");
      interrupts_setIRQ(nes->interrupts, FALSE);
      cpu_handleInterrupt(nes, CPU_IRQ_VECTOR_LOWER_ADDRESS, FALSE);
    }
	//usart_write_line(&AVR32_USART0,"step");
    cpu_step(nes);
    //nes_checkKeyboard(nes);
  }
}
GUI nes_getGUI(NES nes) {
  //assert(nes != NULL);
  //assert(nes->gui != NULL);
  return nes->gui;
}
Memory nes_getCPUMemory(NES nes) {
  //assert(nes != NULL);
  return nes->cpuMemory;
}
Memory nes_getPPUMemory(NES nes) {
  //assert(nes != NULL);
  return nes->ppuMemory;
}
CPU nes_getCPU(NES nes) {
  //assert(nes != NULL);
  return nes->cpu1;
}
PPU nes_getPPU(NES nes) {
  //assert(nes != NULL);
  return nes->ppu;
}
Cartridge nes_getCartridge(NES nes) {
 // //assert(nes != NULL);
  return nes->cartridge;
}
Interrupts nes_getInterrupts(NES nes) {
  //assert(nes != NULL);
  return nes->interrupts;
}
Byte nes_readCPUMemory(NES nes1, Address address) {
  ////assert(nes1 != NULL);
  ////assert(nes1->cpuMemory != NULL);
   //usart_write_line(&AVR32_USART0,"readCPUmem\n");
  Byte data = memory_read_callback(nes1, nes1->cpuMemory, address);
  //usart_write_line(&AVR32_USART0,"readCPUmem1\n");
  nes_cpuCycled(nes1);
   
  return data;
}
void nes_writeCPUMemory(NES nes, Address address, Byte data) {
  //assert(nes != NULL);
  //assert(nes->cpuMemory != NULL);
  memory_write_callback(nes, nes->cpuMemory, address, data);
  //nes_cpuCycled(nes);
}
Byte nes_readObjectAttributeMemory(NES nes, Address address) {
  //assert(nes != NULL);
  //assert(nes->objectAttributeMemory != NULL);
  return memory_read_callback(nes, nes->objectAttributeMemory, address);
}
void nes_writeObjectAttributeMemory(NES nes, Address address, Byte data) {
  //assert(nes != NULL);
  //assert(nes->objectAttributeMemory != NULL);
  memory_write_callback(nes, nes->objectAttributeMemory, address, data);
}
// the PPU has 64kb of address space but can only access 16kb of it
// we handle the "big picture" ppu mirrors here (the four 16kb mirrors)
// and we handle the smaller mirrors (mirrors within mirrors) with callbacks
static Address nes_getLowPPUAddress(Address address) {
  while(address > PPU_LAST_REAL_ADDRESS) {
    address -= PPU_NUM_REAL_ADDRESSES;
  }
  //assert(address <= PPU_LAST_REAL_ADDRESS);
  return address;
}
Byte nes_readPPUMemory(NES nes, Address address) {
  //assert(nes != NULL);
  address = nes_getLowPPUAddress(address);
  return memory_read_callback(nes, nes_getPPUMemory(nes), address);
}
void nes_writePPUMemory(NES nes, Address address, Byte data) {
  //assert(nes != NULL);
  address = nes_getLowPPUAddress(address);
  memory_write_callback(nes, nes_getPPUMemory(nes), address, data);
}
APU getAPU(NES nes) {
  //assert(nes != NULL);
  return nes->apu;
}
void nes_cpuCycled(NES nes) {
  //assert(nes != NULL);
  ppu_step(nes);
 // apu_step(nes); 3 ppu_step por cada cpu_step para NTSC
  ppu_step(nes);
 // apu_step(nes);
  ppu_step(nes);
 // apu_step(nes);
  // adjust for pal frequency here
  // if (PAL) {
  // }
}