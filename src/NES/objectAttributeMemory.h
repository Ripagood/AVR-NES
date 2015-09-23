#include "globals.h"
#include "memory_type.h"
#define OAM_FIRST_ADDRESS 0
#define OAM_LAST_ADDRESS 255
#define OAM_NUM_ADDRESSES (OAM_LAST_ADDRESS + 1)
#define OAM_BYTES_PER_SPRITE 4
#define OAM_NUMBER_OF_SPRITES (OAM_NUM_ADDRESSES / OAM_BYTES_PER_SPRITE)
#define OAM_BYTES_PER_SPRITE 4
#define OAM_Y_BYTE_OFFSET 0
#define OAM_TILE_BYTE_OFFSET 1
#define OAM_ATTRIBUTES_BYTE_OFFSET 2
#define OAM_X_BYTE_OFFSET 3
#define MASK_OAM_FLIP_HORIZONTAL_ON MASK_BIT6
#define MASK_OAM_FLIP_VERTICAL_ON MASK_BIT7
#define MASK_OAM_BEHIND_BACKGROUND_ON MASK_BIT5
#define MASK_OAM_BANK_NUMBER_ON MASK_BIT0
Memory objectAttributeMemory_init(void);
// byte 0
Byte objectAttributeMemory_getY(Memory memory, int spriteIndex);
// byte 1
Byte objectAttributeMemory_getTileNumber(Memory memory, int spriteIndex, Boolean
using8x16);
// only valid for 8x16 sprites
Byte objectAttributeMemory_getBankNumber(Memory memory, int spriteIndex);
// byte 2
Byte objectAttributeMemory_getAttributes(Memory memory, int spriteIndex);
Byte objectAttributeMemory_getPalette(Memory memory, int spriteIndex);
Boolean objectAttributeMemory_isBehindBackground(Memory memory, int spriteIndex);
Boolean objectAttributeMemory_isFlippedHorizontal(Memory memory, int spriteIndex);
Boolean objectAttributeMemory_isFlippedVertical(Memory memory, int spriteIndex);
// byte 3
Byte objectAttributeMemory_getX(Memory memory, int spriteIndex);