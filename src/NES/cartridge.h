#include "cartridge_type.h"
#include "globals.h"
#define BYTES_PER_CHARACTER_BANK (BYTES_PER_KILOBYTE * 8)
#define BYTES_PER_PROGRAM_BANK (BYTES_PER_KILOBYTE * 16)
#define MAX_TITLE_BYTES 128
#define TRAINER_BYTES 512
// affects how ppu memory address spaces are mirrored
typedef enum {
  HORIZONTAL = 0,
  VERTICAL = 1,
  BOTH = 2
} MirrorType;
Cartridge cartridge_init(char *filename);
void cartridge_print(Cartridge cartridge);
void cartridge_destroy(Cartridge cartridge);
Byte cartridge_getMMUNumber(Cartridge cartridge);
Byte cartridge_getNumProgramBanks(Cartridge cartridge);
Byte cartridge_getNumCharacterBanks(Cartridge cartridge);
MirrorType cartridge_getMirrorType(Cartridge cartridge);
Byte cartridge_readProgramBank(Cartridge cartridge, Byte bank, Address address);
Byte cartridge_readCharacterBank(Cartridge cartridge, Byte bank, Address address);