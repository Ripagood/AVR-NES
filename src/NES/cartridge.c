#include  <stdio.h>
#include  <stdlib.h>
#include  <assert.h>
#include  <stdint.h>
#include  "globals.h"
#include  "memory.h"
#include  "cartridge.h"

struct characterBank {
  Byte bytes[BYTES_PER_CHARACTER_BANK];
};
typedef struct characterBank *CharacterBank;
struct programBank {
  Byte bytes[BYTES_PER_PROGRAM_BANK];
};
typedef struct programBank *ProgramBank;
struct cartridge {
  Byte numProgramBanks;
  Byte numCharacterBanks;
  MirrorType mirrorType;
  Boolean hasBatteryPack;
  Boolean hasTrainer;
  Byte mmuNumber;
  Byte numRamBanks;
  CharacterBank *characterBanks;
  ProgramBank *programBanks;
  Byte title[MAX_TITLE_BYTES];
};
void cartridge_print(Cartridge cartridge) {
  assert(cartridge != NULL);
  debug_printf("\n");
  debug_printf("Cartridge details:\n");
  debug_printf(" numProgramBanks: %d\n", cartridge->numProgramBanks);
  debug_printf(" numCharacterBanks: %d\n", cartridge->numCharacterBanks);
  debug_printf(" mirrorType: %d\n", cartridge->mirrorType);
  debug_printf(" hasBatteryPack: %d\n", cartridge->hasBatteryPack);
  debug_printf(" hasTrainer: %d\n", cartridge->hasTrainer);
  debug_printf(" mmuNumber: %d\n", cartridge->mmuNumber);
  debug_printf(" numRamkBanks: %d\n", cartridge->numRamBanks);
  debug_printf(" title: %s\n", cartridge->title);
}
typedef struct header *Header;
struct  header {
  Byte  magicLabel[3];
  Byte  magicValue;
  Byte  numProgramBanks;
  Byte  numCharacterBanks;
  Byte  controlByte1;
  Byte  controlByte2;
  Byte  numRamBanks;
  Byte  magicReservedValues[7];
};
Byte cartridge_getNumProgramBanks(Cartridge cartridge) {
  assert(cartridge != NULL);
  return cartridge->numProgramBanks;
}
Byte cartridge_readProgramBank(Cartridge cartridge, Byte bank, Address address) {
  assert(cartridge != NULL);
  assert(bank < cartridge->numProgramBanks);
  assert(address < BYTES_PER_PROGRAM_BANK);
  return cartridge->programBanks[bank]->bytes[address];
}
Byte cartridge_readCharacterBank(Cartridge cartridge, Byte bank, Address address) {
  assert(cartridge != NULL);
  assert(bank < cartridge->numCharacterBanks);
  assert(address < BYTES_PER_CHARACTER_BANK);
  return cartridge->characterBanks[bank]->bytes[address];
}
//static void parseHeader(Cartridge cartridge, FILE *file) {
static void parseHeader(Cartridge cartridge) {
  assert(cartridge != NULL);
  //assert(file != NULL);
  file_seek(0, FS_SEEK_SET);
  Header header = malloc(sizeof(struct header));
  //assert(fread(header, sizeof(struct header), 1, file) == 1);
  file_read_buf( header,sizeof(struct header);
  //memcpy(&rawData[0],&header,sizeof(struct header));
  assert(header->magicLabel[0] == 'N');
  assert(header->magicLabel[1] == 'E');
  assert(header->magicLabel[2] == 'S');
  assert(header->magicValue == 0x1A);
  cartridge->numProgramBanks = header->numProgramBanks;
  assert(cartridge->numProgramBanks > 0);
  cartridge->numCharacterBanks = header->numCharacterBanks;
  cartridge->numRamBanks = header->numRamBanks;
  if ((header->controlByte1 & MASK_BIT3) == MASK_BIT3) {
    cartridge->mirrorType = BOTH;
  } else if ((header->controlByte1 & MASK_BIT0) == 0) {
    cartridge->mirrorType = HORIZONTAL;
  } else if ((header->controlByte1 & MASK_BIT0) == MASK_BIT0) {
    cartridge->mirrorType = VERTICAL;
  } else {
    assert(FALSE);
  }
  if ((header->controlByte1 & MASK_BIT1) == MASK_BIT1) {
    cartridge->hasBatteryPack = 1;
  } else {
    cartridge->hasBatteryPack = 0;
  }
  if ((header->controlByte1 & MASK_BIT2) == MASK_BIT2) {
    cartridge->hasTrainer = 1;
  } else {
    cartridge->hasTrainer = 0;
  }
  cartridge->mmuNumber = (header->controlByte1 & (MASK_BIT4 | MASK_BIT5 | MASK_BIT6 |
MASK_BIT7)) >> 4;
  cartridge->mmuNumber += header->controlByte2 & (MASK_BIT4 | MASK_BIT5 | MASK_BIT6 |
MASK_BIT7);
  assert((header->controlByte2 & MASK_BIT0) == 0);
  assert((header->controlByte2 & MASK_BIT1) == 0);
  assert((header->controlByte2 & MASK_BIT2) == 0);
  assert((header->controlByte2 & MASK_BIT3) == 0);
  int i;
  for (i=0; i < 7; i++) {
    assert(header->magicReservedValues[i] == 0);
  }
  
  file_seek(sizeof(struct header), FS_SEEK_SET);
  if (cartridge->hasTrainer) {
    fseek(TRAINER_BYTES, FS_SEEK_CUR);
 
  free(header);
}
//static void parseProgramBanks(Cartridge cartridge, FILE *file) {
static void parseProgramBanks(Cartridge cartridge) {
  assert(cartridge != NULL);
  //assert(file != NULL);
  cartridge->programBanks = (ProgramBank*) malloc(sizeof(ProgramBank) * cartridge->numProgramBanks);
  assert(cartridge->programBanks != NULL);
  int i;
  for (i=0; i < cartridge->numProgramBanks; i++) {
    ProgramBank programBank = malloc(sizeof(struct programBank));
    assert(programBank != NULL);
    //assert(fread(programBank, sizeof(struct programBank), 1, file) == 1);
    //memcpy(&rawData[sizeof(struct header)-1+i*sizeof(struct programBank)],programBank,sizeof(struct programBank));
    file_read_buf(programBank,sizeof(struct programBank));
    cartridge->programBanks[i] = programBank;
  }
}
//static void parseCharacterBanks(Cartridge cartridge, FILE *file) {
static void parseCharacterBanks(Cartridge cartridge) {
  assert(cartridge != NULL);
 // assert(file != NULL);
  cartridge->characterBanks = (CharacterBank*) malloc(sizeof(CharacterBank) * cartridge->numCharacterBanks);
  assert(cartridge->characterBanks != NULL);
  int i;
  for (i=0; i < cartridge->numCharacterBanks; i++) {
    CharacterBank characterBank = malloc(sizeof(struct characterBank));
    assert(characterBank != NULL);
    ///assert(fread(characterBank, sizeof(struct characterBank), 1, file) == 1);
   // memcpy(&rawData[sizeof(struct header)-1+(cartridge->numProgramBanks)*sizeof(struct programBank)+i*sizeof(struct characterBank)],characterBank,sizeof(struct characterBank));
    file_read_buf(characterBank,sizeof(struct characterBank));
    cartridge->characterBanks[i] = characterBank;
  }
}
static void parseTitle(Cartridge cartridge) {
  assert(cartridge != NULL);
  assert(file != NULL);
  //fread(cartridge->title, sizeof(Byte), 128, file);
}
Cartridge cartridge_init( int file) {
  Cartridge cartridge = malloc(sizeof(struct cartridge));
  assert(cartridge != NULL);
  file_open(FOPEN_MODE_R);
  //assert(file != NULL);
  cartridge->numProgramBanks = 0;
  cartridge->numCharacterBanks = 0;
  cartridge->mirrorType = HORIZONTAL;
  cartridge->hasBatteryPack = FALSE;
  cartridge->hasTrainer = FALSE;
  cartridge->mmuNumber = 0;
  cartridge->numRamBanks = 0;
  cartridge->characterBanks = NULL;
  cartridge->programBanks = NULL;
  cartridge->title[0] = 0;
  parseHeader(cartridge);
  parseProgramBanks(cartridge);
  parseCharacterBanks(cartridge);
  parseTitle(cartridge);
  file_close();
  return cartridge;
}
void cartridge_destroy(Cartridge cartridge) {
  assert(cartridge != NULL);
  int i;
  for (i=0; i < cartridge->numCharacterBanks; i++) {
    free(cartridge->characterBanks[i]);
  }
  for (i=0; i < cartridge->numProgramBanks; i++) {
    free(cartridge->programBanks[i]);
  }
  free(cartridge);
}
Byte cartridge_getMMUNumber(Cartridge cartridge) {
  assert(cartridge != NULL);
  return cartridge->mmuNumber;
}
Byte cartridge_getNumCharacterBanks(Cartridge cartridge) {
  assert(cartridge != NULL);
  return cartridge->numCharacterBanks;
}
MirrorType cartridge_getMirrorType(Cartridge cartridge) {
  assert(cartridge != NULL);
  return cartridge->mirrorType;
}

















