#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "globals.h"
#include "nes.h"
#include "mmu.h"
#include "mmu0.h"
#include "memory.h"
#include "cartridge.h"
#include "ppuMemory.h"
#include "usart.h"
////////////////////////////////////////////////////////////////////////////////////////
////////////
static Address mmu0_cpuMemory_getLocalAddress(Address address) {
	//usart_write_line(&AVR32_USART0,"mmu0");
  while (address >= BYTES_PER_PROGRAM_BANK) {
    address -= BYTES_PER_PROGRAM_BANK;
  }
  ////assert(address < BYTES_PER_PROGRAM_BANK);
  //usart_write_line(&AVR32_USART0,"mmu0");
  return address;
}
////////////////////////////////////////////////////////////////////////////////////////
////////////
static Byte mmu0_cpuMemory_readProgramBank0(NES nes, Address address) {
  ////assert(nes != NULL);
  //Memory memory = nes_getCPUMemory(nes);
  ////assert(memory != NULL);
  //address = mmu0_cpuMemory_getLocalAddress(address);
  address = address -0x8000; //primer banko
  Cartridge cartridge = nes_getCartridge(nes);
  //assert(nes != NULL);
  //assert(0 < cartridge_getNumProgramBanks(cartridge));
  return cartridge_readProgramBank(cartridge, 0, address);
}
static void mmu0_cpuMemory_writeProgramBank0(NES nes, Address address, Byte byte) {
  //assert(nes != NULL);
  Memory memory = nes_getCPUMemory(nes);
  //assert(memory != NULL);
  // not allowed to write to ROM
  //assert(FALSE);
}
////////////////////////////////////////////////////////////////////////////////////////
////////////
static Byte mmu0_cpuMemory_readProgramBank1(NES nes, Address address) {
  ////assert(nes != NULL);
  //address = mmu0_cpuMemory_getLocalAddress(address);
  address = address-0xC000; //segundo banko
  Cartridge cartridge = nes_getCartridge(nes);
 // usart_write_line(&AVR32_USART0,"Cartridge\n");
  ////assert(nes != NULL);
  ////assert(1 < cartridge_getNumProgramBanks(cartridge));
  return cartridge_readProgramBank(cartridge, 1, address);
}
static void mmu0_cpuMemory_writeProgramBank1(NES nes, Address address, Byte byte) {
  //assert(nes != NULL);
  Memory memory = nes_getCPUMemory(nes);
  //assert(memory != NULL);
  // not allowed to write to ROM
  //assert(FALSE);
}
////////////////////////////////////////////////////////////////////////////////////////
////////////
static Byte mmu0_ppuMemory_readPattern(NES nes, Address address) {
  //assert(nes != NULL);
  Cartridge cartridge = nes_getCartridge(nes);
  //assert(cartridge != NULL);
  return cartridge_readCharacterBank(cartridge, 0, address);
}
static void mmu0_ppuMemory_writePattern(NES nes, Address address, Byte byte) {
  //assert(nes != NULL);
  ////assert(FALSE); // can't write to ROM
}
////////////////////////////////////////////////////////////////////////////////////////
////////////
void mmu0_callbackCreator(NES nes, MMU mmu) {
  //assert(nes != NULL);
  //assert(mmu != NULL);
  usart_write_line(&AVR32_USART0,"mmu0_callbackCreator\n");
  Memory cpuMemory = nes_getCPUMemory(nes);
  //assert(cpuMemory != NULL);
  int i;
  for (i = MMU_PROGRAM_BANK_LOWER_FIRST_ADDRESS; i <=MMU_PROGRAM_BANK_LOWER_LAST_ADDRESS; i++) {
    //memory_setWriteCallback(cpuMemory, i, &mmu0_cpuMemory_writeProgramBank0);
    memory_setReadCallback(cpuMemory, i, &mmu0_cpuMemory_readProgramBank0);
  }
  Cartridge cartridge = nes_getCartridge(nes);
  //assert(cartridge != NULL);
  Byte numProgramBanks = cartridge_getNumProgramBanks(cartridge);
  //usart_write_char(&AVR32_USART0,numProgramBanks+0x30);
  // mmu0 only supports 1 or 2 program banks on the cartridge
  //assert(numProgramBanks >= 1);
  //assert(numProgramBanks <= 2);
  if (numProgramBanks == 1) {
    // mirror the first bank if there is only one
    for (i = MMU_PROGRAM_BANK_UPPER_FIRST_ADDRESS; i <=
MMU_PROGRAM_BANK_UPPER_LAST_ADDRESS; i++) {
     // memory_setWriteCallback(cpuMemory, i, &mmu0_cpuMemory_writeProgramBank0);
      memory_setReadCallback(cpuMemory, i, &mmu0_cpuMemory_readProgramBank0);
    }
  } else if (numProgramBanks == 2) {
    for (i = MMU_PROGRAM_BANK_UPPER_FIRST_ADDRESS; i <=MMU_PROGRAM_BANK_UPPER_LAST_ADDRESS; i++) {
      //memory_setWriteCallback(cpuMemory, i, &mmu0_cpuMemory_writeProgramBank1);
      memory_setReadCallback(cpuMemory, i, &mmu0_cpuMemory_readProgramBank1);
    }
  }
  Byte numCharacterBanks = cartridge_getNumCharacterBanks(cartridge);
  //assert(numCharacterBanks <= 1);
  Memory ppuMemory = nes_getPPUMemory(nes);
  //assert(ppuMemory != NULL);
  if (numCharacterBanks == 1) {
    // dump it in the lower space of ppu memory
    for (i = PPU_PATTERN_TABLE_0_FIRST_ADDRESS; i <= PPU_PATTERN_TABLE_1_LAST_ADDRESS;
i++) {
      memory_setWriteCallback(ppuMemory, i, &mmu0_ppuMemory_writePattern);
      memory_setReadCallback(ppuMemory, i, &mmu0_ppuMemory_readPattern);
    }
  }
  MirrorType mirrorType = cartridge_getMirrorType(cartridge);
  if (mirrorType == HORIZONTAL) {
    for (i = PPU_NAME_TABLE_1_FIRST_ADDRESS; i <= PPU_NAME_TABLE_1_LAST_ADDRESS; i++) {
      // map it back to table 0
      memory_setWriteCallback(ppuMemory, i, &mmu_ppuMemory_writeHorizontalMirror);
      memory_setReadCallback(ppuMemory, i, &mmu_ppuMemory_readHorizontalMirror);
    }
    for (i = PPU_NAME_TABLE_3_FIRST_ADDRESS; i <= PPU_NAME_TABLE_3_LAST_ADDRESS; i++) {
      // map it back to table 2
      memory_setWriteCallback(ppuMemory, i, &mmu_ppuMemory_writeHorizontalMirror);
      memory_setReadCallback(ppuMemory, i, &mmu_ppuMemory_readHorizontalMirror);
    }
  } else if (mirrorType == VERTICAL) {
    // Vertical mirroring: $2000 equals $2800 and $2400 equals $2C00 (e.g. Super MarioBros.)
    for (i = PPU_NAME_TABLE_2_FIRST_ADDRESS; i <= PPU_NAME_TABLE_2_LAST_ADDRESS; i++) {
      // map it back to table 0
      memory_setWriteCallback(ppuMemory, i, &mmu_ppuMemory_writeVerticalMirror);
      memory_setReadCallback(ppuMemory, i, &mmu_ppuMemory_readVerticalMirror);
    }
    for (i = PPU_NAME_TABLE_3_FIRST_ADDRESS; i <= PPU_NAME_TABLE_3_LAST_ADDRESS; i++) {
      // map it back to table 1
      memory_setWriteCallback(ppuMemory, i, &mmu_ppuMemory_writeVerticalMirror);
      memory_setReadCallback(ppuMemory, i, &mmu_ppuMemory_readVerticalMirror);
    }
  } else if (mirrorType == BOTH) {
    //assert(FALSE);
  }
}