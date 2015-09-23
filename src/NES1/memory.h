#include "memory_type.h"
#include "globals.h"
#include "nes_type.h"
Memory memory_init(uint32_t numAddresses);
void memory_destroy(Memory memory);
void memory_write_direct(Memory memory, Address address, Byte data);
Byte memory_read_direct(Memory memory, Address address);
void memory_print(NES nes, Memory memory);
int memory_getNumAddresses(Memory memory);
typedef Byte (*NES_ReadCallback)(NES nes, Address address);
typedef void (*NES_WriteCallback)(NES nes, Address address, Byte data);
void memory_setWriteCallback(Memory memory, Address address, NES_WriteCallback
writeCallback);
void memory_setReadCallback(Memory memory, Address address, NES_ReadCallback
readCallback);
void memory_write_callback(NES nes, Memory memory, Address address, Byte data);
Byte memory_read_callback(NES nes, Memory memory, Address address);