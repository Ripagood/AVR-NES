#include "ppu_type.h"
#include "nes_type.h"
#include "globals.h"


#define PPU_CONTROL_ADDRESS_VERTICAL_INCREMENT 32
#define PPU_CONTROL_ADDRESS_HORIZONTAL_INCREMENT 1
#define PPU_CYCLES_PER_SCANLINE 341
#define PPU_SCANLINES_PER_FRAME 262
#define PPU_NAMETABLE_BYTES_BEFORE_ATTRIBUTE_TABLE (PPU_BACKGROUND_TILES_PER_ROW * PPU_BACKGROUND_TILES_PER_COLUMN)
#define PPU_HORIZONTAL_TILES_PER_ATTRIBUTE_BYTE 4
#define PPU_VERTICAL_TILES_PER_ATTRIBUTE_BYTE 4
#define PPU_ATTRIBUTE_BYTES_PER_ROW 8
#define PPU_ATTRIBUTE_BYTES_PER_COLUMN 8
#define PPU_TILES_PER_ATTRIBUTE_BYTE 16
#define PPU_TILES_PER_ATTRIBUTE_BYTE_ROW (PPU_TILES_PER_ATTIRBUTE_BYTE * PPU_ATTRIBUTE_BYTES_PER_ROW)
#define PPU_ATTIRBUTE_TABLE_OFFSET (PPU_BACKGROUND_TILES_PER_ROW * PPU_BACKGROUND_TILES_PER_COLUMN)
#define PPU_PATTERN_BYTES_PER_TILE 16
#define PPU_PATTERN_TILE_SECOND_BYTE_OFFSET 8
#define PPU_BACKGROUND_TILES_PER_ROW 32
#define PPU_BACKGROUND_TILES_PER_COLUMN 30
#define PPU_HORIZONTAL_PIXELS_PER_TILE 8
#define PPU_VERTICAL_PIXELS_PER_TILE 8
#define VALIDATE_NAMETABLE_ADDRESS(X) { assert(X >= PPU_NAME_TABLE_0_FIRST_ADDRESS); assert(X <= PPU_NAME_TABLE_MIRROR_LAST_ADDRESS); }
#define PPU_WASTED_VBLANK_SCANLINES 20
#define PPU_WASTED_PREFETCH_SCANLINES 1
// Vertical blank (0: not in VBLANK; 1: in VBLANK)
#define MASK_STATUS_VBLANK_ON MASK_BIT7
#define MASK_STATUS_VBLANK_OFF (~MASK_BIT7)
//     6       a primary object pixel has collided with a playfield pixel in the last fram
// bit6: sprite collision hit
// Set when a nonzero pixel of sprite 0 'hits'
// a nonzero background pixel. Used for raster timing.
#define MASK_STATUS_SPRITE_COLLISION_HIT_ON MASK_BIT6
#define MASK_STATUS_SPRITE_COLLISION_HIT_OFF (~MASK_BIT6)
// 2   5       more than 8 objects on a single scanline have been detected in the last frame
// bit5: sprite overflow
// Sprite overflow. The PPU can handle only eight sprites on one
// scanline and sets this bit if it starts dropping sprites.
// Normally, this triggers when there are 9 sprites on a scanline,
// but the actual behavior is significantly more complicated.
#define MASK_STATUS_SPRITE_OVERFLOW_ON MASK_BIT5
#define MASK_STATUS_SPRITE_OVERFLOW_OFF (~MASK_BIT5)
// bit4: sprite visibility
// Enable sprite rendering
#define MASK_MASK_SPRITE_VISIBILITY_ON MASK_BIT4
#define MASK_MASK_SPRITE_VISIBILITY_OFF (~MASK_BIT4)
// bit3: background visibility
// Enable background rendering
#define MASK_MASK_BACKGROUND_VISIBILITY_ON MASK_BIT3
#define MASK_MASK_BACKGROUND_VISIBILITY_OFF (~MASK_BIT3)
//     2       left side screen column (8 pixels wide) object clipping (when 0).
// bit2: sprite clipping
// Disable sprite clipping in leftmost 8 pixels of screen
#define MASK_MASK_SPRITE_CLIPPING_ON MASK_BIT2
#define MASK_MASK_SPRITE_CLIPPING_OFF (~MASK_BIT2)
// 1   left side screen column (8 pixels wide) playfield clipping (when 0).
// bit1: background clipping
// Disable background clipping in leftmost 8 pixels of screen
#define MASK_MASK_BACKGROUND_CLIPPING_ON MASK_BIT1
#define MASK_MASK_BACKGROUND_CLIPPING_OFF (~MASK_BIT1)
// bit0: display type
// Grayscale (0: normal color; 1: AND all palette entries
// with 0x30, effectively producing a monochrome display;
// note that colour emphasis STILL works when this is on!)
#define MASK_MASK_DISPLAY_TYPE_ON MASK_BIT0
#define MASK_MASK_DISPLAY_TYPE_OFF (~MASK_BIT0)
#define MASK_CONTROL_NMI_ON MASK_BIT7
#define MASK_CONTROL_NMI_OFF (~MASK_BIT7)
// Sprite size (0: 8x8; 1: 8x16)
//     5       8/16 scanline objects (0/1)
#define MASK_CONTROL_SPRITE_SIZE_ON MASK_BIT5
#define MASK_CONTROL_SPRITE_SIZE_OFF (~MASK_BIT5)
// Background pattern table address (0: $0000; 1: $1000)
#define MASK_CONTROL_BACKGROUND_TILE_TABLE_ON MASK_BIT4
#define MASK_CONTROL_BACKGROUND_TILE_TABLE_OFF (~MASK_BIT4)
// Sprite pattern table address for 8x8 sprites (0: $0000; 1: $1000)
//     3       object pattern table selection (if bit 5 = 0)
#define MASK_CONTROL_SPRITE_TILE_TABLE_ON MASK_BIT3
#define MASK_CONTROL_SPRITE_TILE_TABLE_OFF (~MASK_BIT3)
#define MASK_CONTROL_PPU_ADDRESS_INCREMENT_ON MASK_BIT2
#define MASK_CONTROL_PPU_ADDRESS_INCREMENT_OFF (~MASK_BIT2)
//     7       B (to be documented)
// bit7: colour emphasis?
#define MASK_MASK_INTENSIFY_BLUE_ON MASK_BIT7
#define MASK_MASK_INTENSIFY_BLUE_OFF (~MASK_BIT7)
//      6      G (to be documented)
// bit6: colour emphasis?
#define MASK_MASK_INTENSIFY_GREEN_ON MASK_BIT6
#define MASK_MASK_INTENSIFY_GREEN_OFF (~MASK_BIT6)
// MASK_MASK
//      5      R (to be documented)
// bit5: colour emphasis?
#define MASK_MASK_INTENSIFY_RED_ON MASK_BIT5
#define MASK_MASK_INTENSIFY_RED_OFF (~MASK_BIT5) // (MASK_ALL_ON ^ MASK_BIT5)
// MASK_CONTROL
//  (0 = $2000; 1 = $2400; 2 = $2800; 3 = $2C00)
//      1      Y scroll name table selection.
// bit1 base name table?
#define MASK_CONTROL_VERTICAL_SCROLL_NAME_TABLE_ON MASK_BIT1
#define MASK_CONTROL_VERTICAL_SCROLL_NAME_TABLE_OFF (~MASK_BIT1) // (MASK_ALL_ON ^ MASK_BIT1)
// bit0 base name table?
// 0    0      X scroll name table selection.
#define MASK_CONTROL_HORIZONTAL_SCROLL_NAME_TABLE_ON MASK_BIT0
#define MASK_CONTROL_HORIZONTAL_SCROLL_NAME_TABLE_OFF (~MASK_BIT0) // (MASK_ALL_ON ^ MASK_BIT0)
#define PPU_SCREEN_WIDTH_IN_PIXELS 256
#define PPU_NUM_SYSTEM_COLOURS 64
PPU ppu_init(void);
void ppu_destroy(PPU ppu);
void ppu_step(NES nes);
void ppu_test(void);
Byte ppu_getControlRegister(PPU ppu);
Byte ppu_getMaskRegister(PPU ppu);
Byte ppu_getStatusRegister(PPU ppu);
Byte ppu_getSpriteAddressRegister(PPU ppu);
Byte ppu_getScrollRegister(PPU ppu);
Byte ppu_getPPUMemoryAddressRegister(PPU ppu);
void ppu_setControlRegister(PPU ppu, Byte controlRegister);
void ppu_setMaskRegister(PPU ppu, Byte maskRegister);
void ppu_setStatusRegister(PPU ppu, Byte statusRegister);
void ppu_setSpriteAddressRegister(PPU ppu, Byte spriteAddressRegister);
void ppu_setScrollRegister(PPU ppu, Byte scrollRegister);
void ppu_setPPUMemoryAddressRegister(PPU ppu, Byte ppuMemoryAddressRegister);
void ppu_setPPUMemoryDataRegister(NES nes, Byte ppuMemoryDataRegister);
Byte ppu_getPPUMemoryDataRegister(NES nes);
Byte ppu_getSpriteDataRegister(NES nes);
void ppu_setSpriteDataRegister(NES nes, Byte spriteDataRegister);