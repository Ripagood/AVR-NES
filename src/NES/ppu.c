#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "globals.h"
#include "ppu.h"
#include "nes.h"
#include "ppuMemory.h"
#include "colour.h"
#include "objectAttributeMemory.h"
// palette adapted from http://nesdev.parodius.com/NESTechFAQ.htm
struct colour systemPalette[PPU_NUM_SYSTEM_COLOURS] = {
  {0x75, 0x75, 0x75},
  {0x27, 0x1B, 0x8F},
  {0x00, 0x00, 0xAB},
  {0x47, 0x00, 0x9F},
  {0x8F, 0x00, 0x77},
  {0xAB, 0x00, 0x13},
  {0xA7, 0x00, 0x00},
  {0x7F, 0x0B, 0x00},
  {0x43, 0x2F, 0x00},
  {0x00, 0x47, 0x00},
  {0x00, 0x51, 0x00},
  {0x00, 0x3F, 0x17},
  {0x1B, 0x3F, 0x5F},
  {0x00, 0x00, 0x00},
  {0x00, 0x00, 0x00},
  {0x00, 0x00, 0x00},
  {0xBC, 0xBC, 0xBC},
  {0x00, 0x73, 0xEF},
  {0x23, 0x3B, 0xEF},
  {0x83, 0x00, 0xF3},
  {0xBF, 0x00, 0xBF},
  {0xE7, 0x00, 0x5B},
  {0xDB, 0x2B, 0x00},
  {0xCB, 0x4F, 0x0F},
  {0x8B, 0x73, 0x00},
  {0x00, 0x97, 0x00},
  {0x00, 0xAB, 0x00},
  {0x00, 0x93, 0x3B},
  {0x00, 0x83, 0x8B},
  {0x00, 0x00, 0x00},
  {0x00, 0x00, 0x00},
  {0x00, 0x00, 0x00},
  {0xFF, 0xFF, 0xFF},
  {0x3F, 0xBF, 0xFF},
  {0x5F, 0x97, 0xFF},
  {0xA7, 0x8B, 0xFD},
  {0xF7, 0x7B, 0xFF},
  {0xFF, 0x77, 0xB7},
  {0xFF, 0x77, 0x63},
  {0xFF, 0x9B, 0x3B},
  {0xF3, 0xBF, 0x3F},
  {0x83, 0xD3, 0x13},
  {0x4F, 0xDF, 0x4B},
  {0x58, 0xF8, 0x98},
  {0x00, 0xEB, 0xDB},
  {0x00, 0x00, 0x00},
  {0x00, 0x00, 0x00},
  {0x00, 0x00, 0x00},
  {0xFF, 0xFF, 0xFF},
  {0xAB, 0xE7, 0xFF},
  {0xC7, 0xD7, 0xFF},
  {0xD7, 0xCB, 0xFF},
  {0xFF, 0xC7, 0xFF},
  {0xFF, 0xC7, 0xDB},
  {0xFF, 0xBF, 0xB3},
  {0xFF, 0xDB, 0xAB},
  {0xFF, 0xE7, 0xA3},
  {0xE3, 0xFF, 0xA3},
  {0xAB, 0xF3, 0xBF},
  {0xB3, 0xFF, 0xCF},
  {0x9F, 0xFF, 0xF3},
  {0x00, 0x00, 0x00},
  {0x00, 0x00, 0x00},
  {0x00, 0x00, 0x00}
};
Byte
attributeTableLookup[PPU_VERTICAL_TILES_PER_ATTRIBUTE_BYTE][PPU_HORIZONTAL_TILES_PER_ATT
RIBUTE_BYTE] = {
  {0x0, 0x1, 0x4, 0x5},
  {0x2, 0x3, 0x6, 0x7},
  {0x8, 0x9, 0xC, 0xD},
  {0xA, 0xB, 0xE, 0xF}
};
struct ppu {
  // the 8 exposed registers
  Byte controlRegister;
  Byte maskRegister;
  Byte statusRegister;
  Byte spriteAddressRegister;
  Byte spriteDataRegister;    // unused?
  Byte scrollRegister;
  Byte ppuMemoryAddressRegister;
  Byte ppuMemoryDataRegister;
  // book-keeping
  Byte ppuMemoryReadBuffer;
  Byte verticalScroll;       // Vertical offsets range from -16 to 239
                             // (values greater than 240 are treated as negative, though they actually render their tile data from the attribute table).
  Byte horizontalScroll;     // Horizontal offsets range from 0 to 255;
  Boolean hasPartial;
  Byte ppuAddressLow;
  Byte ppuAddressHigh;
  int currentCycle;
  int currentScanline;
  int currentScanlineCycle;
  int currentFrame;
  Byte spriteColoursForScanline[PPU_SCREEN_WIDTH_IN_PIXELS];
  Boolean spriteColoursForScanlineSet[PPU_SCREEN_WIDTH_IN_PIXELS];
  Boolean spriteColoursForScanlineIsBehindBackground[PPU_SCREEN_WIDTH_IN_PIXELS];
};
static void ppu_resetSpriteColoursForScanline(PPU ppu) {
  assert(ppu != NULL);
  int i;
  for (i=0; i < PPU_SCREEN_WIDTH_IN_PIXELS; i++) {
    ppu->spriteColoursForScanline[i] = 0;
    ppu->spriteColoursForScanlineSet[i] = FALSE;
    ppu->spriteColoursForScanlineIsBehindBackground[i] = FALSE;
  }
}
PPU ppu_init(void) {
  PPU ppu = (PPU) malloc(sizeof(struct ppu));
  assert(ppu != NULL);
  ppu->controlRegister = 0;
  ppu->maskRegister = 0;
  ppu->statusRegister = 0;
  ppu->spriteAddressRegister = 0;
  ppu->spriteDataRegister = 0;    // unused?
  ppu->ppuAddressLow = 0;
  ppu->ppuAddressHigh = 0;
  ppu->ppuMemoryAddressRegister = 0;
  ppu->ppuMemoryDataRegister = 0;
  ppu->hasPartial = FALSE;
  ppu->verticalScroll = 0;
  ppu->horizontalScroll = 0;
  ppu->scrollRegister = 0;
  ppu->currentCycle = 0;
  ppu->currentScanline = 0;
  ppu->currentScanlineCycle = 0;
  ppu_resetSpriteColoursForScanline(ppu);
  ppu->ppuMemoryReadBuffer = 0;
  ppu->currentFrame = 0;
  return ppu;
}
void ppu_destroy(PPU ppu) {
  assert(ppu != NULL);
  free(ppu);
}
static Boolean ppu_getControlHorizontalScrollNametable(PPU ppu) {
  assert(ppu != NULL);
  if ( (ppu->controlRegister & MASK_CONTROL_HORIZONTAL_SCROLL_NAME_TABLE_ON) ==
MASK_CONTROL_HORIZONTAL_SCROLL_NAME_TABLE_ON) {
    return TRUE;
  } else {
    return FALSE;
  }
}
static void ppu_setControlHorizontalScrollNametable(PPU ppu, Boolean state) {
  assert(ppu != NULL);
  if (state == TRUE) {
    ppu->controlRegister |= MASK_CONTROL_HORIZONTAL_SCROLL_NAME_TABLE_ON;
  } else if (state == FALSE) {
    ppu->controlRegister &= MASK_CONTROL_HORIZONTAL_SCROLL_NAME_TABLE_OFF;
  }
}
static Boolean ppu_getControlVerticalScrollNametable(PPU ppu) {
  assert(ppu != NULL);
  if ( (ppu->controlRegister & MASK_CONTROL_VERTICAL_SCROLL_NAME_TABLE_ON) ==
MASK_CONTROL_VERTICAL_SCROLL_NAME_TABLE_ON) {
    return TRUE;
  } else {
    return FALSE;
  }
}
static void ppu_setControlVerticalScrollNametable(PPU ppu, Boolean state) {
  assert(ppu != NULL);
  if (state == TRUE) {
    ppu->controlRegister |= MASK_CONTROL_VERTICAL_SCROLL_NAME_TABLE_ON;
  } else if (state == FALSE) {
    ppu->controlRegister &= MASK_CONTROL_VERTICAL_SCROLL_NAME_TABLE_OFF;
  }
}
static Address ppu_getCurrentBaseNametableAddress(PPU ppu) {
  assert(ppu != NULL);
  Address address = PPU_NAME_TABLE_0_FIRST_ADDRESS;
  if (ppu_getControlHorizontalScrollNametable(ppu) == TRUE) {
    address += PPU_NAME_TABLE_SIZE;
  }
  if (ppu_getControlVerticalScrollNametable(ppu) == TRUE) {
    address += PPU_NAME_TABLE_SIZE;
    address += PPU_NAME_TABLE_SIZE;
  }
  return address;
}
static Boolean cpu_getMaskIntensifyRed(PPU ppu) {
  assert(ppu != NULL);
  if ((ppu->maskRegister & MASK_MASK_INTENSIFY_RED_ON) == MASK_MASK_INTENSIFY_RED_ON) {
    return TRUE;
  } else {
    return FALSE;
  }
}
static void cpu_setMarkIntensifyRed(PPU ppu, Boolean state) {
  assert(ppu != NULL);
  if (state == TRUE) {
    ppu->maskRegister |= MASK_MASK_INTENSIFY_RED_ON;
  } else if (state == FALSE) {
    ppu->maskRegister &= MASK_MASK_INTENSIFY_RED_OFF;
  }
}
static Boolean cpu_getMaskIntensifyGren(PPU ppu) {
  assert(ppu != NULL);
  if ((ppu->maskRegister & MASK_MASK_INTENSIFY_GREEN_ON) ==
MASK_MASK_INTENSIFY_GREEN_ON) {
    return TRUE;
  } else {
    return FALSE;
  }
}
static void cpu_setMarkIntensifyGreen(PPU ppu, Boolean state) {
  assert(ppu != NULL);
  if (state == TRUE) {
    ppu->maskRegister |= MASK_MASK_INTENSIFY_GREEN_ON;
  } else if (state == FALSE) {
    ppu->maskRegister &= MASK_MASK_INTENSIFY_GREEN_OFF;
  }
}
static Boolean cpu_getMaskIntensifyBlue(PPU ppu) {
  assert(ppu != NULL);
  if ((ppu->maskRegister & MASK_MASK_INTENSIFY_BLUE_ON) == MASK_MASK_INTENSIFY_BLUE_ON)
{
    return TRUE;
  } else {
    return FALSE;
  }
}
static void cpu_setMarkIntensifyBlue(PPU ppu, Boolean state) {
  assert(ppu != NULL);
  if (state == TRUE) {
    ppu->maskRegister |= MASK_MASK_INTENSIFY_BLUE_ON;
  } else if (state == FALSE) {
    ppu->maskRegister &= MASK_MASK_INTENSIFY_BLUE_OFF;
  }
}
static void ppu_setControlPPUAddressIncrement(PPU ppu, Boolean state) {
  assert(ppu != NULL);
  if (state == TRUE) {
    ppu->controlRegister |= MASK_CONTROL_PPU_ADDRESS_INCREMENT_ON;
  } else if (state == FALSE) {
    ppu->controlRegister &= MASK_CONTROL_PPU_ADDRESS_INCREMENT_OFF;
  }
}
static Boolean ppu_getControlPPUAddressIncrement(PPU ppu) {
  assert(ppu != NULL);
  if ((ppu->controlRegister & MASK_CONTROL_PPU_ADDRESS_INCREMENT_ON) ==
MASK_CONTROL_PPU_ADDRESS_INCREMENT_ON) {
    return TRUE;
  } else {
    return FALSE;
  }
}
static void ppu_setControlSpriteTileTable(PPU ppu, Boolean state) {
  assert(ppu != NULL);
  if (state == TRUE) {
    ppu->controlRegister |= MASK_CONTROL_SPRITE_TILE_TABLE_ON;
  } else if (state == FALSE) {
    ppu->controlRegister &= MASK_CONTROL_SPRITE_TILE_TABLE_OFF;
  }
}
static Boolean ppu_getControlSpriteTileTable(PPU ppu) {
  assert(ppu != NULL);
  if ((ppu->controlRegister & MASK_CONTROL_SPRITE_TILE_TABLE_ON) ==
MASK_CONTROL_SPRITE_TILE_TABLE_ON) {
    return TRUE;
  } else {
    return FALSE;
  }
}
static void ppu_setControlBackgroundTileTable(PPU ppu, Boolean state) {
  assert(ppu != NULL);
  if (state == TRUE) {
    ppu->controlRegister |= MASK_CONTROL_BACKGROUND_TILE_TABLE_ON;
  } else if (state == FALSE) {
    ppu->controlRegister &= MASK_CONTROL_BACKGROUND_TILE_TABLE_OFF;
  }
}
static Boolean ppu_getControlBackgroundTileTable(PPU ppu) {
  assert(ppu != NULL);
  if ((ppu->controlRegister & MASK_CONTROL_BACKGROUND_TILE_TABLE_ON) ==
MASK_CONTROL_BACKGROUND_TILE_TABLE_ON) {
    return TRUE;
  } else {
    return FALSE;
  }
}
static void ppu_setControlSpriteSize(PPU ppu, Boolean state) {
  assert(ppu != NULL);
  if (state == TRUE) {
    ppu->controlRegister |= MASK_CONTROL_SPRITE_SIZE_ON;
  } else if (state == FALSE) {
    ppu->controlRegister &= MASK_CONTROL_SPRITE_SIZE_OFF;
  }
}
static Boolean ppu_getControlSpriteSize(PPU ppu) {
  assert(ppu != NULL);
  if ((ppu->controlRegister & MASK_CONTROL_SPRITE_SIZE_ON) ==
MASK_CONTROL_SPRITE_SIZE_ON) {
    return TRUE;
  } else {
    return FALSE;
  }
}
static void ppu_setControlNMI(PPU ppu, Boolean state) {
  assert(ppu != NULL);
  if (state == TRUE) {
    ppu->controlRegister |= MASK_CONTROL_NMI_ON;
  } else if (state == FALSE) {
    ppu->controlRegister &= MASK_CONTROL_NMI_OFF;
  }
}
static Boolean ppu_getControlNMI(PPU ppu) {
  assert(ppu != NULL);
  if ((ppu->controlRegister & MASK_CONTROL_NMI_ON) == MASK_CONTROL_NMI_ON) {
    return TRUE;
  } else {
    return FALSE;
  }
}
static void ppu_setMaskDisplayType(PPU ppu, Boolean state) {
  assert(ppu != NULL);
  if (state == TRUE) {
    ppu->maskRegister |= MASK_MASK_DISPLAY_TYPE_ON;
  } else if (state == FALSE) {
    ppu->maskRegister &= MASK_MASK_DISPLAY_TYPE_OFF;
  }
}
static Boolean ppu_getMaskDisplayType(PPU ppu) {
  assert(ppu != NULL);
  if ((ppu->maskRegister & MASK_MASK_DISPLAY_TYPE_ON) == MASK_MASK_DISPLAY_TYPE_ON) {
    return TRUE;
  } else {
    return FALSE;
  }
}
static void ppu_setMaskBackgroundClipping(PPU ppu, Boolean state) {
  assert(ppu != NULL);
  if (state == TRUE) {
    ppu->maskRegister |= MASK_MASK_BACKGROUND_CLIPPING_ON;
  } else if (state == FALSE) {
    ppu->maskRegister &= MASK_MASK_BACKGROUND_CLIPPING_OFF;
  }
}
static Boolean ppu_getMaskBackgroundClipping(PPU ppu) {
  assert(ppu != NULL);
  if ((ppu->maskRegister & MASK_MASK_BACKGROUND_CLIPPING_ON) ==
MASK_MASK_BACKGROUND_CLIPPING_ON) {
    return TRUE;
  } else {
    return FALSE;
  }
}
static void ppu_setMaskSpriteClipping(PPU ppu, Boolean state) {
  assert(ppu != NULL);
  if (state == TRUE) {
    ppu->maskRegister |= MASK_MASK_SPRITE_CLIPPING_ON;
  } else if (state == FALSE) {
    ppu->maskRegister &= MASK_MASK_SPRITE_CLIPPING_OFF;
  }
}
static Boolean ppu_getMaskSpriteClipping(PPU ppu) {
  assert(ppu != NULL);
  if ((ppu->maskRegister & MASK_MASK_SPRITE_CLIPPING_ON) ==
MASK_MASK_SPRITE_CLIPPING_ON) {
    return TRUE;
  } else {
    return FALSE;
  }
}
static void ppu_setMaskBackgroundVisiblity(PPU ppu, Boolean state) {
  assert(ppu != NULL);
  if (state == TRUE) {
    ppu->maskRegister |= MASK_MASK_BACKGROUND_VISIBILITY_ON;
  } else if (state == FALSE) {
    ppu->maskRegister &= MASK_MASK_BACKGROUND_VISIBILITY_OFF;
  }
}
static Boolean ppu_getMaskBackgroundVisibility (PPU ppu) {
  assert(ppu != NULL);
  if ((ppu->maskRegister & MASK_MASK_BACKGROUND_VISIBILITY_ON) ==
MASK_MASK_BACKGROUND_VISIBILITY_ON) {
    return TRUE;
  } else {
    return FALSE;
  }
}
static void ppu_setMaskSpriteVisibility(PPU ppu, Boolean state) {
  assert(ppu != NULL);
  if (state == TRUE) {
    ppu->maskRegister |= MASK_MASK_SPRITE_VISIBILITY_ON;
  } else if (state == FALSE) {
    ppu->maskRegister &= MASK_MASK_SPRITE_VISIBILITY_OFF;
  }
}
static Boolean ppu_getMaskSpriteVisibility(PPU ppu) {
  assert(ppu != NULL);
  if ((ppu->maskRegister & MASK_MASK_SPRITE_VISIBILITY_ON) ==
MASK_MASK_SPRITE_VISIBILITY_ON) {
    return TRUE;
  } else {
    return FALSE;
  }
}
static void ppu_setStatusSpriteOverflow(PPU ppu, Boolean state) {
  assert(ppu != NULL);
  if (state == TRUE) {
    ppu->statusRegister |= MASK_STATUS_SPRITE_OVERFLOW_ON;
  } else if (state == FALSE) {
    ppu->statusRegister &= MASK_STATUS_SPRITE_OVERFLOW_OFF;
  }
}
static Boolean ppu_getStatusSpriteOverflow(PPU ppu) {
  assert(ppu != NULL);
  if ((ppu->statusRegister & MASK_STATUS_SPRITE_OVERFLOW_ON) ==
MASK_STATUS_SPRITE_OVERFLOW_ON) {
    return TRUE;
  } else {
    return FALSE;
  }
}
static void ppu_setStatusSpriteCollisionHit(PPU ppu, Boolean state) {
  assert(ppu != NULL);
  if (state == TRUE) {
    ppu->statusRegister |= MASK_STATUS_SPRITE_COLLISION_HIT_ON;
  } else if (state == FALSE) {
    ppu->statusRegister &= MASK_STATUS_SPRITE_COLLISION_HIT_OFF;
  }
}
static Boolean ppu_getStatusSpriteCollisionHit(PPU ppu) {
  assert(ppu != NULL);
  if ((ppu->statusRegister & MASK_STATUS_SPRITE_COLLISION_HIT_ON) ==
MASK_STATUS_SPRITE_COLLISION_HIT_ON) {
    return TRUE;
  } else {
    return FALSE;
  }
}
static void ppu_setStatusVerticalBlank(PPU ppu, Boolean state) {
  assert(ppu != NULL);
  if (state == TRUE) {
    ppu->statusRegister |= MASK_STATUS_VBLANK_ON;
  } else if (state == FALSE) {
    ppu->statusRegister &= MASK_STATUS_VBLANK_OFF;
  }
}
static Boolean ppu_getStatusVerticalBlank(PPU ppu) {
  assert(ppu != NULL);
  if ((ppu->statusRegister & MASK_STATUS_VBLANK_ON) == MASK_STATUS_VBLANK_ON) {
    return TRUE;
  } else {
    return FALSE;
  }
}
static Byte ppu_getCurrentX(PPU ppu) {
  assert(ppu != NULL);
  return ppu->currentScanlineCycle;
}
static Byte ppu_getCurrentY(PPU ppu) {
  assert(ppu != NULL);
  return ppu->currentScanline - PPU_WASTED_VBLANK_SCANLINES -
PPU_WASTED_PREFETCH_SCANLINES;
}
static Word ppu_getBackgroundTileNumber(Word x, Word y) {
  Word horizontalOffset = (x / PPU_HORIZONTAL_PIXELS_PER_TILE);
  Word verticalOffset = (y / PPU_VERTICAL_PIXELS_PER_TILE) *
PPU_BACKGROUND_TILES_PER_ROW;
  Word tileNumber = horizontalOffset + verticalOffset;
  return tileNumber;
}
static Byte ppu_getBackgroundNametableByte(NES nes) {
  assert(nes != NULL);
  PPU ppu = nes_getPPU(nes);
  assert(ppu != NULL);
  Address nametableStart = ppu_getCurrentBaseNametableAddress(ppu);
  assert(nametableStart == PPU_NAME_TABLE_0_FIRST_ADDRESS ||
         nametableStart == PPU_NAME_TABLE_1_FIRST_ADDRESS ||
         nametableStart == PPU_NAME_TABLE_2_FIRST_ADDRESS ||
         nametableStart == PPU_NAME_TABLE_3_FIRST_ADDRESS);
  Word x = ppu_getCurrentX(ppu);
  Word y = ppu_getCurrentY(ppu);
  Word tileNumber = ppu_getBackgroundTileNumber(x, y);
  Address nametableByteAddress = nametableStart + tileNumber;
  VALIDATE_NAMETABLE_ADDRESS(nametableByteAddress);
  return nes_readPPUMemory(nes,nametableByteAddress);
}
static Byte ppu_combinePatternBytes(PPU ppu, Byte pattern1, Byte pattern2, Byte x) {
  assert(ppu != NULL);
  x %= PPU_HORIZONTAL_PIXELS_PER_TILE;
  pattern1 = pattern1 << x;
  pattern1 = pattern1 >> (PPU_HORIZONTAL_PIXELS_PER_TILE - 1);
  pattern2 = pattern2 << x;
  pattern2 = pattern2 >> (PPU_HORIZONTAL_PIXELS_PER_TILE - 1);
  pattern2 = pattern2 << 1;
  Byte patternIndex = pattern1 + pattern2;
  assert(patternIndex <= 3);
  return patternIndex;
}
static Byte ppu_getPatternColourIndex(NES nes, Address basePatternAddress, Byte
patternTileNumber, Byte x, Byte y) {
  assert(nes != NULL);
  PPU ppu = nes_getPPU(nes);
  assert(ppu != NULL);
  Word horizontalOffset = patternTileNumber * PPU_PATTERN_BYTES_PER_TILE;
  Word verticalOffset = (y % PPU_VERTICAL_PIXELS_PER_TILE);
  Address pattern1Address = basePatternAddress + horizontalOffset + verticalOffset;
  Address pattern2Address = pattern1Address + 8;
  Byte pattern1 = nes_readPPUMemory(nes, pattern1Address);
  Byte pattern2 = nes_readPPUMemory(nes, pattern2Address);
  Address patternIndex = ppu_combinePatternBytes(ppu, pattern1, pattern2, x);
  return patternIndex;
}
static Byte ppu_getBackgroundAttributeByte(NES nes) {
  assert(nes != NULL);
  PPU ppu = nes_getPPU(nes);
  assert(ppu != NULL);
  Address nametableStart = ppu_getCurrentBaseNametableAddress(ppu);
  assert(nametableStart == PPU_NAME_TABLE_0_FIRST_ADDRESS ||
         nametableStart == PPU_NAME_TABLE_1_FIRST_ADDRESS ||
         nametableStart == PPU_NAME_TABLE_2_FIRST_ADDRESS ||
         nametableStart == PPU_NAME_TABLE_3_FIRST_ADDRESS);
  Word x = ppu_getCurrentX(ppu);
  Word y = ppu_getCurrentY(ppu);
  assert(PPU_NAMETABLE_BYTES_BEFORE_ATTRIBUTE_TABLE == 960);
  Address attributetableStart = nametableStart +
PPU_NAMETABLE_BYTES_BEFORE_ATTRIBUTE_TABLE;
  Word tileNumber = ppu_getBackgroundTileNumber(x, y);
  Word tileRowNumber = (tileNumber / PPU_BACKGROUND_TILES_PER_ROW);
  Word tileColumnNumber = tileNumber % PPU_BACKGROUND_TILES_PER_ROW;
  Word horizontalOffset = (tileColumnNumber / PPU_HORIZONTAL_TILES_PER_ATTRIBUTE_BYTE);
  Word verticalOffset = (tileRowNumber / PPU_VERTICAL_TILES_PER_ATTRIBUTE_BYTE) *
PPU_ATTRIBUTE_BYTES_PER_ROW;
  Address attributeByteAddress = attributetableStart + horizontalOffset +
verticalOffset;
  VALIDATE_NAMETABLE_ADDRESS(attributeByteAddress);
  Byte attributeByte = nes_readPPUMemory(nes,attributeByteAddress);
  return attributeByte;
}
static Byte ppu_getBackgroundAttributeColourIndex(NES nes) {
  assert(nes != NULL);
  PPU ppu = nes_getPPU(nes);
  assert(ppu != NULL);
  Byte attributeByte = ppu_getBackgroundAttributeByte(nes);
  Word x = ppu_getCurrentX(ppu);
  Word y = ppu_getCurrentY(ppu);
  Word tileNumber = ppu_getBackgroundTileNumber(x, y);
  Word horizontalOffset = (tileNumber % PPU_HORIZONTAL_TILES_PER_ATTRIBUTE_BYTE);
  Word tileRowNumber = (tileNumber / PPU_BACKGROUND_TILES_PER_ROW);
  Word verticalOffset = (tileRowNumber % PPU_VERTICAL_TILES_PER_ATTRIBUTE_BYTE);
  int attributeTileNumber = attributeTableLookup[verticalOffset][horizontalOffset];
  Byte attributeColourIndex = 0;
  switch(attributeTileNumber) {
    case 0x0:
    case 0x1:
    case 0x2:
    case 0x3:
      attributeColourIndex = (attributeByte << 6 );
      break;
    case 0x4:
    case 0x5:
    case 0x6:
    case 0x7:
      attributeColourIndex = (attributeByte << 4 );
      break;
    case 0x8:
    case 0x9:
    case 0xA:
    case 0xB:
      attributeColourIndex = (attributeByte << 2 );
      break;
    case 0xC:
    case 0xD:
    case 0xE:
    case 0xF:
      attributeColourIndex = attributeByte;
      break;
  }
  attributeColourIndex = attributeColourIndex >> 6;
  attributeColourIndex = attributeColourIndex << 2;
  return attributeColourIndex;
}
static Byte ppu_getSystemIndexFromBackgroundIndex(NES nes, Byte backgroundColourIndex) {
  assert(nes != NULL);
  Address address = PPU_BACKGROUND_PALETTE_FIRST_ADDRESS;
  address += backgroundColourIndex;
  Byte systemIndex = nes_readPPUMemory(nes, address);
  return systemIndex;
}
static Byte ppu_getCurrentBackgroundColour(NES nes) {
  assert(nes != NULL);
  PPU ppu = nes_getPPU(nes);
  assert(ppu != NULL);
  Address basePatternAddress;
  if (ppu_getControlBackgroundTileTable(ppu) == FALSE) {
    basePatternAddress = PPU_PATTERN_TABLE_0_FIRST_ADDRESS;
  } else {
    basePatternAddress = PPU_PATTERN_TABLE_1_FIRST_ADDRESS;
  }
  Byte x = ppu_getCurrentX(ppu);
  Byte y = ppu_getCurrentY(ppu);
  Byte patternTileNumber = ppu_getBackgroundNametableByte(nes);
  Byte patternColourIndex = ppu_getPatternColourIndex(nes, basePatternAddress,
patternTileNumber, x, y);
  Byte attributeColourIndex = ppu_getBackgroundAttributeColourIndex(nes);
  Byte backgroundColourIndex = 0;
  // if transparent, use the background colour
  if (patternColourIndex == 0) {
    backgroundColourIndex = 0;
  } else {
    backgroundColourIndex = patternColourIndex + attributeColourIndex ;
  }
  if (ppu_getMaskBackgroundVisibility(ppu) == FALSE) {
    backgroundColourIndex = 0;
  }
  return backgroundColourIndex;
}
static Byte ppu_getSystemIndexFromSpriteIndex(NES nes, Byte spriteColourIndex) {
  assert(nes != NULL);
  Address address = PPU_SPRITE_PALETTE_FIRST_ADDRESS;
  address += spriteColourIndex;
  Byte systemIndex = nes_readPPUMemory(nes, address);
  return systemIndex;
}
static void ppu_updateScanlineSpriteColour8(NES nes, Byte spriteIndex, Address
basePatternAddress, Byte multiplier) {
  assert(nes != NULL);
  PPU ppu = nes_getPPU(nes);
  assert(ppu != NULL);
  Memory objectAttributeMemory = nes_getObjectAttributeMemory(nes);
  assert(objectAttributeMemory != NULL);
  Byte screenY = ppu_getCurrentY(ppu);
  Byte spriteLeftX = objectAttributeMemory_getX(objectAttributeMemory, spriteIndex);
  Byte spriteTopY = objectAttributeMemory_getY(objectAttributeMemory, spriteIndex) - 1;
  int yOffset = 0;
  Boolean inRange = FALSE;
  if (screenY >= spriteTopY) {
    yOffset = screenY - spriteTopY;
    if (yOffset < (PPU_VERTICAL_PIXELS_PER_TILE*multiplier)) {
      inRange = TRUE;
    }
  }
  Byte y = yOffset;
  Byte patternTileNumber = objectAttributeMemory_getTileNumber(objectAttributeMemory,
spriteIndex, FALSE);
  if (yOffset >= 8) {
    y = yOffset - 8;
    patternTileNumber++;
  }
  if (objectAttributeMemory_isFlippedVertical(objectAttributeMemory, spriteIndex) ==
TRUE) {
    y = PPU_VERTICAL_PIXELS_PER_TILE - y - 1;
  }
  Byte attributeIndex = objectAttributeMemory_getPalette(objectAttributeMemory,
spriteIndex);
  if (inRange == TRUE) {
    int i;
    for (i=0; i < PPU_HORIZONTAL_PIXELS_PER_TILE; i++) {
      if (ppu->spriteColoursForScanlineSet[spriteLeftX+i] == FALSE) {
        Byte x = i;
        if (objectAttributeMemory_isFlippedHorizontal(objectAttributeMemory,
spriteIndex) == TRUE) {
          x = PPU_HORIZONTAL_PIXELS_PER_TILE - x - 1;
        }
        Byte patternIndex = ppu_getPatternColourIndex(nes, basePatternAddress,
patternTileNumber, x, y);
        if (patternIndex != 0) {
          ppu->spriteColoursForScanline[spriteLeftX +i] = patternIndex + attributeIndex;
          ppu->spriteColoursForScanlineSet[spriteLeftX+i] = TRUE;
          ppu->spriteColoursForScanlineIsBehindBackground[spriteLeftX +i] =
objectAttributeMemory_isBehindBackground(objectAttributeMemory, spriteIndex);
        }
      }
    }
  }
}
// this is almost a copy paste of ppu_updateScanlineSpriteColour8
// think of a better way
static void ppu_updateScanlineSpriteColour16(NES nes, Byte spriteIndex) {
    assert(nes != NULL);
  Memory objectAttributeMemory = nes_getObjectAttributeMemory(nes);
  assert(objectAttributeMemory != NULL);
  Address basePatternAddress = objectAttributeMemory_getBankNumber(objectAttributeMemory,
spriteIndex);
  ppu_updateScanlineSpriteColour8(nes, spriteIndex, basePatternAddress, 2);
}
static void ppu_calculateSpriteColoursForCurrentScanline(NES nes) {
  assert(nes != NULL);
  PPU ppu = nes_getPPU(nes);
  assert(ppu != NULL);
  ppu_resetSpriteColoursForScanline(ppu);
  if ( ppu_getMaskSpriteVisibility(ppu) == TRUE) {
    Byte spriteIndex;
    for (spriteIndex = 0; spriteIndex < OAM_NUMBER_OF_SPRITES; spriteIndex++) {
      Address basePatternAddress;
      if (ppu_getControlSpriteTileTable(ppu) == FALSE) {
        basePatternAddress = PPU_PATTERN_TABLE_0_FIRST_ADDRESS;
      } else {
        basePatternAddress = PPU_PATTERN_TABLE_1_FIRST_ADDRESS;
      }
      if (ppu_getControlSpriteSize(ppu) == TRUE) {
        ppu_updateScanlineSpriteColour16(nes, spriteIndex);
      } else {
        ppu_updateScanlineSpriteColour8(nes, spriteIndex, basePatternAddress, 1);
      }
    }
  }
}
static Byte ppu_getCurrentSpriteColour(NES nes, Byte currentBackgroundColour) {
  assert(nes != NULL);
  PPU ppu = nes_getPPU(nes);
  assert(ppu != NULL);
  Byte screenX = ppu_getCurrentX(ppu);
  Byte systemColourIndex = 0;
  if (ppu->spriteColoursForScanlineSet[screenX] == FALSE) {
    systemColourIndex = ppu_getSystemIndexFromBackgroundIndex(nes,
currentBackgroundColour);
                                                                                   //
one of the lower two bits set
  } else if ((ppu->spriteColoursForScanlineIsBehindBackground[screenX] == TRUE) &&
(currentBackgroundColour % 4 != 0)) {
    systemColourIndex = ppu_getSystemIndexFromBackgroundIndex(nes,
currentBackgroundColour);
    ppu_setStatusSpriteCollisionHit(ppu, TRUE);
  } else {
    ppu_setStatusSpriteCollisionHit(ppu, TRUE);
    systemColourIndex = ppu_getSystemIndexFromSpriteIndex(nes, ppu-
>spriteColoursForScanline[screenX]);
  }
  assert(systemColourIndex < PPU_NUM_SYSTEM_COLOURS);
  return systemColourIndex;
}
static void ppu_renderCurrentPixel(NES nes) {
  assert(nes != NULL);
  PPU ppu = nes_getPPU(nes);
  assert(ppu != NULL);
  GUI gui = nes_getGUI(nes);
  assert(gui != NULL);
  if (ppu->currentScanlineCycle == 0) {
    ppu_calculateSpriteColoursForCurrentScanline(nes);
  }
  Byte currentBackgroundColour = ppu_getCurrentBackgroundColour(nes);
  Byte currentSpriteColour = ppu_getCurrentSpriteColour(nes, currentBackgroundColour);
  Colour colour =
colour_init(systemPalette[currentSpriteColour].red,systemPalette[currentSpriteColour].gr
een,systemPalette[currentSpriteColour].blue);
gui_drawPixel(gui,ppu_getCurrentX(ppu),ppu_getCurrentY(ppu),colour_getRed(colour),colour
_getGreen(colour),colour_getBlue(colour));
  colour_destroy(colour);
}
void ppu_step(NES nes) {
  assert(nes != NULL);
  GUI gui = nes_getGUI(nes);
  assert(gui != NULL);
  debug_printf("ppu_step\n");
  PPU ppu = nes_getPPU(nes);
  assert(ppu != NULL);
  if (ppu->currentCycle == 0) {
    ppu->statusRegister = 0; // everything is cleared on line 0
  }
  if (ppu->currentScanline >=0 && ppu->currentScanline <= 19) {
    // do nothing, we are in vblank
  } else if (ppu->currentScanline == 20) {
    // debug_printf("%d\n", ppu->currentCycle);
    /*
    20: After 20 scanlines worth of time go by (since the VINT flag was set),
    the PPU starts to render scanlines. This first scanline is a dummy one;
    although it will access it's external memory in the same sequence it would
    for drawing a valid scanline, no on-screen pixels are rendered during this
    time, making the fetched background data immaterial. Both horizontal *and*
    vertical scroll counters are updated (presumably) at cc offset 256 in this
    scanline. Other than that, the operation of this scanline is identical to
    any other. The primary reason this scanline exists is to start the object
    render pipeline, since it takes 256 cc's worth of time to determine which
    objects are in range or not for any particular scanline.
    */
  } else if (ppu->currentScanline >= 21 && ppu->currentScanline <= 260) {
    /*
    21..260: after rendering 1 dummy scanline, the PPU starts to render the
    actual data to be displayed on the screen. This is done for 240 scanlines,
    of course.
    */
    if (ppu->currentScanlineCycle >=0 && ppu->currentScanlineCycle <
PPU_SCREEN_WIDTH_IN_PIXELS) {
      ppu_renderCurrentPixel(nes);
    }
  } else if (ppu->currentScanline == 261) {
    /*
    261:       after the very last rendered scanline finishes, the PPU does nothing
    for 1 scanline (i.e. the programmer gets screwed out of perfectly good VINT
    time). When this scanline finishes, the VINT flag is set, and the process of
    drawing lines starts all over again.
    */
  } else {
    assert(FALSE);
  }
  ppu->currentCycle++;
  ppu->currentScanlineCycle++;
  if (ppu->currentCycle % PPU_CYCLES_PER_SCANLINE == 0) {
    ppu->currentScanline++;
    ppu->currentScanlineCycle = 0;
    if (ppu->currentScanline == 20) {
      // The VBL flag is cleared 6820 PPU clocks, or exactly 20 scanlines, after it is
set.
      assert(ppu->currentCycle == 6820);
      ppu_setStatusSpriteOverflow(ppu, FALSE);
      ppu_setStatusSpriteCollisionHit(ppu, FALSE);
      ppu_setStatusVerticalBlank(ppu, FALSE);
    } else if (ppu->currentScanline % PPU_SCANLINES_PER_FRAME == 0) {
      // $2002.5 and $2002.6 after being set, stay that way for the first 20
      // scanlines of the new frame, relative to the VINT.
      ppu_setStatusVerticalBlank(ppu, TRUE);
      ppu->currentCycle = 0;
      ppu->currentScanline = 0;
      ppu->currentFrame++;
      gui_refresh(gui);
      printf("Frame: %d\n", ppu->currentFrame);
      if (ppu_getControlNMI(ppu) == TRUE) {
        nes_generateNMI(nes);
      }
    }
  }
}
Byte ppu_getControlRegister(PPU ppu) {
  assert(ppu != NULL);
  return ppu->controlRegister;
}
Byte ppu_getMaskRegister(PPU ppu) {
  assert(ppu != NULL);
  return ppu->maskRegister;
}
Byte ppu_getStatusRegister(PPU ppu) {
  assert(ppu != NULL);
  Byte status = ppu->statusRegister;
  // cleared on read
  ppu_setStatusVerticalBlank(ppu, FALSE);
  ppu->hasPartial = FALSE;
  return status;
}
Byte ppu_getSpriteAddressRegister(PPU ppu) {
  assert(ppu != NULL);
  return ppu->spriteAddressRegister;
}
Byte ppu_getSpriteDataRegister(NES nes) {
  assert(nes != NULL);
  PPU ppu = nes_getPPU(nes);
  assert(ppu != NULL);
  Byte data = nes_readObjectAttributeMemory(nes, ppu->spriteAddressRegister);
  //reads don't increase this?
  //ppu->spriteAddressRegister++;
  return data;
}
void ppu_setSpriteDataRegister(NES nes, Byte spriteDataRegister) {
  assert(nes != NULL);
  PPU ppu = nes_getPPU(nes);
  assert(ppu != NULL);
  nes_writeObjectAttributeMemory(nes, ppu->spriteAddressRegister, spriteDataRegister);
  ppu->spriteAddressRegister++;
}
Byte ppu_getScrollRegister(PPU ppu) {
  assert(ppu != NULL);
  return ppu->scrollRegister;
}
Byte ppu_getPPUMemoryAddressRegister(PPU ppu) {
  assert(ppu != NULL);
  return ppu->ppuMemoryAddressRegister;
}
void ppu_setControlRegister(PPU ppu, Byte controlRegister) {
  assert(ppu != NULL);
  ppu->controlRegister = controlRegister;
}
void ppu_setMaskRegister(PPU ppu, Byte maskRegister) {
  assert(ppu != NULL);
  ppu->maskRegister = maskRegister;
}
void ppu_setStatusRegister(PPU ppu, Byte statusRegister) {
  assert(ppu != NULL);
  ppu->statusRegister = statusRegister;
}
void ppu_setSpriteAddressRegister(PPU ppu, Byte spriteAddressRegister) {
  assert(ppu != NULL);
  ppu->spriteAddressRegister = spriteAddressRegister;
}
void ppu_setScrollRegister(PPU ppu, Byte scrollRegister) {
  assert(ppu != NULL);
  if (ppu->hasPartial == FALSE) {
    ppu->horizontalScroll = scrollRegister;
    ppu->hasPartial = TRUE;
  } else {
    // we have a partial scroll
    ppu->verticalScroll = scrollRegister;
    ppu->hasPartial = FALSE;
  }
  ppu->scrollRegister = scrollRegister;
}
void ppu_setPPUMemoryAddressRegister(PPU ppu, Byte ppuMemoryAddressRegister) {
  assert(ppu != NULL);
  if (ppu->hasPartial == FALSE) {
    ppu->ppuAddressHigh = ppuMemoryAddressRegister;
    ppu->hasPartial = TRUE;
  } else {
    // we have a partial address
    ppu->ppuAddressLow = ppuMemoryAddressRegister;
    ppu->hasPartial = FALSE;
  }
  ppu->ppuMemoryAddressRegister = ppuMemoryAddressRegister;
}
static void ppu_increasePPUMemoryAddress(PPU ppu) {
  //assert(ppu->hasPartial == FALSE);
  Address address = ppu->ppuAddressLow;
  address += ppu->ppuAddressHigh << BITS_PER_BYTE;
  if (ppu_getControlPPUAddressIncrement(ppu) == TRUE) {
    address += PPU_CONTROL_ADDRESS_VERTICAL_INCREMENT;
  } else {
    address += PPU_CONTROL_ADDRESS_HORIZONTAL_INCREMENT;
  }
  ppu->ppuAddressLow = GET_ADDRESS_LOW_BYTE(address);
  ppu->ppuAddressHigh = GET_ADDRESS_HIGH_BYTE(address);
}
void ppu_setPPUMemoryDataRegister(NES nes, Byte ppuMemoryDataRegister) {
  assert(nes != NULL);
  PPU ppu = nes_getPPU(nes);
  assert(ppu != NULL);
  Address address = ppu->ppuAddressLow;
  address += ppu->ppuAddressHigh << BITS_PER_BYTE;
  nes_writePPUMemory(nes, address, ppuMemoryDataRegister);
  //ppu->ppuMemoryDataRegister = ppuMemoryDataRegister;
  ppu_increasePPUMemoryAddress(ppu);
}
// one buffer pipeline
Byte ppu_getPPUMemoryDataRegister(NES nes) {
  assert(nes != NULL);
  PPU ppu = nes_getPPU(nes);
  assert(ppu != NULL);
  Byte data = ppu->ppuMemoryReadBuffer;
  Address address = ppu->ppuAddressLow;
  address += ppu->ppuAddressHigh << BITS_PER_BYTE;
  ppu->ppuMemoryReadBuffer = nes_readPPUMemory(nes, address);
  while (address > PPU_LAST_REAL_ADDRESS) {
    address -= PPU_NUM_REAL_ADDRESSES;
  }
  assert(address <= PPU_LAST_REAL_ADDRESS);
  // palette reads are not buffered
  if (address >= PPU_BACKGROUND_PALETTE_FIRST_ADDRESS) {
    data = ppu->ppuMemoryReadBuffer;
  }
  ppu_increasePPUMemoryAddress(ppu);
  return data;
}
void ppu_test() {
  {
    PPU ppu = ppu_init();
    assert(ppu != NULL);
    ppu_setStatusVerticalBlank(ppu, TRUE);
    assert(ppu_getStatusVerticalBlank(ppu) == TRUE);
    ppu_setStatusVerticalBlank(ppu, FALSE);
  assert(ppu_getStatusVerticalBlank(ppu) == FALSE);
}
{
  PPU ppu = ppu_init();
  assert(ppu != NULL);
  ppu_setControlPPUAddressIncrement(ppu, TRUE);
  assert(ppu_getControlPPUAddressIncrement(ppu) == TRUE);
  assert(ppu_getControlSpriteTileTable(ppu) == FALSE);
  assert(ppu_getControlBackgroundTileTable(ppu) == FALSE);
  assert(ppu_getControlSpriteSize(ppu) == FALSE);
  assert(ppu_getControlNMI(ppu) == FALSE);
  assert(ppu_getMaskDisplayType(ppu) == FALSE);
  assert(ppu_getMaskBackgroundClipping(ppu) == FALSE);
  assert(ppu_getMaskSpriteClipping(ppu) == FALSE);
  assert(ppu_getMaskBackgroundVisibility(ppu) == FALSE);
  assert(ppu_getMaskSpriteVisibility(ppu) == FALSE);
  assert(ppu_getStatusVerticalBlank(ppu) == FALSE);
  assert(ppu_getStatusSpriteOverflow(ppu) == FALSE);
  assert(ppu_getStatusSpriteCollisionHit(ppu) == FALSE);
  ppu_setControlPPUAddressIncrement(ppu, FALSE);
  assert(ppu_getControlPPUAddressIncrement(ppu) == FALSE);
  assert(ppu_getControlSpriteTileTable(ppu) == FALSE);
  assert(ppu_getControlBackgroundTileTable(ppu) == FALSE);
  assert(ppu_getControlSpriteSize(ppu) == FALSE);
  assert(ppu_getControlNMI(ppu) == FALSE);
  assert(ppu_getMaskDisplayType(ppu) == FALSE);
  assert(ppu_getMaskBackgroundClipping(ppu) == FALSE);
  assert(ppu_getMaskSpriteClipping(ppu) == FALSE);
  assert(ppu_getMaskBackgroundVisibility(ppu) == FALSE);
  assert(ppu_getMaskSpriteVisibility(ppu) == FALSE);
  assert(ppu_getStatusVerticalBlank(ppu) == FALSE);
  assert(ppu_getStatusSpriteOverflow(ppu) == FALSE);
  assert(ppu_getStatusSpriteCollisionHit(ppu) == FALSE);
  ppu_setControlSpriteTileTable(ppu, TRUE);
  assert(ppu_getControlPPUAddressIncrement(ppu) == FALSE);
  assert(ppu_getControlSpriteTileTable(ppu) == TRUE);
  assert(ppu_getControlBackgroundTileTable(ppu) == FALSE);
  assert(ppu_getControlSpriteSize(ppu) == FALSE);
  assert(ppu_getControlNMI(ppu) == FALSE);
  assert(ppu_getMaskDisplayType(ppu) == FALSE);
  assert(ppu_getMaskBackgroundClipping(ppu) == FALSE);
  assert(ppu_getMaskSpriteClipping(ppu) == FALSE);
  assert(ppu_getMaskBackgroundVisibility(ppu) == FALSE);
  assert(ppu_getMaskSpriteVisibility(ppu) == FALSE);
  assert(ppu_getStatusVerticalBlank(ppu) == FALSE);
  assert(ppu_getStatusSpriteOverflow(ppu) == FALSE);
  assert(ppu_getStatusSpriteCollisionHit(ppu) == FALSE);
  ppu_setControlSpriteTileTable(ppu, FALSE);
  assert(ppu_getControlPPUAddressIncrement(ppu) == FALSE);
  assert(ppu_getControlSpriteTileTable(ppu) == FALSE);
assert(ppu_getControlBackgroundTileTable(ppu) == FALSE);
assert(ppu_getControlSpriteSize(ppu) == FALSE);
assert(ppu_getControlNMI(ppu) == FALSE);
assert(ppu_getMaskDisplayType(ppu) == FALSE);
assert(ppu_getMaskBackgroundClipping(ppu) == FALSE);
assert(ppu_getMaskSpriteClipping(ppu) == FALSE);
assert(ppu_getMaskBackgroundVisibility(ppu) == FALSE);
assert(ppu_getMaskSpriteVisibility(ppu) == FALSE);
assert(ppu_getStatusVerticalBlank(ppu) == FALSE);
assert(ppu_getStatusSpriteOverflow(ppu) == FALSE);
assert(ppu_getStatusSpriteCollisionHit(ppu) == FALSE);
ppu_setControlBackgroundTileTable(ppu, TRUE);
assert(ppu_getControlPPUAddressIncrement(ppu) == FALSE);
assert(ppu_getControlSpriteTileTable(ppu) == FALSE);
assert(ppu_getControlBackgroundTileTable(ppu) == TRUE);
assert(ppu_getControlSpriteSize(ppu) == FALSE);
assert(ppu_getControlNMI(ppu) == FALSE);
assert(ppu_getMaskDisplayType(ppu) == FALSE);
assert(ppu_getMaskBackgroundClipping(ppu) == FALSE);
assert(ppu_getMaskSpriteClipping(ppu) == FALSE);
assert(ppu_getMaskBackgroundVisibility(ppu) == FALSE);
assert(ppu_getMaskSpriteVisibility(ppu) == FALSE);
assert(ppu_getStatusVerticalBlank(ppu) == FALSE);
assert(ppu_getStatusSpriteOverflow(ppu) == FALSE);
assert(ppu_getStatusSpriteCollisionHit(ppu) == FALSE);
ppu_setControlBackgroundTileTable(ppu, FALSE);
assert(ppu_getControlPPUAddressIncrement(ppu) == FALSE);
assert(ppu_getControlSpriteTileTable(ppu) == FALSE);
assert(ppu_getControlBackgroundTileTable(ppu) == FALSE);
assert(ppu_getControlSpriteSize(ppu) == FALSE);
assert(ppu_getControlNMI(ppu) == FALSE);
assert(ppu_getMaskDisplayType(ppu) == FALSE);
assert(ppu_getMaskBackgroundClipping(ppu) == FALSE);
assert(ppu_getMaskSpriteClipping(ppu) == FALSE);
assert(ppu_getMaskBackgroundVisibility(ppu) == FALSE);
assert(ppu_getMaskSpriteVisibility(ppu) == FALSE);
assert(ppu_getStatusVerticalBlank(ppu) == FALSE);
assert(ppu_getStatusSpriteOverflow(ppu) == FALSE);
assert(ppu_getStatusSpriteCollisionHit(ppu) == FALSE);
ppu_setControlSpriteSize(ppu, TRUE);
assert(ppu_getControlPPUAddressIncrement(ppu) == FALSE);
assert(ppu_getControlSpriteTileTable(ppu) == FALSE);
assert(ppu_getControlBackgroundTileTable(ppu) == FALSE);
assert(ppu_getControlSpriteSize(ppu) == TRUE);
assert(ppu_getControlNMI(ppu) == FALSE);
assert(ppu_getMaskDisplayType(ppu) == FALSE);
assert(ppu_getMaskBackgroundClipping(ppu) == FALSE);
assert(ppu_getMaskSpriteClipping(ppu) == FALSE);
assert(ppu_getMaskBackgroundVisibility(ppu) == FALSE);
assert(ppu_getMaskSpriteVisibility(ppu) == FALSE);
assert(ppu_getStatusVerticalBlank(ppu) == FALSE);
assert(ppu_getStatusSpriteOverflow(ppu) == FALSE);
assert(ppu_getStatusSpriteCollisionHit(ppu) == FALSE);
ppu_setControlSpriteSize(ppu, FALSE);
assert(ppu_getControlPPUAddressIncrement(ppu) == FALSE);
assert(ppu_getControlSpriteTileTable(ppu) == FALSE);
assert(ppu_getControlBackgroundTileTable(ppu) == FALSE);
assert(ppu_getControlSpriteSize(ppu) == FALSE);
assert(ppu_getControlNMI(ppu) == FALSE);
assert(ppu_getMaskDisplayType(ppu) == FALSE);
assert(ppu_getMaskBackgroundClipping(ppu) == FALSE);
assert(ppu_getMaskSpriteClipping(ppu) == FALSE);
assert(ppu_getMaskBackgroundVisibility(ppu) == FALSE);
assert(ppu_getMaskSpriteVisibility(ppu) == FALSE);
assert(ppu_getStatusVerticalBlank(ppu) == FALSE);
assert(ppu_getStatusSpriteOverflow(ppu) == FALSE);
assert(ppu_getStatusSpriteCollisionHit(ppu) == FALSE);
ppu_setControlNMI(ppu, TRUE);
assert(ppu_getControlPPUAddressIncrement(ppu) == FALSE);
assert(ppu_getControlSpriteTileTable(ppu) == FALSE);
assert(ppu_getControlBackgroundTileTable(ppu) == FALSE);
assert(ppu_getControlSpriteSize(ppu) == FALSE);
assert(ppu_getControlNMI(ppu) == TRUE);
assert(ppu_getMaskDisplayType(ppu) == FALSE);
assert(ppu_getMaskBackgroundClipping(ppu) == FALSE);
assert(ppu_getMaskSpriteClipping(ppu) == FALSE);
assert(ppu_getMaskBackgroundVisibility(ppu) == FALSE);
assert(ppu_getMaskSpriteVisibility(ppu) == FALSE);
assert(ppu_getStatusVerticalBlank(ppu) == FALSE);
assert(ppu_getStatusSpriteOverflow(ppu) == FALSE);
assert(ppu_getStatusSpriteCollisionHit(ppu) == FALSE);
ppu_setControlNMI(ppu, FALSE);
assert(ppu_getControlPPUAddressIncrement(ppu) == FALSE);
assert(ppu_getControlSpriteTileTable(ppu) == FALSE);
assert(ppu_getControlBackgroundTileTable(ppu) == FALSE);
assert(ppu_getControlSpriteSize(ppu) == FALSE);
assert(ppu_getControlNMI(ppu) == FALSE);
assert(ppu_getMaskDisplayType(ppu) == FALSE);
assert(ppu_getMaskBackgroundClipping(ppu) == FALSE);
assert(ppu_getMaskSpriteClipping(ppu) == FALSE);
assert(ppu_getMaskBackgroundVisibility(ppu) == FALSE);
assert(ppu_getMaskSpriteVisibility(ppu) == FALSE);
assert(ppu_getStatusVerticalBlank(ppu) == FALSE);
assert(ppu_getStatusSpriteOverflow(ppu) == FALSE);
assert(ppu_getStatusSpriteCollisionHit(ppu) == FALSE);
ppu_setStatusVerticalBlank(ppu, TRUE);
assert(ppu_getControlPPUAddressIncrement(ppu) == FALSE);
assert(ppu_getControlSpriteTileTable(ppu) == FALSE);
assert(ppu_getControlBackgroundTileTable(ppu) == FALSE);
assert(ppu_getControlSpriteSize(ppu) == FALSE);
assert(ppu_getControlNMI(ppu) == FALSE);
assert(ppu_getMaskDisplayType(ppu) == FALSE);
assert(ppu_getMaskBackgroundClipping(ppu) == FALSE);
assert(ppu_getMaskSpriteClipping(ppu) == FALSE);
assert(ppu_getMaskBackgroundVisibility(ppu) == FALSE);
assert(ppu_getMaskSpriteVisibility(ppu) == FALSE);
assert(ppu_getStatusVerticalBlank(ppu) == TRUE);
assert(ppu_getStatusSpriteOverflow(ppu) == FALSE);
assert(ppu_getStatusSpriteCollisionHit(ppu) == FALSE);
ppu_setStatusVerticalBlank(ppu, FALSE);
assert(ppu_getControlPPUAddressIncrement(ppu) == FALSE);
assert(ppu_getControlSpriteTileTable(ppu) == FALSE);
assert(ppu_getControlBackgroundTileTable(ppu) == FALSE);
assert(ppu_getControlSpriteSize(ppu) == FALSE);
assert(ppu_getControlNMI(ppu) == FALSE);
assert(ppu_getMaskDisplayType(ppu) == FALSE);
assert(ppu_getMaskBackgroundClipping(ppu) == FALSE);
assert(ppu_getMaskSpriteClipping(ppu) == FALSE);
assert(ppu_getMaskBackgroundVisibility(ppu) == FALSE);
assert(ppu_getMaskSpriteVisibility(ppu) == FALSE);
assert(ppu_getStatusVerticalBlank(ppu) == FALSE);
assert(ppu_getStatusSpriteOverflow(ppu) == FALSE);
assert(ppu_getStatusSpriteCollisionHit(ppu) == FALSE);
ppu_setMaskDisplayType(ppu, TRUE);
assert(ppu_getControlPPUAddressIncrement(ppu) == FALSE);
assert(ppu_getControlSpriteTileTable(ppu) == FALSE);
assert(ppu_getControlBackgroundTileTable(ppu) == FALSE);
assert(ppu_getControlSpriteSize(ppu) == FALSE);
assert(ppu_getControlNMI(ppu) == FALSE);
assert(ppu_getMaskDisplayType(ppu) == TRUE);
assert(ppu_getMaskBackgroundClipping(ppu) == FALSE);
assert(ppu_getMaskSpriteClipping(ppu) == FALSE);
assert(ppu_getMaskBackgroundVisibility(ppu) == FALSE);
assert(ppu_getMaskSpriteVisibility(ppu) == FALSE);
assert(ppu_getStatusVerticalBlank(ppu) == FALSE);
assert(ppu_getStatusSpriteOverflow(ppu) == FALSE);
assert(ppu_getStatusSpriteCollisionHit(ppu) == FALSE);
ppu_setMaskDisplayType(ppu, FALSE);
assert(ppu_getControlPPUAddressIncrement(ppu) == FALSE);
assert(ppu_getControlSpriteTileTable(ppu) == FALSE);
assert(ppu_getControlBackgroundTileTable(ppu) == FALSE);
assert(ppu_getControlSpriteSize(ppu) == FALSE);
assert(ppu_getControlNMI(ppu) == FALSE);
assert(ppu_getMaskDisplayType(ppu) == FALSE);
assert(ppu_getMaskBackgroundClipping(ppu) == FALSE);
assert(ppu_getMaskSpriteClipping(ppu) == FALSE);
assert(ppu_getMaskBackgroundVisibility(ppu) == FALSE);
assert(ppu_getMaskSpriteVisibility(ppu) == FALSE);
assert(ppu_getStatusVerticalBlank(ppu) == FALSE);
assert(ppu_getStatusSpriteOverflow(ppu) == FALSE);
assert(ppu_getStatusSpriteCollisionHit(ppu) == FALSE);
ppu_setMaskBackgroundClipping(ppu, TRUE);
assert(ppu_getControlPPUAddressIncrement(ppu) == FALSE);
assert(ppu_getControlSpriteTileTable(ppu) == FALSE);
assert(ppu_getControlBackgroundTileTable(ppu) == FALSE);
assert(ppu_getControlSpriteSize(ppu) == FALSE);
assert(ppu_getControlNMI(ppu) == FALSE);
assert(ppu_getMaskDisplayType(ppu) == FALSE);
assert(ppu_getMaskBackgroundClipping(ppu) == TRUE);
assert(ppu_getMaskSpriteClipping(ppu) == FALSE);
assert(ppu_getMaskBackgroundVisibility(ppu) == FALSE);
assert(ppu_getMaskSpriteVisibility(ppu) == FALSE);
assert(ppu_getStatusVerticalBlank(ppu) == FALSE);
assert(ppu_getStatusSpriteOverflow(ppu) == FALSE);
assert(ppu_getStatusSpriteCollisionHit(ppu) == FALSE);
ppu_setMaskBackgroundClipping(ppu, FALSE);
assert(ppu_getControlPPUAddressIncrement(ppu) == FALSE);
assert(ppu_getControlSpriteTileTable(ppu) == FALSE);
assert(ppu_getControlBackgroundTileTable(ppu) == FALSE);
assert(ppu_getControlSpriteSize(ppu) == FALSE);
assert(ppu_getControlNMI(ppu) == FALSE);
assert(ppu_getMaskDisplayType(ppu) == FALSE);
assert(ppu_getMaskBackgroundClipping(ppu) == FALSE);
assert(ppu_getMaskSpriteClipping(ppu) == FALSE);
assert(ppu_getMaskBackgroundVisibility(ppu) == FALSE);
assert(ppu_getMaskSpriteVisibility(ppu) == FALSE);
assert(ppu_getStatusVerticalBlank(ppu) == FALSE);
assert(ppu_getStatusSpriteOverflow(ppu) == FALSE);
assert(ppu_getStatusSpriteCollisionHit(ppu) == FALSE);
ppu_setMaskSpriteClipping(ppu, TRUE);
assert(ppu_getControlPPUAddressIncrement(ppu) == FALSE);
assert(ppu_getControlSpriteTileTable(ppu) == FALSE);
assert(ppu_getControlBackgroundTileTable(ppu) == FALSE);
assert(ppu_getControlSpriteSize(ppu) == FALSE);
assert(ppu_getControlNMI(ppu) == FALSE);
assert(ppu_getMaskDisplayType(ppu) == FALSE);
assert(ppu_getMaskBackgroundClipping(ppu) == FALSE);
assert(ppu_getMaskSpriteClipping(ppu) == TRUE);
assert(ppu_getMaskBackgroundVisibility(ppu) == FALSE);
assert(ppu_getMaskSpriteVisibility(ppu) == FALSE);
assert(ppu_getStatusVerticalBlank(ppu) == FALSE);
assert(ppu_getStatusSpriteOverflow(ppu) == FALSE);
assert(ppu_getStatusSpriteCollisionHit(ppu) == FALSE);
ppu_setMaskSpriteClipping(ppu, FALSE);
assert(ppu_getControlPPUAddressIncrement(ppu) == FALSE);
assert(ppu_getControlSpriteTileTable(ppu) == FALSE);
assert(ppu_getControlBackgroundTileTable(ppu) == FALSE);
assert(ppu_getControlSpriteSize(ppu) == FALSE);
assert(ppu_getControlNMI(ppu) == FALSE);
assert(ppu_getMaskDisplayType(ppu) == FALSE);
assert(ppu_getMaskBackgroundClipping(ppu) == FALSE);
assert(ppu_getMaskSpriteClipping(ppu) == FALSE);
assert(ppu_getMaskBackgroundVisibility(ppu) == FALSE);
assert(ppu_getMaskSpriteVisibility(ppu) == FALSE);
assert(ppu_getStatusVerticalBlank(ppu) == FALSE);
assert(ppu_getStatusSpriteOverflow(ppu) == FALSE);
assert(ppu_getStatusSpriteCollisionHit(ppu) == FALSE);
ppu_setMaskBackgroundVisiblity(ppu, TRUE);
assert(ppu_getControlPPUAddressIncrement(ppu) == FALSE);
assert(ppu_getControlSpriteTileTable(ppu) == FALSE);
assert(ppu_getControlBackgroundTileTable(ppu) == FALSE);
assert(ppu_getControlSpriteSize(ppu) == FALSE);
assert(ppu_getControlNMI(ppu) == FALSE);
assert(ppu_getMaskDisplayType(ppu) == FALSE);
assert(ppu_getMaskBackgroundClipping(ppu) == FALSE);
assert(ppu_getMaskSpriteClipping(ppu) == FALSE);
assert(ppu_getMaskBackgroundVisibility(ppu) == TRUE);
assert(ppu_getMaskSpriteVisibility(ppu) == FALSE);
assert(ppu_getStatusVerticalBlank(ppu) == FALSE);
assert(ppu_getStatusSpriteOverflow(ppu) == FALSE);
assert(ppu_getStatusSpriteCollisionHit(ppu) == FALSE);
ppu_setMaskBackgroundVisiblity(ppu, FALSE);
assert(ppu_getControlPPUAddressIncrement(ppu) == FALSE);
assert(ppu_getControlSpriteTileTable(ppu) == FALSE);
assert(ppu_getControlBackgroundTileTable(ppu) == FALSE);
assert(ppu_getControlSpriteSize(ppu) == FALSE);
assert(ppu_getControlNMI(ppu) == FALSE);
assert(ppu_getMaskDisplayType(ppu) == FALSE);
assert(ppu_getMaskBackgroundClipping(ppu) == FALSE);
assert(ppu_getMaskSpriteClipping(ppu) == FALSE);
assert(ppu_getMaskBackgroundVisibility(ppu) == FALSE);
assert(ppu_getMaskSpriteVisibility(ppu) == FALSE);
assert(ppu_getStatusVerticalBlank(ppu) == FALSE);
assert(ppu_getStatusSpriteOverflow(ppu) == FALSE);
assert(ppu_getStatusSpriteCollisionHit(ppu) == FALSE);
ppu_setMaskSpriteVisibility(ppu, TRUE);
assert(ppu_getControlPPUAddressIncrement(ppu) == FALSE);
assert(ppu_getControlSpriteTileTable(ppu) == FALSE);
assert(ppu_getControlBackgroundTileTable(ppu) == FALSE);
assert(ppu_getControlSpriteSize(ppu) == FALSE);
assert(ppu_getControlNMI(ppu) == FALSE);
assert(ppu_getMaskDisplayType(ppu) == FALSE);
assert(ppu_getMaskBackgroundClipping(ppu) == FALSE);
assert(ppu_getMaskSpriteClipping(ppu) == FALSE);
assert(ppu_getMaskBackgroundVisibility(ppu) == FALSE);
assert(ppu_getMaskSpriteVisibility(ppu) == TRUE);
assert(ppu_getStatusVerticalBlank(ppu) == FALSE);
assert(ppu_getStatusSpriteOverflow(ppu) == FALSE);
assert(ppu_getStatusSpriteCollisionHit(ppu) == FALSE);
ppu_setMaskSpriteVisibility(ppu, FALSE);
assert(ppu_getControlPPUAddressIncrement(ppu) == FALSE);
assert(ppu_getControlSpriteTileTable(ppu) == FALSE);
assert(ppu_getControlBackgroundTileTable(ppu) == FALSE);
assert(ppu_getControlSpriteSize(ppu) == FALSE);
assert(ppu_getControlNMI(ppu) == FALSE);
assert(ppu_getMaskDisplayType(ppu) == FALSE);
assert(ppu_getMaskBackgroundClipping(ppu) == FALSE);
assert(ppu_getMaskSpriteClipping(ppu) == FALSE);
assert(ppu_getMaskBackgroundVisibility(ppu) == FALSE);
assert(ppu_getMaskSpriteVisibility(ppu) == FALSE);
assert(ppu_getStatusVerticalBlank(ppu) == FALSE);
assert(ppu_getStatusSpriteOverflow(ppu) == FALSE);
assert(ppu_getStatusSpriteCollisionHit(ppu) == FALSE);
ppu_setStatusSpriteOverflow(ppu, TRUE);
assert(ppu_getControlPPUAddressIncrement(ppu) == FALSE);
assert(ppu_getControlSpriteTileTable(ppu) == FALSE);
assert(ppu_getControlBackgroundTileTable(ppu) == FALSE);
assert(ppu_getControlSpriteSize(ppu) == FALSE);
assert(ppu_getControlNMI(ppu) == FALSE);
assert(ppu_getMaskDisplayType(ppu) == FALSE);
assert(ppu_getMaskBackgroundClipping(ppu) == FALSE);
assert(ppu_getMaskSpriteClipping(ppu) == FALSE);
assert(ppu_getMaskBackgroundVisibility(ppu) == FALSE);
assert(ppu_getMaskSpriteVisibility(ppu) == FALSE);
assert(ppu_getStatusVerticalBlank(ppu) == FALSE);
assert(ppu_getStatusSpriteOverflow(ppu) == TRUE);
assert(ppu_getStatusSpriteCollisionHit(ppu) == FALSE);
ppu_setStatusSpriteOverflow(ppu, FALSE);
assert(ppu_getControlPPUAddressIncrement(ppu) == FALSE);
assert(ppu_getControlSpriteTileTable(ppu) == FALSE);
assert(ppu_getControlBackgroundTileTable(ppu) == FALSE);
assert(ppu_getControlSpriteSize(ppu) == FALSE);
assert(ppu_getControlNMI(ppu) == FALSE);
assert(ppu_getMaskDisplayType(ppu) == FALSE);
assert(ppu_getMaskBackgroundClipping(ppu) == FALSE);
assert(ppu_getMaskSpriteClipping(ppu) == FALSE);
assert(ppu_getMaskBackgroundVisibility(ppu) == FALSE);
assert(ppu_getMaskSpriteVisibility(ppu) == FALSE);
assert(ppu_getStatusVerticalBlank(ppu) == FALSE);
assert(ppu_getStatusSpriteOverflow(ppu) == FALSE);
assert(ppu_getStatusSpriteCollisionHit(ppu) == FALSE);
ppu_setStatusSpriteCollisionHit(ppu, TRUE);
assert(ppu_getControlPPUAddressIncrement(ppu) == FALSE);
assert(ppu_getControlSpriteTileTable(ppu) == FALSE);
assert(ppu_getControlBackgroundTileTable(ppu) == FALSE);
assert(ppu_getControlSpriteSize(ppu) == FALSE);
assert(ppu_getControlNMI(ppu) == FALSE);
assert(ppu_getMaskDisplayType(ppu) == FALSE);
  assert(ppu_getMaskBackgroundClipping(ppu) == FALSE);
  assert(ppu_getMaskSpriteClipping(ppu) == FALSE);
  assert(ppu_getMaskBackgroundVisibility(ppu) == FALSE);
  assert(ppu_getMaskSpriteVisibility(ppu) == FALSE);
  assert(ppu_getStatusVerticalBlank(ppu) == FALSE);
  assert(ppu_getStatusSpriteOverflow(ppu) == FALSE);
  assert(ppu_getStatusSpriteCollisionHit(ppu) == TRUE);
  ppu_setStatusSpriteCollisionHit(ppu, FALSE);
  assert(ppu_getControlPPUAddressIncrement(ppu) == FALSE);
  assert(ppu_getControlSpriteTileTable(ppu) == FALSE);
  assert(ppu_getControlBackgroundTileTable(ppu) == FALSE);
  assert(ppu_getControlSpriteSize(ppu) == FALSE);
  assert(ppu_getControlNMI(ppu) == FALSE);
  assert(ppu_getMaskDisplayType(ppu) == FALSE);
  assert(ppu_getMaskBackgroundClipping(ppu) == FALSE);
  assert(ppu_getMaskSpriteClipping(ppu) == FALSE);
  assert(ppu_getMaskBackgroundVisibility(ppu) == FALSE);
  assert(ppu_getMaskSpriteVisibility(ppu) == FALSE);
  assert(ppu_getStatusVerticalBlank(ppu) == FALSE);
  assert(ppu_getStatusSpriteOverflow(ppu) == FALSE);
  assert(ppu_getStatusSpriteCollisionHit(ppu) == FALSE);
}
{
  PPU ppu = ppu_init();
  assert(ppu != NULL);
  cpu_setMarkIntensifyRed(ppu, TRUE);
  assert(ppu_getControlPPUAddressIncrement(ppu) == FALSE);
  assert(ppu_getControlSpriteTileTable(ppu) == FALSE);
  assert(ppu_getControlBackgroundTileTable(ppu) == FALSE);
  assert(ppu_getControlSpriteSize(ppu) == FALSE);
  assert(ppu_getControlNMI(ppu) == FALSE);
  assert(ppu_getMaskDisplayType(ppu) == FALSE);
  assert(ppu_getMaskBackgroundClipping(ppu) == FALSE);
  assert(ppu_getMaskSpriteClipping(ppu) == FALSE);
  assert(ppu_getMaskBackgroundVisibility(ppu) == FALSE);
  assert(ppu_getMaskSpriteVisibility(ppu) == FALSE);
  assert(cpu_getMaskIntensifyRed(ppu) == TRUE);
  assert(cpu_getMaskIntensifyGren(ppu) == FALSE);
  assert(cpu_getMaskIntensifyBlue(ppu) == FALSE);
  assert(ppu_getStatusVerticalBlank(ppu) == FALSE);
  assert(ppu_getStatusSpriteOverflow(ppu) == FALSE);
  assert(ppu_getStatusSpriteCollisionHit(ppu) == FALSE);
  cpu_setMarkIntensifyRed(ppu, FALSE);
  assert(ppu_getControlPPUAddressIncrement(ppu) == FALSE);
  assert(ppu_getControlSpriteTileTable(ppu) == FALSE);
  assert(ppu_getControlBackgroundTileTable(ppu) == FALSE);
  assert(ppu_getControlSpriteSize(ppu) == FALSE);
  assert(ppu_getControlNMI(ppu) == FALSE);
  assert(ppu_getMaskDisplayType(ppu) == FALSE);
  assert(ppu_getMaskBackgroundClipping(ppu) == FALSE);
  assert(ppu_getMaskSpriteClipping(ppu) == FALSE);
assert(ppu_getMaskBackgroundVisibility(ppu) == FALSE);
assert(ppu_getMaskSpriteVisibility(ppu) == FALSE);
assert(cpu_getMaskIntensifyRed(ppu) == FALSE);
assert(cpu_getMaskIntensifyGren(ppu) == FALSE);
assert(cpu_getMaskIntensifyBlue(ppu) == FALSE);
assert(ppu_getStatusVerticalBlank(ppu) == FALSE);
assert(ppu_getStatusSpriteOverflow(ppu) == FALSE);
assert(ppu_getStatusSpriteCollisionHit(ppu) == FALSE);
cpu_setMarkIntensifyGreen(ppu, TRUE);
assert(ppu_getControlPPUAddressIncrement(ppu) == FALSE);
assert(ppu_getControlSpriteTileTable(ppu) == FALSE);
assert(ppu_getControlBackgroundTileTable(ppu) == FALSE);
assert(ppu_getControlSpriteSize(ppu) == FALSE);
assert(ppu_getControlNMI(ppu) == FALSE);
assert(ppu_getMaskDisplayType(ppu) == FALSE);
assert(ppu_getMaskBackgroundClipping(ppu) == FALSE);
assert(ppu_getMaskSpriteClipping(ppu) == FALSE);
assert(ppu_getMaskBackgroundVisibility(ppu) == FALSE);
assert(ppu_getMaskSpriteVisibility(ppu) == FALSE);
assert(cpu_getMaskIntensifyRed(ppu) == FALSE);
assert(cpu_getMaskIntensifyGren(ppu) == TRUE);
assert(cpu_getMaskIntensifyBlue(ppu) == FALSE);
assert(ppu_getStatusVerticalBlank(ppu) == FALSE);
assert(ppu_getStatusSpriteOverflow(ppu) == FALSE);
assert(ppu_getStatusSpriteCollisionHit(ppu) == FALSE);
cpu_setMarkIntensifyGreen(ppu, FALSE);
assert(ppu_getControlPPUAddressIncrement(ppu) == FALSE);
assert(ppu_getControlSpriteTileTable(ppu) == FALSE);
assert(ppu_getControlBackgroundTileTable(ppu) == FALSE);
assert(ppu_getControlSpriteSize(ppu) == FALSE);
assert(ppu_getControlNMI(ppu) == FALSE);
assert(ppu_getMaskDisplayType(ppu) == FALSE);
assert(ppu_getMaskBackgroundClipping(ppu) == FALSE);
assert(ppu_getMaskSpriteClipping(ppu) == FALSE);
assert(ppu_getMaskBackgroundVisibility(ppu) == FALSE);
assert(ppu_getMaskSpriteVisibility(ppu) == FALSE);
assert(cpu_getMaskIntensifyRed(ppu) == FALSE);
assert(cpu_getMaskIntensifyGren(ppu) == FALSE);
assert(cpu_getMaskIntensifyBlue(ppu) == FALSE);
assert(ppu_getStatusVerticalBlank(ppu) == FALSE);
assert(ppu_getStatusSpriteOverflow(ppu) == FALSE);
assert(ppu_getStatusSpriteCollisionHit(ppu) == FALSE);
cpu_setMarkIntensifyBlue(ppu, TRUE);
assert(ppu_getControlPPUAddressIncrement(ppu) == FALSE);
assert(ppu_getControlSpriteTileTable(ppu) == FALSE);
assert(ppu_getControlBackgroundTileTable(ppu) == FALSE);
assert(ppu_getControlSpriteSize(ppu) == FALSE);
assert(ppu_getControlNMI(ppu) == FALSE);
assert(ppu_getMaskDisplayType(ppu) == FALSE);
assert(ppu_getMaskBackgroundClipping(ppu) == FALSE);
assert(ppu_getMaskSpriteClipping(ppu) == FALSE);
assert(ppu_getMaskBackgroundVisibility(ppu) == FALSE);
assert(ppu_getMaskSpriteVisibility(ppu) == FALSE);
assert(cpu_getMaskIntensifyRed(ppu) == FALSE);
assert(cpu_getMaskIntensifyGren(ppu) == FALSE);
assert(cpu_getMaskIntensifyBlue(ppu) == TRUE);
assert(ppu_getStatusVerticalBlank(ppu) == FALSE);
assert(ppu_getStatusSpriteOverflow(ppu) == FALSE);
assert(ppu_getStatusSpriteCollisionHit(ppu) == FALSE);
cpu_setMarkIntensifyBlue(ppu, FALSE);
assert(ppu_getControlPPUAddressIncrement(ppu) == FALSE);
assert(ppu_getControlSpriteTileTable(ppu) == FALSE);
assert(ppu_getControlBackgroundTileTable(ppu) == FALSE);
assert(ppu_getControlSpriteSize(ppu) == FALSE);
assert(ppu_getControlNMI(ppu) == FALSE);
assert(ppu_getMaskDisplayType(ppu) == FALSE);
assert(ppu_getMaskBackgroundClipping(ppu) == FALSE);
assert(ppu_getMaskSpriteClipping(ppu) == FALSE);
assert(ppu_getMaskBackgroundVisibility(ppu) == FALSE);
assert(ppu_getMaskSpriteVisibility(ppu) == FALSE);
assert(cpu_getMaskIntensifyRed(ppu) == FALSE);
assert(cpu_getMaskIntensifyGren(ppu) == FALSE);
assert(cpu_getMaskIntensifyBlue(ppu) == FALSE);
assert(ppu_getStatusVerticalBlank(ppu) == FALSE);
assert(ppu_getStatusSpriteOverflow(ppu) == FALSE);
assert(ppu_getStatusSpriteCollisionHit(ppu) == FALSE);
ppu_setControlHorizontalScrollNametable(ppu, TRUE);
assert(ppu_getControlHorizontalScrollNametable(ppu) == TRUE);
assert(ppu_getControlVerticalScrollNametable(ppu) == FALSE);
assert(ppu_getControlPPUAddressIncrement(ppu) == FALSE);
assert(ppu_getControlSpriteTileTable(ppu) == FALSE);
assert(ppu_getControlBackgroundTileTable(ppu) == FALSE);
assert(ppu_getControlSpriteSize(ppu) == FALSE);
assert(ppu_getControlNMI(ppu) == FALSE);
assert(ppu_getMaskDisplayType(ppu) == FALSE);
assert(ppu_getMaskBackgroundClipping(ppu) == FALSE);
assert(ppu_getMaskSpriteClipping(ppu) == FALSE);
assert(ppu_getMaskBackgroundVisibility(ppu) == FALSE);
assert(ppu_getMaskSpriteVisibility(ppu) == FALSE);
assert(cpu_getMaskIntensifyRed(ppu) == FALSE);
assert(cpu_getMaskIntensifyGren(ppu) == FALSE);
assert(cpu_getMaskIntensifyBlue(ppu) == FALSE);
assert(ppu_getStatusVerticalBlank(ppu) == FALSE);
assert(ppu_getStatusSpriteOverflow(ppu) == FALSE);
assert(ppu_getStatusSpriteCollisionHit(ppu) == FALSE);
ppu_setControlHorizontalScrollNametable(ppu, FALSE);
assert(ppu_getControlHorizontalScrollNametable(ppu) == FALSE);
assert(ppu_getControlVerticalScrollNametable(ppu) == FALSE);
assert(ppu_getControlPPUAddressIncrement(ppu) == FALSE);
assert(ppu_getControlSpriteTileTable(ppu) == FALSE);
assert(ppu_getControlBackgroundTileTable(ppu) == FALSE);
    assert(ppu_getControlSpriteSize(ppu) == FALSE);
    assert(ppu_getControlNMI(ppu) == FALSE);
    assert(ppu_getMaskDisplayType(ppu) == FALSE);
    assert(ppu_getMaskBackgroundClipping(ppu) == FALSE);
    assert(ppu_getMaskSpriteClipping(ppu) == FALSE);
    assert(ppu_getMaskBackgroundVisibility(ppu) == FALSE);
    assert(ppu_getMaskSpriteVisibility(ppu) == FALSE);
    assert(cpu_getMaskIntensifyRed(ppu) == FALSE);
    assert(cpu_getMaskIntensifyGren(ppu) == FALSE);
    assert(cpu_getMaskIntensifyBlue(ppu) == FALSE);
    assert(ppu_getStatusVerticalBlank(ppu) == FALSE);
    assert(ppu_getStatusSpriteOverflow(ppu) == FALSE);
    assert(ppu_getStatusSpriteCollisionHit(ppu) == FALSE);
    ppu_setControlVerticalScrollNametable(ppu, TRUE);
    assert(ppu_getControlHorizontalScrollNametable(ppu) == FALSE);
    assert(ppu_getControlVerticalScrollNametable(ppu) == TRUE);
    assert(ppu_getControlPPUAddressIncrement(ppu) == FALSE);
    assert(ppu_getControlSpriteTileTable(ppu) == FALSE);
    assert(ppu_getControlBackgroundTileTable(ppu) == FALSE);
    assert(ppu_getControlSpriteSize(ppu) == FALSE);
    assert(ppu_getControlNMI(ppu) == FALSE);
    assert(ppu_getMaskDisplayType(ppu) == FALSE);
    assert(ppu_getMaskBackgroundClipping(ppu) == FALSE);
    assert(ppu_getMaskSpriteClipping(ppu) == FALSE);
    assert(ppu_getMaskBackgroundVisibility(ppu) == FALSE);
    assert(ppu_getMaskSpriteVisibility(ppu) == FALSE);
    assert(cpu_getMaskIntensifyRed(ppu) == FALSE);
    assert(cpu_getMaskIntensifyGren(ppu) == FALSE);
    assert(cpu_getMaskIntensifyBlue(ppu) == FALSE);
    assert(ppu_getStatusVerticalBlank(ppu) == FALSE);
    assert(ppu_getStatusSpriteOverflow(ppu) == FALSE);
    assert(ppu_getStatusSpriteCollisionHit(ppu) == FALSE);
    ppu_setControlVerticalScrollNametable(ppu, FALSE);
    assert(ppu_getControlHorizontalScrollNametable(ppu) == FALSE);
    assert(ppu_getControlVerticalScrollNametable(ppu) == FALSE);
    assert(ppu_getControlPPUAddressIncrement(ppu) == FALSE);
    assert(ppu_getControlSpriteTileTable(ppu) == FALSE);
    assert(ppu_getControlBackgroundTileTable(ppu) == FALSE);
    assert(ppu_getControlSpriteSize(ppu) == FALSE);
    assert(ppu_getControlNMI(ppu) == FALSE);
    assert(ppu_getMaskDisplayType(ppu) == FALSE);
    assert(ppu_getMaskBackgroundClipping(ppu) == FALSE);
    assert(ppu_getMaskSpriteClipping(ppu) == FALSE);
    assert(ppu_getMaskBackgroundVisibility(ppu) == FALSE);
    assert(ppu_getMaskSpriteVisibility(ppu) == FALSE);
    assert(cpu_getMaskIntensifyRed(ppu) == FALSE);
    assert(cpu_getMaskIntensifyGren(ppu) == FALSE);
    assert(cpu_getMaskIntensifyBlue(ppu) == FALSE);
    assert(ppu_getStatusVerticalBlank(ppu) == FALSE);
    assert(ppu_getStatusSpriteOverflow(ppu) == FALSE);
    assert(ppu_getStatusSpriteCollisionHit(ppu) == FALSE);
  }
}