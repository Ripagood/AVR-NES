#ifndef GLOBALS_H
  #define GLOBALS_H
  #include <stdint.h>
  #define MASK_BIT0 1
  #define MASK_BIT1 2
  #define MASK_BIT2 4
  #define MASK_BIT3 8
  #define MASK_BIT4 16
  #define MASK_BIT5 32
  #define MASK_BIT6 64
  #define MASK_BIT7 128
  #define MASK_BIT8 256
  #define BITS_PER_BYTE 8
  #define BYTES_PER_KILOBYTE 1024
  typedef enum {
    FALSE = 0,
    TRUE = 1
  } Boolean;
  typedef uint8_t Byte;
  typedef int8_t SignedByte;
  typedef uint16_t Word;
  typedef int16_t SignedWord;
  typedef uint16_t Address;
  #define GET_ADDRESS_HIGH_BYTE(X) ((Byte) (X >> BITS_PER_BYTE))
  #define GET_ADDRESS_LOW_BYTE(X) ((Byte) X)
  void debug_printf (const char *format, ... );
#endif