#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "nes.h"
#include "mmu.h"
#include "mmu0.h"
#include "cartridge.h"
#include "ppuMemory.h"
#include "memory.h"
typedef void (*MMU_CallbackCreator)(NES nes, MMU mmu);
MMU_CallbackCreator mmu_callbackCreators[] = {
  &mmu0_callbackCreator
};
int num_mmu_callbackCreators = sizeof(mmu_callbackCreators)/sizeof(MMU_CallbackCreator);
struct mmu {
  Byte activeProgramBank_lower;
  Byte activeProgramBank_upper;
  Byte activeCharacterBank;
};
MMU mmu_init(NES nes) {
  //assert(nes != NULL);
  MMU mmu = (MMU) malloc(sizeof(struct mmu));
  //assert(mmu != NULL);
  mmu->activeProgramBank_lower = 0;
  mmu->activeProgramBank_upper = 0;
  mmu->activeCharacterBank = 0;
  Cartridge cartridge = nes_getCartridge(nes);
  //assert(cartridge != NULL);
  Byte mmuNumber = cartridge_getMMUNumber(cartridge);
  //assert(mmuNumber < num_mmu_callbackCreators);
  MMU_CallbackCreator mmu_callbackCreator = mmu_callbackCreators[mmuNumber];
 // MMU_CallbackCreator mmu_callbackCreator = mmu_callbackCreators[0];
  //assert(mmu_callbackCreator != NULL);
  mmu_callbackCreator(nes, mmu);
  return mmu;
}
Byte mmu_getActiveProgramBank_lower(MMU mmu) {
  //assert(mmu != NULL);
  return mmu->activeProgramBank_lower;
}
Byte mmu_getActiveProgramBank_upper(MMU mmu) {
  //assert(mmu != NULL);
  return mmu->activeProgramBank_upper;
}
Byte mmu_getActiveCharacterBank(MMU mmu) {
  //assert(mmu != NULL);
  return mmu->activeCharacterBank;
}
void mmu_setActiveProgramBank_lower(MMU mmu, Byte activeProgramBank_lower) {
  //assert(mmu != NULL);
  mmu->activeProgramBank_lower = activeProgramBank_lower;
}
void mmu_setActiveProgramBank_upper(MMU mmu, Byte activeProgramBank_upper) {
  //assert(mmu != NULL);
  mmu->activeProgramBank_upper = activeProgramBank_upper;
}
void mmu_setActiveCharacterBank(MMU mmu, Byte activeCharacterBank) {
  //assert(mmu != NULL);
  mmu->activeCharacterBank = activeCharacterBank;
}
void mmu_destroy(MMU mmu) {
  //assert(mmu != NULL);
  free(mmu);
}
////////////////////////////////////////////////////////////////////////////////////////
////////////
Byte mmu_ppuMemory_readHorizontalMirror(NES nes, Address address) {
  //assert(nes != NULL);
  Memory ppuMemory = nes_getPPUMemory(nes);
  //assert(ppuMemory != NULL);
  address -= PPU_NAME_TABLE_SIZE;
  return memory_read_callback(nes, ppuMemory, address);
}
void mmu_ppuMemory_writeHorizontalMirror(NES nes, Address address, Byte byte) {
  //assert(nes != NULL);
  Memory ppuMemory = nes_getPPUMemory(nes);
  //assert(ppuMemory != NULL);
  address -= PPU_NAME_TABLE_SIZE;
  memory_write_callback(nes, ppuMemory, address, byte);
}
////////////////////////////////////////////////////////////////////////////////////////
////////////
Byte mmu_ppuMemory_readVerticalMirror(NES nes, Address address) {
  //assert(nes != NULL);
  Memory ppuMemory = nes_getPPUMemory(nes);
  //assert(ppuMemory != NULL);
  address -= PPU_NAME_TABLE_SIZE;
  address -= PPU_NAME_TABLE_SIZE;
  return memory_read_callback(nes, ppuMemory, address);
}
void mmu_ppuMemory_writeVerticalMirror(NES nes, Address address, Byte byte) {
  //assert(nes != NULL);
  Memory ppuMemory = nes_getPPUMemory(nes);
  //assert(ppuMemory != NULL);
  address -= PPU_NAME_TABLE_SIZE;
  address -= PPU_NAME_TABLE_SIZE;
  memory_write_callback(nes, ppuMemory, address, byte);
}
////////////////////////////////////////////////////////////////////////////////////////
////////////