#include  <stdio.h>
#include  <stdlib.h>
#include  <assert.h>
#include  <stdint.h>
#include  "globals.h"
#include  "memory.h"
#include  "cartridge.h"
#include "file.h"
#include "fs_com.h"
#include "nav_filterlist.h"
#include "usart.h"
#include <string.h>


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
  Bool hasBatteryPack;
  Bool hasTrainer;
  Byte mmuNumber;
  Byte numRamBanks;
  CharacterBank *characterBanks;
  ProgramBank *programBanks;
  Byte title[MAX_TITLE_BYTES];
};
void cartridge_print(Cartridge cartridge1) {
	/*
  assert(cartridge1 != NULL);
  debug_printf("\n");
  debug_printf("Cartridge details:\n");
  debug_printf(" numProgramBanks: %d\n", cartridge1->numProgramBanks);
  debug_printf(" numCharacterBanks: %d\n", cartridge1->numCharacterBanks);
  debug_printf(" mirrorType: %d\n", cartridge1->mirrorType);
  debug_printf(" hasBatteryPack: %d\n", cartridge1->hasBatteryPack);
  debug_printf(" hasTrainer: %d\n", cartridge1->hasTrainer);
  debug_printf(" mmuNumber: %d\n", cartridge1->mmuNumber);
  debug_printf(" numRamkBanks: %d\n", cartridge1->numRamBanks);
  debug_printf(" title: %s\n", cartridge1->title);*/
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

void initHeader( Header header1){
	//file_read_buf(&(header1->magicLabel),sizeof(header1->magicLabel));
	
	header1->magicLabel[0]=(uint8_t)file_getc();
	header1->magicLabel[1]=(uint8_t)file_getc();
	header1->magicLabel[2]=(uint8_t)file_getc();
	char mensaje[]={""};
	char z;
	
	

sprintf(mensaje,"%c",header1->magicLabel[0]);
//et024006_PrintString(mensaje,(const unsigned char*)&FONT8x8,10,110+(l+=10),BLUE,-1);
//usart_write_line(&AVR32_USART0,mensaje);


sprintf(mensaje,"%c",header1->magicLabel[1]);
//et024006_PrintString(mensaje,(const unsigned char*)&FONT8x8,10,110+(l+=10),BLUE,-1);
//usart_write_line(&AVR32_USART0,mensaje);


sprintf(mensaje,"%c",header1->magicLabel[2]);
//et024006_PrintString(mensaje,(const unsigned char*)&FONT8x8,10,110+(l+=10),BLUE,-1);
//usart_write_line(&AVR32_USART0,mensaje);
	
	header1->magicValue= file_getc();
	header1->numProgramBanks= file_getc();
	header1->numCharacterBanks= file_getc();
	header1->controlByte1= file_getc();
	header1->controlByte2= file_getc();
	header1->numRamBanks= file_getc();
	file_read_buf(header1->magicReservedValues,sizeof(header1->magicReservedValues));
	};




Byte cartridge_getNumProgramBanks(Cartridge cartridge1) {
//  assert(cartridge1 != NULL);
  return cartridge1->numProgramBanks;
}
inline Byte cartridge_readProgramBank(Cartridge cartridge1, Byte bank, Address address) {
 // assert(cartridge1 != NULL);
 // assert(bank < cartridge1->numProgramBanks);
//  assert(address < BYTES_PER_PROGRAM_BANK);
  return cartridge1->programBanks[bank]->bytes[address];
}
inline Byte cartridge_readCharacterBank(Cartridge cartridge1, Byte bank, Address address) {
 // assert(cartridge1 != NULL);
  //assert(bank < cartridge1->numCharacterBanks);
  //assert(address < BYTES_PER_CHARACTER_BANK);
  return cartridge1->characterBanks[bank]->bytes[address];
}
//static void parseHeader(Cartridge cartridge, FILE *file) {
static void parseHeader(Cartridge cartridge1) {
  //assert(cartridge1 != NULL);
  //assert(file != NULL);
  //file_seek(0, FS_SEEK_SET);
  Header header1 = malloc(sizeof(struct header));
  //assert(fread(header, sizeof(struct header), 1, file) == 1);
 // file_read_buf( header,sizeof(struct header));
  //memcpy(&rawData[0],&header,sizeof(struct header));
  
  initHeader(header1);
  //usart_write_line(&AVR32_USART0,"parsing Header");
  assert(header1->magicLabel[0] == 'N');
  assert(header1->magicLabel[1] == 'E');
  assert(header1->magicLabel[2] == 'S');
  //usart_write_line(&AVR32_USART0,"parsing Header2");
  assert(header1->magicValue == 0x1A);
  //usart_write_line(&AVR32_USART0,"parsing Header3");
  cartridge1->numProgramBanks = header1->numProgramBanks;
  assert(cartridge1->numProgramBanks > 0);
  //usart_write_line(&AVR32_USART0,"parsing Header4");
  cartridge1->numCharacterBanks = header1->numCharacterBanks;
  cartridge1->numRamBanks = header1->numRamBanks;
  if ((header1->controlByte1 & MASK_BIT3) == MASK_BIT3) {
    cartridge1->mirrorType = BOTH;
  } else if ((header1->controlByte1 & MASK_BIT0) == 0) {
    cartridge1->mirrorType = HORIZONTAL;
  } else if ((header1->controlByte1 & MASK_BIT0) == MASK_BIT0) {
    cartridge1->mirrorType = VERTICAL;
  } else {
    assert(FALSE);
  }
  if ((header1->controlByte1 & MASK_BIT1) == MASK_BIT1) {
    cartridge1->hasBatteryPack = 1;
  } else {
    cartridge1->hasBatteryPack = 0;
  }
  if ((header1->controlByte1 & MASK_BIT2) == MASK_BIT2) {
    cartridge1->hasTrainer = 1;
  } else {
    cartridge1->hasTrainer = 0;
  }
  cartridge1->mmuNumber = (header1->controlByte1 & (MASK_BIT4 | MASK_BIT5 | MASK_BIT6 |MASK_BIT7)) >> 4;
  cartridge1->mmuNumber += header1->controlByte2 & (MASK_BIT4 | MASK_BIT5 | MASK_BIT6 |MASK_BIT7);
  assert((header1->controlByte2 & MASK_BIT0) == 0);
  assert((header1->controlByte2 & MASK_BIT1) == 0);
  assert((header1->controlByte2 & MASK_BIT2) == 0);
  assert((header1->controlByte2 & MASK_BIT3) == 0);
  int i;
  //usart_write_line(&AVR32_USART0,"parsing Header5");
  /*
  for (i=0; i < 7; i++) {
    assert(header1->magicReservedValues[i] == 0);
  }
  */
  //usart_write_line(&AVR32_USART0,"parsing Header6");
  
  file_seek(sizeof(struct header), FS_SEEK_SET);
  if (cartridge1->hasTrainer) {
    file_seek(TRAINER_BYTES, FS_SEEK_CUR_FW);
  }
  free(header1);
  
  //usart_write_line(&AVR32_USART0,"parsing Header7");

  }
//static void parseProgramBanks(Cartridge cartridge, FILE *file) {
static void parseProgramBanks(Cartridge cartridge1) {
  assert(cartridge1 != NULL);
  //usart_write_line(&AVR32_USART0,"parse BANK1");
  //assert(file != NULL);
  cartridge1->programBanks = (ProgramBank*) malloc(sizeof(ProgramBank) * cartridge1->numProgramBanks);
  assert(cartridge1->programBanks != NULL);
  //usart_write_line(&AVR32_USART0,"parsing Bak2");
  int i;
 // Byte bytes[BYTES_PER_PROGRAM_BANK];
  for (i=0; i < cartridge1->numProgramBanks; i++) {
    ProgramBank programBank1 = malloc(sizeof(struct programBank));
    
	assert(programBank1 != NULL);
	//usart_write_line(&AVR32_USART0,"parsing Bak3");
    //assert(fread(programBank, sizeof(struct programBank), 1, file) == 1);
    //memcpy(&rawData[sizeof(struct header)-1+i*sizeof(struct programBank)],programBank,sizeof(struct programBank));
    //file_read_buf(bytes,sizeof(bytes));
	file_read_buf(programBank1,sizeof(struct programBank));
	cartridge1->programBanks[i]=programBank1;
	//memcpy(cartridge1->programBanks[i]->bytes,bytes,sizeof(bytes));
    //cartridge1->programBanks[i]->bytes = bytes;
  }
}
//static void parseCharacterBanks(Cartridge cartridge, FILE *file) {
static void parseCharacterBanks(Cartridge cartridge1) {
  assert(cartridge1 != NULL);
  //usart_write_line(&AVR32_USART0,"parsing Bak4");
 // assert(file != NULL);
  cartridge1->characterBanks = (CharacterBank*) malloc(sizeof(struct characterBank) * cartridge1->numCharacterBanks);
  assert(cartridge1->characterBanks != NULL);
  //usart_write_line(&AVR32_USART0,"parsing Bak5");
  int i;
//  Byte bytes[BYTES_PER_CHARACTER_BANK];
  for (i=0; i < cartridge1->numCharacterBanks; i++) {
    CharacterBank characterBank1 = malloc(sizeof(struct characterBank));
    assert(characterBank1 != NULL);
	//usart_write_line(&AVR32_USART0,"parsing Bak6");
	file_read_buf(characterBank1,sizeof(struct characterBank));
    ///assert(fread(characterBank, sizeof(struct characterBank), 1, file) == 1);
   // memcpy(&rawData[sizeof(struct header)-1+(cartridge->numProgramBanks)*sizeof(struct programBank)+i*sizeof(struct characterBank)],characterBank,sizeof(struct characterBank));
    //file_read_buf(bytes,sizeof(bytes));
	//memcpy(cartridge1->characterBanks[i]->bytes,bytes,sizeof(bytes));
      cartridge1->characterBanks[i] = characterBank1;
  }
}

static void parseTitle(Cartridge cartridge1) {
 // assert(cartridge1 != NULL);
  //fread(cartridge->title, sizeof(Byte), 128, file);
}
Cartridge cartridge_init( int filename) {
  Cartridge cartridge1 = malloc(sizeof(struct cartridge));
  assert(cartridge1 != NULL);
  //nav_filterlist_goto((uint16_t)filename); 
  
  //file_open(FOPEN_MODE_R);
  //assert(file != NULL);
  cartridge1->numProgramBanks = 0;
  cartridge1->numCharacterBanks = 0;
  cartridge1->mirrorType = HORIZONTAL;
  cartridge1->hasBatteryPack = FALSE;
  cartridge1->hasTrainer = FALSE;
  cartridge1->mmuNumber = 0;
  cartridge1->numRamBanks = 0;
  cartridge1->characterBanks = NULL;
  cartridge1->programBanks = NULL;
  cartridge1->title[0] = 0;
  //usart_write_line(&AVR32_USART0,"Initiating Cartridge\n");
  parseHeader(cartridge1);
 //usart_write_line(&AVR32_USART0,"Initiating header\n");
  parseProgramBanks(cartridge1);
  //usart_write_line(&AVR32_USART0,"Initiating program banks\n");
  parseCharacterBanks(cartridge1);
  //usart_write_line(&AVR32_USART0,"Initiating Cartridge\n");
  parseTitle(cartridge1);
  file_close();
  //usart_write_line(&AVR32_USART0,"Cartridge1");
  return cartridge1;
}
void cartridge_destroy(Cartridge cartridge1) {
  assert(cartridge1 != NULL);
  int i;
  for (i=0; i < cartridge1->numCharacterBanks; i++) {
    free(cartridge1->characterBanks[i]);
  }
  for (i=0; i < cartridge1->numProgramBanks; i++) {
    free(cartridge1->programBanks[i]);
  }
  free(cartridge1);
}
Byte cartridge_getMMUNumber(Cartridge cartridge1) {
 // assert(cartridge1 != NULL);
  return cartridge1->mmuNumber;
}
Byte cartridge_getNumCharacterBanks(Cartridge cartridge1) {
 // assert(cartridge1 != NULL);
  return cartridge1->numCharacterBanks;
}

MirrorType cartridge_getMirrorType(Cartridge cartridge1) {
 // assert(cartridge1 != NULL);
  return (cartridge1->mirrorType);
}

















