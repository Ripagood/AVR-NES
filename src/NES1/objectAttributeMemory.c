#include  <stdio.h>
#include  <stdlib.h>
#include  <assert.h>
#include  "memory.h"
#include  "objectAttributeMemory.h"
#include  "globals.h"
/*
       There is an independent area of memory known as sprite ram, which is
256 bytes in size. This memory stores 4 bytes of information for 64 sprites.
These 4 bytes contain the x and y location of the sprite on the screen, the
upper two color bits, the tile index number (pattern table tile of the sprite),
and information about flipping (horizontal and vertical), and priority
(behind/on top of background). Sprite ram can be accessed byte-by-byte through
the NES registers, or also can be loaded via DMA transfer through another
register.
*/
// assumption: not using callbacks within sprite memory?
Memory objectAttributeMemory_init(void) {
  Memory memory = memory_init(OAM_NUM_ADDRESSES);
  //assert(memory != NULL);
  return memory;
}
// byte 0
// 0    -       scanline coordinate minus one of object's top pixel row.
Byte objectAttributeMemory_getY(Memory memory, int spriteIndex) {
  //assert(memory != NULL);
  Byte address = (OAM_BYTES_PER_SPRITE * spriteIndex) + OAM_Y_BYTE_OFFSET;
  return memory_read_direct(memory, address);
}
// byte 1
// only valid for 8x16 sprites
Byte objectAttributeMemory_getBankNumber(Memory memory, int spriteIndex) {
  //assert(memory != NULL);
  Byte address = (OAM_BYTES_PER_SPRITE * spriteIndex) + OAM_TILE_BYTE_OFFSET;
  Byte data = memory_read_direct(memory, address);
  if ( (data & MASK_OAM_BANK_NUMBER_ON) == MASK_OAM_BANK_NUMBER_ON) {
    return 1;
  } else {
    return 0;
  }
}
Byte objectAttributeMemory_getTileNumber(Memory memory, int spriteIndex, Bool
using8x16) {
  //assert(memory != NULL);
  Byte address = (OAM_BYTES_PER_SPRITE * spriteIndex) + OAM_TILE_BYTE_OFFSET;
  Byte data = memory_read_direct(memory, address);
  if (using8x16 == FALSE) {
    return data;
  } else {
    // lose the 0th bit
    return (data >> 1) << 1;
  }
}
// byte 2
Byte objectAttributeMemory_getAttributes(Memory memory, int spriteIndex) {
  //assert(memory != NULL);
  Byte address = (OAM_BYTES_PER_SPRITE * spriteIndex) + OAM_ATTRIBUTES_BYTE_OFFSET;
  return memory_read_direct(memory, address);
}
// Palette (4 to 7) of sprite
Byte objectAttributeMemory_getPalette(Memory memory, int spriteIndex) {
  //assert(memory != NULL);
  Byte attributes = objectAttributeMemory_getAttributes(memory, spriteIndex);
  // lose the 765432 bits
  attributes = (attributes << 6) >> 4;
  return attributes; // + 4;
}
Bool objectAttributeMemory_isBehindBackground(Memory memory, int spriteIndex) {
  //assert(memory != NULL);
  Byte attributes = objectAttributeMemory_getAttributes(memory, spriteIndex);
  if ( (attributes & MASK_OAM_BEHIND_BACKGROUND_ON) == MASK_OAM_BEHIND_BACKGROUND_ON) {
    return TRUE;
  } else {
    return FALSE;
  }
}
Bool objectAttributeMemory_isFlippedHorizontal(Memory memory, int spriteIndex) {
  //assert(memory != NULL);
  Byte attributes = objectAttributeMemory_getAttributes(memory, spriteIndex);
  if ( (attributes & MASK_OAM_FLIP_HORIZONTAL_ON) == MASK_OAM_FLIP_HORIZONTAL_ON) {
    return TRUE;
  } else {
    return FALSE;
  }
}
Bool objectAttributeMemory_isFlippedVertical(Memory memory, int spriteIndex) {
  //assert(memory != NULL);
  Byte attributes = objectAttributeMemory_getAttributes(memory, spriteIndex);
  if ( (attributes & MASK_OAM_FLIP_VERTICAL_ON) == MASK_OAM_FLIP_VERTICAL_ON) {
    return TRUE;
  } else {
    return FALSE;
  }
}
// byte 3
// 3    -      scanline pixel coordite of most left-hand side of object.
Byte objectAttributeMemory_getX(Memory memory, int spriteIndex) {
  //assert(memory != NULL);
  Byte address = (OAM_BYTES_PER_SPRITE * spriteIndex) + OAM_X_BYTE_OFFSET;
  return memory_read_direct(memory, address);
}