#include "mmu_type.h"
#include "nes_type.h"
#include "globals.h"
#define MMU_PROGRAM_BANK_LOWER_FIRST_ADDRESS 0x8000
#define MMU_PROGRAM_BANK_LOWER_LAST_ADDRESS 0xBFFF
#define MMU_PROGRAM_BANK_UPPER_FIRST_ADDRESS 0xC000
#define MMU_PROGRAM_BANK_UPPER_LAST_ADDRESS 0xFFFF
MMU mmu_init(NES nes);
Byte mmu_getActiveProgramBank_lower(MMU mmu);
Byte mmu_getActiveProgramBank_upper(MMU mmu);
Byte mmu_getActiveCharacterBank(MMU mmu);
void mmu_setActiveProgramBank_lower(MMU mmu, Byte activeProgramBank_lower);
void mmu_setActiveProgramBank_upper(MMU mmu, Byte activeProgramBank_upper);
void mmu_setActiveCharacterBank(MMU mmu, Byte activeCharacterBank);
void mmu_destroy(MMU mmu);
Byte mmu_ppuMemory_readHorizontalMirror(NES nes, Address address);
void mmu_ppuMemory_writeHorizontalMirror(NES nes, Address address, Byte byte);
Byte mmu_ppuMemory_readVerticalMirror(NES nes, Address address);
void mmu_ppuMemory_writeVerticalMirror(NES nes, Address address, Byte byte);