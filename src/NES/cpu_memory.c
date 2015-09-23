#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "globals.h"
#include "memory.h"
#include "cpuMemory.h"
#include "nes.h"
#include "ppu.h"
#include "ppuMemory.h"
#include "objectAttributeMemory.h"
////////////////////////////////////////////////////////////////////////////////////////
//
static Address cpuMemory_ramMirror_getLowestAddress(Address address) {
  while(address > CPU_GENUINE_RAM_LAST_ADDRESS) {
    address -= CPU_RAM_MIRRORED_SIZE;
  }
  assert(address <= CPU_GENUINE_RAM_LAST_ADDRESS);
  return address;
}
static void cpuMemory_ramMirror_writer(NES nes, Address address, Byte data) {
  // Memory locations $0000-$07FF are mirrored three times at $0800-$1FFF.
  // This means that, for example, any data written to $0000 will also be written to
$0800, $1000 and $1800.
  address = cpuMemory_ramMirror_getLowestAddress(address);
  Memory cpuMemory = nes_getCPUMemory(nes);
  assert(cpuMemory != NULL);
  memory_write_callback(nes, cpuMemory, address, data);
}
static Byte cpuMemory_ramMirror_reader(NES nes, Address address) {
  // Memory locations $0000-$07FF are mirrored three times at $0800-$1FFF.
  // This means that, for example, any data written to $0000 will also be written to
$0800, $1000 and $1800.
  address = cpuMemory_ramMirror_getLowestAddress(address);
  Memory cpuMemory = nes_getCPUMemory(nes);
  assert(cpuMemory != NULL);
  Byte data = memory_read_callback(nes, cpuMemory, address);
  return data;
}
////////////////////////////////////////////////////////////////////////////////////////
//
static Address cpuMemory_ppuMirror_getLowestAddress(Address address) {
  while(address > CPU_GENUINE_PPU_LAST_ADDRESS) {
    address -= CPU_PPU_MIRRORED_SIZE;
  }
  assert(address <= CPU_GENUINE_PPU_LAST_ADDRESS);
  return address;
}
static void cpuMemory_ppuMirror_writer(NES nes, Address address, Byte data) {
  address = cpuMemory_ppuMirror_getLowestAddress(address);
  Memory cpuMemory = nes_getCPUMemory(nes);
  assert(cpuMemory != NULL);
  memory_write_callback(nes, cpuMemory, address, data);
}
static Byte cpuMemory_ppuMirror_reader(NES nes, Address address) {
  address = cpuMemory_ppuMirror_getLowestAddress(address);
  Memory cpuMemory = nes_getCPUMemory(nes);
  assert(cpuMemory != NULL);
  Byte data = memory_read_callback(nes, cpuMemory, address);
  return data;
}
////////////////////////////////////////////////////////////////////////////////////////
///
// #define CPU_PPU_CONTROL_REGISTER_ADDRESS                0x2000 // write
static Byte cpuMemory_ppuControlRegister_reader(NES nes, Address address) {
  assert(nes != NULL);
  // not allowed to read this
  // assert(FALSE);
  PPU ppu = nes_getPPU(nes);
  assert(ppu != NULL);
  return ppu_getControlRegister(ppu);
}
static void cpuMemory_ppuControlRegister_writer(NES nes, Address address, Byte data) {
  assert(nes != NULL);
  PPU ppu = nes_getPPU(nes);
  assert(ppu != NULL);
  ppu_setControlRegister(ppu, data);
}
////////////////////////////////////////////////////////////////////////////////////////
///
// #define CPU_PPU_MASK_REGISTER_ADDRESS                   0x2001 // write
static Byte cpuMemory_ppuMaskRegister_reader(NES nes, Address address) {
  assert(nes != NULL);
  // not allowed to read this
  // assert(FALSE);
  PPU ppu = nes_getPPU(nes);
  assert(ppu != NULL);
  return ppu_getMaskRegister(ppu);
}
static void cpuMemory_ppuMaskRegister_writer(NES nes, Address address, Byte data) {
  assert(nes != NULL);
  PPU ppu = nes_getPPU(nes);
  assert(ppu != NULL);
  ppu_setMaskRegister(ppu, data);
}
////////////////////////////////////////////////////////////////////////////////////////
///
// #define CPU_PPU_STATUS_REGISTER_ADDRESS                 0x2002 // read
static Byte cpuMemory_ppuStatusRegister_reader(NES nes, Address address) {
  assert(nes != NULL);
  PPU ppu = nes_getPPU(nes);
  assert(ppu != NULL);
  return ppu_getStatusRegister(ppu);
}
static void cpuMemory_ppuStatusRegister_writer(NES nes, Address address, Byte data) {
  assert(nes != NULL);
  // not allowed to write this
  assert(FALSE);
}
////////////////////////////////////////////////////////////////////////////////////////
///
// #define CPU_PPU_SPRITE_ADDRESS_REGISTER_ADDRESS         0x2003 // write
static Byte cpuMemory_ppuSpriteAddressRegister_reader(NES nes, Address address) {
  assert(nes != NULL);
  PPU ppu = nes_getPPU(nes);
  assert(ppu != NULL);
  return ppu_getSpriteAddressRegister(ppu);
}
static void cpuMemory_ppuSpriteAddressRegister_writer(NES nes, Address address, Byte
data) {
  assert(nes != NULL);
  PPU ppu = nes_getPPU(nes);
  assert(ppu != NULL);
  ppu_setSpriteAddressRegister(ppu, data);
}
////////////////////////////////////////////////////////////////////////////////////////
///
// #define CPU_PPU_SPRITE_DATA_REGISTER_ADDRESS            0x2004 // write
static Byte cpuMemory_ppuSpriteDataRegister_reader(NES nes, Address address) {
  assert(nes != NULL);
  PPU ppu = nes_getPPU(nes);
  assert(ppu != NULL);
  return ppu_getSpriteDataRegister(nes);
}
static void cpuMemory_ppuSpriteDataRegister_writer(NES nes, Address address, Byte data)
{
  assert(nes != NULL);
  PPU ppu = nes_getPPU(nes);
  assert(ppu != NULL);
  ppu_setSpriteDataRegister(nes, data);
}
////////////////////////////////////////////////////////////////////////////////////////
///
// #define CPU_PPU_SCROLL_REGISTER_ADDRESS                 0x2005 // write
static Byte cpuMemory_ppuScrollRegister_reader(NES nes, Address address) {
  assert(nes != NULL);
  PPU ppu = nes_getPPU(nes);
  assert(ppu != NULL);
  return ppu_getScrollRegister(ppu);
}
static void cpuMemory_ppuScrollRegister_writer(NES nes, Address address, Byte data) {
  assert(nes != NULL);
  PPU ppu = nes_getPPU(nes);
  assert(ppu != NULL);
  ppu_setScrollRegister(ppu, data);
}
////////////////////////////////////////////////////////////////////////////////////////
///
// #define CPU_PPUMEMORY_ADDRESS_REGISTER_ADDRESS          0x2006 // write
static Byte cpuMemory_ppuMemoryAddressRegister_reader(NES nes, Address address) {
  assert(nes != NULL);
  PPU ppu = nes_getPPU(nes);
  assert(ppu != NULL);
  return ppu_getPPUMemoryAddressRegister(ppu);
}
static void cpuMemory_ppuMemoryAddressRegister_writer(NES nes, Address address, Byte
data) {
  assert(nes != NULL);
  PPU ppu = nes_getPPU(nes);
  assert(ppu != NULL);
  ppu_setPPUMemoryAddressRegister(ppu, data);
}
////////////////////////////////////////////////////////////////////////////////////////
///
// #define CPU_PPUMEMORY_DATA_REGISTER_ADDRESS             0x2007 // read/write
static Byte cpuMemory_ppuMemoryDataRegister_reader(NES nes, Address address) {
  assert(nes != NULL);
  PPU ppu = nes_getPPU(nes);
  assert(ppu != NULL);
  return ppu_getPPUMemoryDataRegister(nes);
}
static void cpuMemory_ppuMemoryDataRegister_writer(NES nes, Address address, Byte data)
{
  assert(nes != NULL);
  PPU ppu = nes_getPPU(nes);
  assert(ppu != NULL);
  ppu_setPPUMemoryDataRegister(nes, data);
}
////////////////////////////////////////////////////////////////////////////////////////
///
// #define CPU_SPRITE_DMA_REGISTER_ADDRESS                 0x4014 // write
#define CPU_DMA_ADDRESS_MULTIPLIER 0x0100
static Byte cpuMemory_spriteDMARegister_reader(NES nes, Address address) {
  assert(nes != NULL);
  return 0;
}
static void cpuMemory_spriteDMARegister_writer(NES nes, Address address, Byte data) {
  assert(nes != NULL);
  Address readAddress = data * CPU_DMA_ADDRESS_MULTIPLIER;
  Address offset;
  Memory cpuMemory = nes_getCPUMemory(nes);
  assert(cpuMemory != NULL);
  Memory objectAttributeMemory = nes_getObjectAttributeMemory(nes);
  assert(objectAttributeMemory != NULL);
  for (offset=0; offset < OAM_NUM_ADDRESSES; offset++) {
    Byte data = memory_read_callback(nes, cpuMemory, readAddress + offset);
    nes_cpuCycled(nes);
    memory_write_callback(nes, objectAttributeMemory, offset, data);
    nes_cpuCycled(nes);
  }
}
////////////////////////////////////////////////////////////////////////////////////////
///
// CPU_JOYPAD_0_ADDRESS
// CPU_JOYPAD_1_ADDRESS
static Byte cpuMemory_joypad_reader(NES nes, Address address) {
  assert(nes != NULL);
  int currentJoypad = address - CPU_JOYPAD_0_ADDRESS;
  assert(currentJoypad >= 0);
  assert(currentJoypad <= 3);
  return nes_readJoypad(nes, currentJoypad);
}
static void cpuMemory_joypad_writer(NES nes, Address address, Byte data) {
  assert(nes != NULL);
  int currentJoypad = address - CPU_JOYPAD_0_ADDRESS;
  assert(currentJoypad >= 0);
  assert(currentJoypad <= 3);
  nes_writeJoypad(nes, currentJoypad, data);
}
////////////////////////////////////////////////////////////////////////////////////////
///
Memory cpuMemory_init(void) {
  Memory memory = memory_init(CPU_TOTAL_MEMORY_ADDRESSES);
  assert(memory != NULL);
  int address;
  //for (address=CPU_FIRST_RAM_MIRRORED_ADDRESS; address <=
CPU_LAST_RAM_MIRRORED_ADDRESS; address++) {
  for (address=CPU_RAM_MIRROR_FIRST_ADDRESS; address <= CPU_RAM_MIRROR_LAST_ADDRESS;
address++) {
    memory_setWriteCallback(memory, address, &cpuMemory_ramMirror_writer);
    memory_setReadCallback(memory, address, &cpuMemory_ramMirror_reader);
  }
  // setup the ppu register redirectors
  memory_setReadCallback(memory, CPU_PPU_CONTROL_REGISTER_ADDRESS,
&cpuMemory_ppuControlRegister_reader);
  memory_setWriteCallback(memory, CPU_PPU_CONTROL_REGISTER_ADDRESS,
&cpuMemory_ppuControlRegister_writer);
  memory_setReadCallback(memory, CPU_PPU_MASK_REGISTER_ADDRESS,
&cpuMemory_ppuMaskRegister_reader);
  memory_setWriteCallback(memory, CPU_PPU_MASK_REGISTER_ADDRESS,
&cpuMemory_ppuMaskRegister_writer);
  memory_setReadCallback(memory, CPU_PPU_STATUS_REGISTER_ADDRESS,
&cpuMemory_ppuStatusRegister_reader);
  memory_setWriteCallback(memory, CPU_PPU_STATUS_REGISTER_ADDRESS,
&cpuMemory_ppuStatusRegister_writer);
  memory_setReadCallback(memory, CPU_PPU_SPRITE_ADDRESS_REGISTER_ADDRESS,
&cpuMemory_ppuSpriteAddressRegister_reader);
  memory_setWriteCallback(memory, CPU_PPU_SPRITE_ADDRESS_REGISTER_ADDRESS,
&cpuMemory_ppuSpriteAddressRegister_writer);
  memory_setReadCallback(memory, CPU_PPU_SPRITE_DATA_REGISTER_ADDRESS,
&cpuMemory_ppuSpriteDataRegister_reader);
  memory_setWriteCallback(memory, CPU_PPU_SPRITE_DATA_REGISTER_ADDRESS,
&cpuMemory_ppuSpriteDataRegister_writer);
  memory_setReadCallback(memory, CPU_PPU_SCROLL_REGISTER_ADDRESS,
&cpuMemory_ppuScrollRegister_reader);
  memory_setWriteCallback(memory, CPU_PPU_SCROLL_REGISTER_ADDRESS,
&cpuMemory_ppuScrollRegister_writer);
  memory_setReadCallback(memory, CPU_PPUMEMORY_ADDRESS_REGISTER_ADDRESS,
&cpuMemory_ppuMemoryAddressRegister_reader);
  memory_setWriteCallback(memory, CPU_PPUMEMORY_ADDRESS_REGISTER_ADDRESS,
&cpuMemory_ppuMemoryAddressRegister_writer);
  memory_setReadCallback(memory, CPU_PPUMEMORY_DATA_REGISTER_ADDRESS,
&cpuMemory_ppuMemoryDataRegister_reader);
  memory_setWriteCallback(memory, CPU_PPUMEMORY_DATA_REGISTER_ADDRESS,
&cpuMemory_ppuMemoryDataRegister_writer);
  // now setup their mirrors
  for (address=CPU_PPU_MIRROR_FIRST_ADDRESS; address <= CPU_PPU_MIRROR_LAST_ADDRESS;
address++) {
    memory_setWriteCallback(memory, address, &cpuMemory_ppuMirror_writer);
    memory_setReadCallback(memory, address, &cpuMemory_ppuMirror_reader);
  }
  memory_setReadCallback(memory, CPU_SPRITE_DMA_REGISTER_ADDRESS,
&cpuMemory_spriteDMARegister_reader);
  memory_setWriteCallback(memory, CPU_SPRITE_DMA_REGISTER_ADDRESS,
&cpuMemory_spriteDMARegister_writer);
  memory_setReadCallback(memory, CPU_JOYPAD_0_ADDRESS, &cpuMemory_joypad_reader);
  memory_setWriteCallback(memory, CPU_JOYPAD_0_ADDRESS, &cpuMemory_joypad_writer);
  memory_setReadCallback(memory, CPU_JOYPAD_1_ADDRESS, &cpuMemory_joypad_reader);
  memory_setWriteCallback(memory, CPU_JOYPAD_1_ADDRESS, &cpuMemory_joypad_writer);
  return memory;
}