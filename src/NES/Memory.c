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
struct memory {
  int numAddresses;
  Byte *memory;
  NES_WriteCallback *writeCallbacks;
  NES_ReadCallback *readCallbacks;
};
Memory memory_init(int numAddresses) {
  Memory memory = malloc(sizeof(struct memory));
  assert(memory != NULL);
  memory->memory = (Byte*) malloc(sizeof(Byte) * numAddresses);
  assert(memory->memory != NULL);
  int i;
  for (i=0; i < numAddresses; i++) {
    memory->memory[i] = 0;
  }
  memory->writeCallbacks = (NES_WriteCallback*) malloc(sizeof(NES_WriteCallback) *
numAddresses);
  assert(memory->writeCallbacks != NULL);
  for (i=0; i < numAddresses; i++) {
    memory->writeCallbacks[i] = NULL;
  }
  memory->readCallbacks = (NES_ReadCallback*) malloc(sizeof(NES_ReadCallback) *
numAddresses);
  assert(memory->readCallbacks != NULL);
  for (i=0; i < numAddresses; i++) {
    memory->readCallbacks[i] = NULL;
  }
  memory->numAddresses = numAddresses;
  return memory;
}
void memory_destroy(Memory memory) {
  assert(memory != NULL);
  free(memory);
}
void memory_write_direct(Memory memory, Address address, Byte data) {
  assert(memory != NULL);
  assert(address < memory->numAddresses);
  memory->memory[address] = data;
}
Byte memory_read_direct(Memory memory, Address address) {
  assert(memory != NULL);
  assert(address < memory->numAddresses);
  return memory->memory[address];
}
void memory_print(NES nes, Memory memory) {
  assert(memory != NULL);
  debug_printf("\n");
  debug_printf("%d addresses:\n", memory->numAddresses);
  int i;
  for (i=0; i < memory->numAddresses; i++) {
    if (i % 16 == 0) {
      debug_printf("\n");
      debug_printf("%5x: ", i);
    }
    debug_printf("%02x ", memory_read_callback(nes, memory, i));
  }
  debug_printf("\n");
}
int memory_getNumAddresses(Memory memory) {
  assert(memory != NULL);
  return memory->numAddresses;
}
void memory_setWriteCallback(Memory memory, Address address, NES_WriteCallback
writeCallback) {
  assert(memory != NULL);
  memory->writeCallbacks[address] = writeCallback;
}
void memory_setReadCallback(Memory memory, Address address, NES_ReadCallback
readCallback) {
  assert(memory != NULL);
  memory->readCallbacks[address] = readCallback;
}
void memory_write_callback(NES nes, Memory memory, Address address, Byte data) {
  assert(memory != NULL);
  if (memory->writeCallbacks[address] == NULL) {
    memory_write_direct(memory, address, data);
  } else {
    memory->writeCallbacks[address](nes, address, data);
  }
}
Byte memory_read_callback(NES nes, Memory memory, Address address) {
  assert(memory != NULL);
  if (memory->readCallbacks[address] == NULL) {
    return memory_read_direct(memory, address);
  } else {
    return memory->readCallbacks[address](nes, address);
  }
}