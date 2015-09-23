#include <assert.h>
#include "cpu_type.h"
#include "globals.h"
#include "nes_type.h"
#define MASK_STATUS_ZERO_ON MASK_BIT1
#define MASK_STATUS_ZERO_OFF (~MASK_BIT1)
#define MASK_STATUS_DECIMAL_ON MASK_BIT3
#define MASK_STATUS_DECIMAL_OFF (~MASK_BIT3)
#define MASK_STATUS_NEGATIVE_ON MASK_BIT7
#define MASK_STATUS_NEGATIVE_OFF (~MASK_BIT7)
#define MASK_STATUS_OVERFLOW_ON MASK_BIT6
#define MASK_STATUS_OVERFLOW_OFF (~MASK_BIT6)
#define MASK_STATUS_CARRY_ON MASK_BIT0
#define MASK_STATUS_CARRY_OFF (~MASK_BIT0)
#define MASK_STATUS_INTERRUPT_ON MASK_BIT2
#define MASK_STATUS_INTERRUPT_OFF (~MASK_BIT2)
#define MASK_STATUS_BREAK_ON MASK_BIT4
#define MASK_STATUS_BREAK_OFF (~MASK_BIT4)
#define CPU_STATUS_REGISTER_INITIAL_VALUE MASK_BIT5
#define CPU_STACK_POINTER_INITIAL_VALUE 0xFF
#define GET_STACK_ADDRESS(X) ((Address) 0x0100 + X)
#define STACK_ADDRESS_TOP 0x01FF
#define STACK_ADDRESS_BOTTOM 0x0100
#define VALIDATE_STACK_ADDRESS(X) {assert(X <= STACK_ADDRESS_TOP); assert(X >=
STACK_ADDRESS_BOTTOM);}
#define VALIDATE_STACK_POINTER(X) VALIDATE_STACK_ADDRESS(GET_STACK_ADDRESS(X))
CPU cpu_init(void);
void cpu_destroy(CPU cpu);
Address cpu_getProgramCounter(CPU cpu);
Byte cpu_getStackPointer(CPU cpu);
Byte cpu_getStatus(CPU cpu);
Byte cpu_getIndexX(CPU cpu);
Byte cpu_getIndexY(CPU cpu);
Byte cpu_getAccumulator(CPU cpu);
void cpu_setProgramCounter(CPU cpu, Address programCounter);
void cpu_setStackPointer(CPU cpu, Byte stackPointer);
void cpu_setStatus(CPU cpu, Byte status);
void cpu_setIndexX(CPU cpu, Byte indexX);
void cpu_setIndexY(CPU cpu, Byte indexY);
void cpu_setAccumulator(CPU cpu, Byte accumulator);
void cpu_step(NES nes);
void cpu_tests(void);
void cpu_handleInterrupt(NES nes, Address handlerLowByte, Boolean fromBRK);
Boolean cpu_getInterruptDisable(CPU cpu);