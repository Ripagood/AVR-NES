#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "memory.h"
#include "ppuMemory.h"
#include "nes.h"
////////////////////////////////////////////////////////////////////////////////////////
//
// Addresses $3F10/$3F14/$3F18/$3F1C are mirrors of $3F00/$3F04/$3F08/$3F0C.
static void ppuMemory_transparencyMirror_writer(NES nes, Address address, Byte data) {
  Memory ppuMemory = nes_getPPUMemory(nes);
  assert(ppuMemory != NULL);
  address -= PPU_BACKGROUND_PALETTE_TOTAL_SIZE;
  assert(address >= PPU_BACKGROUND_PALETTE_FIRST_ADDRESS);
  assert(address <= PPU_BACKGROUND_PALETTE_LAST_ADDRESS);
  memory_write_callback(nes, ppuMemory, address, data);
}
static Byte ppuMemory_transparencyMirror_reader(NES nes, Address address) {
  Memory ppuMemory = nes_getPPUMemory(nes);
  assert(ppuMemory != NULL);
  address -= PPU_BACKGROUND_PALETTE_TOTAL_SIZE;
  assert(address >= PPU_BACKGROUND_PALETTE_FIRST_ADDRESS);
  assert(address <= PPU_BACKGROUND_PALETTE_LAST_ADDRESS);
  Byte data = memory_read_callback(nes, ppuMemory, address);
  return data;
}
////////////////////////////////////////////////////////////////////////////////////////
//
static Address ppuMemory_paletteMirror_getLowestAddress(Address address) {
  while(address > PPU_GENUINE_PALETTE_LAST_ADDRESS) {
    address -= PPU_PALETTE_MIRRORED_SIZE;
  }
  assert(address <= PPU_GENUINE_PALETTE_LAST_ADDRESS);
  return address;
}
// Both palettes are also mirrored to $3F20-$3FFF.
static void ppuMemory_paletteMirror_writer(NES nes, Address address, Byte data) {
  Memory ppuMemory = nes_getPPUMemory(nes);
  assert(ppuMemory != NULL);
  address = ppuMemory_paletteMirror_getLowestAddress(address);
  memory_write_callback(nes, ppuMemory, address, data);
}
static Byte ppuMemory_paletteMirror_reader(NES nes, Address address) {
  Memory ppuMemory = nes_getPPUMemory(nes);
  assert(ppuMemory != NULL);
  address = ppuMemory_paletteMirror_getLowestAddress(address);
  Byte data = memory_read_callback(nes, ppuMemory, address);
  return data;
}
////////////////////////////////////////////////////////////////////////////////////////
//
static void ppuMemory_nametableMirror_writer(NES nes, Address address, Byte data) {
  Memory ppuMemory = nes_getPPUMemory(nes);
  assert(ppuMemory != NULL);
  address -= PPU_NAME_TABLE_TOTAL_SIZE;
  assert(address >= PPU_NAME_TABLE_0_FIRST_ADDRESS);
  assert(address <= PPU_NAME_TABLE_3_FIRST_ADDRESS);
  memory_write_callback(nes, ppuMemory, address, data);
}
static Byte ppuMemory_nametableMirror_reader(NES nes, Address address) {
  Memory ppuMemory = nes_getPPUMemory(nes);
  assert(ppuMemory != NULL);
  address -= PPU_NAME_TABLE_TOTAL_SIZE;
  assert(address >= PPU_NAME_TABLE_0_FIRST_ADDRESS);
  assert(address <= PPU_NAME_TABLE_3_FIRST_ADDRESS);
  Byte data = memory_read_callback(nes, ppuMemory, address);
  return data;
}
////////////////////////////////////////////////////////////////////////////////////////
//
Memory ppuMemory_init(void) {
  Memory memory = memory_init(PPU_NUM_REAL_ADDRESSES);
  assert(memory != NULL);
  Address address;
  // first setup the name table mirror
  for (address = PPU_NAME_TABLE_MIRROR_FIRST_ADDRESS; address <=
PPU_NAME_TABLE_MIRROR_FIRST_ADDRESS; address++) {
    memory_setWriteCallback(memory, address, &ppuMemory_nametableMirror_writer);
    memory_setReadCallback(memory, address, &ppuMemory_nametableMirror_reader);
  }
  // setup the background mirrors for the first byte in each sub palette of the sprite
palette
  memory_setWriteCallback(memory, PPU_SPRITE_PALETTE_0_FIRST_ADDRESS,
&ppuMemory_transparencyMirror_writer);
  memory_setReadCallback(memory, PPU_SPRITE_PALETTE_0_FIRST_ADDRESS,
&ppuMemory_transparencyMirror_reader);
  memory_setWriteCallback(memory, PPU_SPRITE_PALETTE_1_FIRST_ADDRESS,
&ppuMemory_transparencyMirror_writer);
  memory_setReadCallback(memory, PPU_SPRITE_PALETTE_1_FIRST_ADDRESS,
&ppuMemory_transparencyMirror_reader);
  memory_setWriteCallback(memory, PPU_SPRITE_PALETTE_2_FIRST_ADDRESS,
&ppuMemory_transparencyMirror_writer);
  memory_setReadCallback(memory, PPU_SPRITE_PALETTE_2_FIRST_ADDRESS,
&ppuMemory_transparencyMirror_reader);
  memory_setWriteCallback(memory, PPU_SPRITE_PALETTE_3_FIRST_ADDRESS,
&ppuMemory_transparencyMirror_writer);
  memory_setReadCallback(memory, PPU_SPRITE_PALETTE_3_FIRST_ADDRESS,
&ppuMemory_transparencyMirror_reader);
  // now setup the mirror proper
  for (address=PPU_PALETTE_MIRROR_FIRST_ADDRESS; address <=
PPU_PALETTE_MIRROR_LAST_ADDRESS; address++) {
    memory_setWriteCallback(memory, address, &ppuMemory_paletteMirror_writer);
    memory_setReadCallback(memory, address, &ppuMemory_paletteMirror_reader);
  }
  return memory;
}