#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "globals.h"
#include "memory.h"
#include "cpu.h"
#include "ppu.h"
#include "interrupts.h"
#include "cartridge.h"
#include "nes.h"
#include "usart.h"
struct memory {
  uint32_t numAddresses;
  Byte *memoryU;
  NES_WriteCallback *writeCallbacks;
  NES_ReadCallback *readCallbacks;
};
Memory memory_init(uint32_t numAddresses) {
  Memory memory1 = malloc(sizeof(struct memory));
  //assert(memory1 != NULL);
  memory1->memoryU = (Byte*) malloc(sizeof(Byte) * numAddresses);
 //assert(memory1->memoryU != NULL);
 usart_write_line(&AVR32_USART0,"CPU33");
 
  int i;
  
  for (i=0; i < numAddresses; i++) {
    memory1->memoryU[i] = 0;
  }
  
  memory1->writeCallbacks = (NES_WriteCallback*) malloc(sizeof(NES_WriteCallback) * numAddresses);
  //assert(memory1->writeCallbacks != NULL);
  for (i=0; i < numAddresses; i++) {
    memory1->writeCallbacks[i] = NULL;
  }
  memory1->readCallbacks = (NES_ReadCallback*) malloc(sizeof(NES_ReadCallback) * numAddresses);
  //assert(memory1->readCallbacks != NULL);
  for (i=0; i < numAddresses; i++) {
    memory1->readCallbacks[i] = NULL;
  }
  usart_write_line(&AVR32_USART0,"CPU43");
  memory1->numAddresses = numAddresses;
  return memory1;
}
void memory_destroy(Memory memory) {
  //assert(memory != NULL);
  free(memory);
}
void memory_write_direct(Memory memory, Address address, Byte data) {
  //assert(memory != NULL);
  //assert(address < memory->numAddresses);
  memory->memoryU[address] = data;
}
Byte memory_read_direct(Memory memory, Address address) {
  //assert(memory != NULL);
  //assert(address < memory->numAddresses);
  return memory->memoryU[address];
}
void memory_print(NES nes, Memory memory) {
  //assert(memory != NULL);
  usart_write_line(&AVR32_USART0,"\n");
  //usart_write_line(&AVR32_USART0,"%d addresses:\n", memory->numAddresses);
  int i;
  for (i=0; i < memory->numAddresses; i++) {
    if (i % 16 == 0) {
      usart_write_line(&AVR32_USART0,"\n");
     // usart_write_line(&AVR32_USART0,"%5x: ", i);
    }
   // usart_write_line(&AVR32_USART0,"%02x ", memory_read_callback(nes, memory, i));
  }
  usart_write_line(&AVR32_USART0,"\n");
}
int memory_getNumAddresses(Memory memory) {
  //assert(memory != NULL);
  return memory->numAddresses;
}
void memory_setWriteCallback(Memory memory, Address address, NES_WriteCallback writeCallback) {
  //assert(memory != NULL);
  memory->writeCallbacks[address] = writeCallback;
}
void memory_setReadCallback(Memory memory, Address address, NES_ReadCallback readCallback) {
  //assert(memory != NULL);
  memory->readCallbacks[address] = readCallback;
}
void memory_write_callback(NES nes, Memory memory1, Address address, Byte data) {
  //assert(memory != NULL);
  if (memory1->writeCallbacks[address] == NULL) {
    memory_write_direct(memory1, address, data);
  } else {
    memory1->writeCallbacks[address](nes, address, data);
  }
}
Byte memory_read_callback(NES nes, Memory memory1, Address address) {
  //assert(memory1 != NULL);
  //usart_write_line(&AVR32_USART0,"readCallback\n");
  if (memory1->readCallbacks[address] == NULL) {
	  //usart_write_line(&AVR32_USART0,"readCallback1_1\n");
    return memory_read_direct(memory1, address);
  } else {
	  //usart_write_line(&AVR32_USART0,"readCallback1_2\n");
    return memory1->readCallbacks[address](nes,address);
	//return memory_read_direct(memory1, address);
  }
  
}