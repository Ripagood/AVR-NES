#include "memory_type.h"
Memory ppuMemory_init(void);
#define PPU_FIRST_REAL_ADDRESS 0x0000
#define PPU_LAST_REAL_ADDRESS 0x3FFF
#define PPU_NUM_REAL_ADDRESSES (PPU_LAST_REAL_ADDRESS + 1)
/*
        |    $0000      |    $0FFF      |    $1000      | Tile Set #0           |
        |    $1000      |    $1FFF      |    $1000      | Tile Set #1           |
        +---------------+---------------+---------------+-----------------------+
        |    $2000      |    $23FF      |    $0400      | Name Table #0         |
        |    $2400      |    $27FF      |    $0400      | Name Table #1         |
        |    $2800      |    $2BFF      |    $0400      | Name Table #2         |
        |    $2C00      |    $2FFF      |    $0400      | Name Table #3         |
        +---------------+---------------+---------------+-----------------------+
        |    $3000      |    $3EFF      |    $3EFF      | Name Table Mirror *1 |
        |    $3F00      |    $3FFF      |    $0020      | Palette *2            |
        |    $4000      |    $FFFF      |    $C000      | Mirrors of Above *3   |
*/
// tile set 0
#define PPU_PATTERN_TABLE_0_FIRST_ADDRESS 0x0000
#define PPU_PATTERN_TABLE_0_LAST_ADDRESS 0x0FFF
// tile set 1
#define PPU_PATTERN_TABLE_1_FIRST_ADDRESS 0x1000
#define PPU_PATTERN_TABLE_1_LAST_ADDRESS 0x1FFF
#define PPU_NAME_TABLE_0_FIRST_ADDRESS 0x2000
#define PPU_NAME_TABLE_0_LAST_ADDRESS 0x23FF
#define PPU_NAME_TABLE_1_FIRST_ADDRESS 0x2400
#define PPU_NAME_TABLE_1_LAST_ADDRESS 0x27FF
#define PPU_NAME_TABLE_2_FIRST_ADDRESS 0x2800
#define PPU_NAME_TABLE_2_LAST_ADDRESS 0x2BFF
#define PPU_NAME_TABLE_3_FIRST_ADDRESS 0x2C00
#define PPU_NAME_TABLE_3_LAST_ADDRESS 0x2FFF
#define PPU_NAME_TABLE_SIZE (PPU_NAME_TABLE_0_LAST_ADDRESS -PPU_NAME_TABLE_0_FIRST_ADDRESS + 1)
#define PPU_NAME_TABLE_TOTAL_SIZE (PPU_NAME_TABLE_3_LAST_ADDRESS -PPU_NAME_TABLE_0_FIRST_ADDRESS + 1)
#define PPU_NAME_TABLE_MIRROR_FIRST_ADDRESS 0x3000
#define PPU_NAME_TABLE_MIRROR_LAST_ADDRESS 0x3EFF
/*
The palette for the background runs from VRAM $3F00 to $3F0F;
$3F00   Universal background color
$3F01-$3F03    Background palette 0
$3F05-$3F07    Background palette 1
$3F09-$3F0B    Background palette 2
$3F0D-$3F0F    Background palette 3
Addresses $3F04/$3F08/$3F0C can contain unique data,
though these values are not used by the PPU when rendering.
*/
#define PPU_BACKGROUND_PALETTE_FIRST_ADDRESS   0x3F00
#define PPU_BACKGROUND_PALETTE_LAST_ADDRESS    0x3F0F
#define PPU_BACKGROUND_PALETTE_TOTAL_SIZE (PPU_BACKGROUND_PALETTE_LAST_ADDRESS -PPU_BACKGROUND_PALETTE_FIRST_ADDRESS + 1)
#define PPU_BACKGROUND_PALLETE_0_FIRST_ADDRESS 0x3F00
#define PPU_BACKGROUND_PALETTE_0_LAST_ADDRESS 0x3F03
#define PPU_BACKGROUND_PALETTE_1_FIRST_ADDRESS 0x3F04
#define PPU_BACKGROUND_PALETTE_1_LAST_ADDRESS 0x3F07
#define PPU_BACKGROUND_PALETTE_2_FIRST_ADDRESS 0x3F08
#define PPU_BACKGROUND_PALETTE_2_LAST_ADDRESS 0x3F0B
#define PPU_BACKGROUND_PALETTE_3_FIRST_ADDRESS 0x3F0C
#define PPU_BACKGROUND_PALETTE_3_LAST_ADDRESS 0x3F0F
/*
the palette for the sprites runs from $3F10 to $3F1F.
$3F11-$3F13    Sprite palette 0
$3F15-$3F17    Sprite palette 1
$3F19-$3F1B    Sprite palette 2
$3F1D-$3F1F    Sprite palette 3
*/
#define PPU_SPRITE_PALETTE_FIRST_ADDRESS 0x3F10
#define PPU_SPRITE_PALETTE_LAST_ADDRESS 0x3F1F
#define PPU_SPRITE_PALETTE_0_FIRST_ADDRESS 0x3F10
#define PPU_SPRITE_PALETTE_0_LAST_ADDRESS 0x3F13
#define PPU_SPRITE_PALETTE_1_FIRST_ADDRESS 0x3F14
#define PPU_SPRITE_PALETTE_1_LAST_ADDRESS 0x3F17
#define PPU_SPRITE_PALETTE_2_FIRST_ADDRESS 0x3F18
#define PPU_SPRITE_PALETTE_2_LAST_ADDRESS 0x3F1B
#define PPU_SPRITE_PALETTE_3_FIRST_ADDRESS 0x3F1C
#define PPU_SPRITE_PALETTE_3_LAST_ADDRESS 0x3F1F
#define PPU_GENUINE_PALETTE_LAST_ADDRESS PPU_SPRITE_PALETTE_3_LAST_ADDRESS
#define PPU_PALETTE_MIRROR_FIRST_ADDRESS   (PPU_GENUINE_PALETTE_LAST_ADDRESS + 1) //0x3F20
#define PPU_PALETTE_MIRROR_LAST_ADDRESS 0x3FFF
#define PPU_SPRITE_PALETTE_TOTAL_SIZE (PPU_SPRITE_PALETTE_LAST_ADDRESS -PPU_SPRITE_PALETTE_FIRST_ADDRESS + 1)
#define PPU_PALETTE_MIRRORED_SIZE (PPU_BACKGROUND_PALETTE_TOTAL_SIZE +PPU_SPRITE_PALETTE_TOTAL_SIZE)
