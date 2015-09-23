#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "globals.h"
#include "cpu.h"
#include "nes.h"
#include "instructions.h"
#include "cpuMemory.h"
// many comments below are based on operational details from
http://www.obelisk.demon.co.uk/6502/
struct cpu {
  Address programCounter;
  Byte stackPointer;
  Byte status;
  Byte indexX;
  Byte indexY;
  Byte accumulator;
};
CPU cpu_init(void) {
  CPU cpu = (CPU) malloc(sizeof(struct cpu));
  assert(cpu != NULL);
  cpu->programCounter = 0;
  cpu->status = CPU_STATUS_REGISTER_INITIAL_VALUE;
  cpu->indexX = 0;
  cpu->indexY = 0;
  cpu->accumulator = 0;
  cpu->stackPointer = CPU_STACK_POINTER_INITIAL_VALUE;
  return cpu;
}
void cpu_destroy(CPU cpu) {
  assert(cpu != NULL);
  free(cpu);
}
Address cpu_getProgramCounter(CPU cpu) {
  assert(cpu != NULL);
  return cpu->programCounter;
}
Byte cpu_getStackPointer(CPU cpu) {
  assert(cpu != NULL);
  return cpu->stackPointer;
}
Byte cpu_getStatus(CPU cpu) {
  assert(cpu != NULL);
  // 1 at all times
  cpu->status |= MASK_BIT5;
  return cpu->status;
}
Byte cpu_getIndexX(CPU cpu) {
  assert(cpu != NULL);
  return cpu->indexX;
}
Byte cpu_getIndexY(CPU cpu) {
  assert(cpu != NULL);
  return cpu->indexY;
}
Byte cpu_getAccumulator(CPU cpu) {
  assert(cpu != NULL);
  return cpu->accumulator;
}
void cpu_setProgramCounter(CPU cpu, Address programCounter) {
  assert(cpu != NULL);
  cpu->programCounter = programCounter;
}
void cpu_setStackPointer(CPU cpu, Byte stackPointer) {
  assert(cpu != NULL);
  cpu->stackPointer = stackPointer;
}
void cpu_setStatus(CPU cpu, Byte status) {
  assert(cpu != NULL);
  // 1 at all times
  status |= MASK_BIT5;
  cpu->status = status;
}
void cpu_setIndexX(CPU cpu, Byte indexX) {
  assert(cpu != NULL);
  cpu->indexX = indexX;
}
void cpu_setIndexY(CPU cpu, Byte indexY) {
  assert(cpu != NULL);
  cpu->indexY = indexY;
}
void cpu_setAccumulator(CPU cpu, Byte accumulator) {
  assert(cpu != NULL);
  cpu->accumulator = accumulator;
}
static void cpu_increaseProgramCounter(CPU cpu) {
  assert(cpu != NULL);
  cpu->programCounter++;
}
static void cpu_JMP(NES nes, Address address) {
  assert(nes != NULL);
  CPU cpu = nes_getCPU(nes);
  assert(cpu != NULL);
  debug_printf("JMP\n");
  cpu->programCounter = address;
}
static Boolean cpu_getZero(CPU cpu) {
  assert(cpu != NULL);
  if ((cpu->status & MASK_STATUS_ZERO_ON) == MASK_STATUS_ZERO_ON) {
    return TRUE;
  } else {
    return FALSE;
  }
}
static void cpu_setZero(CPU cpu, Boolean state) {
  assert(cpu != NULL);
  if (state == TRUE) {
    cpu->status |= MASK_STATUS_ZERO_ON;
  } else if (state == FALSE) {
    cpu->status &= MASK_STATUS_ZERO_OFF;
  }
}
//Z     Zero Flag      Set if A = 0
static void cpu_updateZero(CPU cpu, Byte data) {
  assert(cpu != NULL);
  if (data == 0) {
    cpu_setZero(cpu, TRUE);
  } else {
    cpu_setZero(cpu, FALSE);
  }
}
static void cpu_setDecimal(CPU cpu, Boolean state) {
  assert(cpu != NULL);
  if (state == TRUE) {
    cpu->status |= MASK_STATUS_DECIMAL_ON;
  } else if (state == FALSE) {
    cpu->status &= MASK_STATUS_DECIMAL_OFF;
  }
}
static Boolean cpu_getDecimal(CPU cpu) {
  assert(cpu != NULL);
  if ((cpu->status & MASK_STATUS_DECIMAL_ON) == MASK_STATUS_DECIMAL_ON) {
    return TRUE;
  } else {
    return FALSE;
  }
}
// SED - Set Decimal Flag
static void cpu_SED(CPU cpu) {
  assert(cpu != NULL);
  cpu_setDecimal(cpu, TRUE);
}
// CLD - Clear Decimal Mode
static void cpu_CLD(CPU cpu) {
  assert(cpu != NULL);
  cpu_setDecimal(cpu, FALSE);
}
static Boolean cpu_getNegative(CPU cpu) {
  assert(cpu != NULL);
  if ((cpu->status & MASK_STATUS_NEGATIVE_ON) == MASK_STATUS_NEGATIVE_ON) {
    return TRUE;
  } else {
    return FALSE;
  }
}
static void cpu_setNegative(CPU cpu, Boolean state) {
  assert(cpu != NULL);
  if (state == TRUE) {
    cpu->status |= MASK_STATUS_NEGATIVE_ON;
  } else if (state == FALSE) {
    cpu->status &= MASK_STATUS_NEGATIVE_OFF;
  }
}
//N     Negative Flag Set if bit 7 of A is set
static void cpu_updateNegative(CPU cpu, Byte data) {
  assert(cpu != NULL);
  if ((data & MASK_BIT7) == MASK_BIT7) {
    cpu_setNegative(cpu, TRUE);
  } else {
    cpu_setNegative(cpu, FALSE);
  }
}
// LDA - Load Accumulator
// Loads a byte of memory into the accumulator setting the zero and negative flags as
appropriate.
static void cpu_LDA(NES nes, Address address) {
  assert(nes != NULL);
  CPU cpu = nes_getCPU(nes);
  assert(cpu != NULL);
  debug_printf("LDA\n");
  Byte data = nes_readCPUMemory(nes, address);
  cpu_updateZero(cpu, data);
  cpu_updateNegative(cpu, data);
  cpu_setAccumulator(cpu, data);
}
// STA - Store Accumulator
// Stores the contents of the accumulator into memory.
static void cpu_STA(NES nes, Address address) {
  assert(nes != NULL);
  CPU cpu = nes_getCPU(nes);
  assert(cpu != NULL);
  debug_printf("STA\n");
  nes_writeCPUMemory(nes, address, cpu->accumulator);
}
// LDX - Load X Register
// Loads a byte of memory into the X register setting the zero and negative flags as
appropriate.
static void cpu_LDX(NES nes, Address address) {
  assert(nes != NULL);
  CPU cpu = nes_getCPU(nes);
  assert(cpu != NULL);
  debug_printf("LDX\n");
  Byte data = nes_readCPUMemory(nes, address);
  cpu_updateZero(cpu, data);
  cpu_updateNegative(cpu, data);
  cpu_setIndexX(cpu, data);
}
// STX - Store X Register
// Stores the contents of the X register into memory.
static void cpu_STX(NES nes, Address address) {
  assert(nes != NULL);
  CPU cpu = nes_getCPU(nes);
  assert(cpu != NULL);
  debug_printf("STX\n");
  nes_writeCPUMemory(nes, address, cpu->indexX);
}
// LDY - Load Y Register
// Loads a byte of memory into the Y register setting the zero and negative flags as
appropriate.
static void cpu_LDY(NES nes, Address address) {
  assert(nes != NULL);
  CPU cpu = nes_getCPU(nes);
  assert(cpu != NULL);
  debug_printf("LDY\n");
  Byte data = nes_readCPUMemory(nes, address);
  cpu_updateZero(cpu, data);
  cpu_updateNegative(cpu, data);
  cpu_setIndexY(cpu, data);
}
// STY - Store Y Register
// Stores the contents of the Y register into memory.
static void cpu_STY(NES nes, Address address) {
  assert(nes != NULL);
  CPU cpu = nes_getCPU(nes);
  assert(cpu != NULL);
  debug_printf("STY\n");
  nes_writeCPUMemory(nes, address, cpu->indexY);
}
// AND - Logical AND
// A logical AND is performed, bit by bit, on the accumulator contents using the
contents of a byte of memory.
static void cpu_AND(NES nes, Address address) {
  assert(nes != NULL);
  CPU cpu = nes_getCPU(nes);
  assert(cpu != NULL);
  debug_printf("AND\n");
  Byte data = nes_readCPUMemory(nes, address);
  cpu->accumulator &= data;
  cpu_updateZero(cpu, cpu->accumulator);
  cpu_updateNegative(cpu, cpu->accumulator);
}
// ORA - Logical Inclusive OR
// An inclusive OR is performed, bit by bit, on the accumulator contents using the
contents of a byte of memory.
static void cpu_ORA(NES nes, Address address) {
  assert(nes != NULL);
  CPU cpu = nes_getCPU(nes);
  assert(cpu != NULL);
  debug_printf("ORA\n");
  Byte data = nes_readCPUMemory(nes, address);
  cpu->accumulator |= data;
  cpu_updateZero(cpu, cpu->accumulator);
  cpu_updateNegative(cpu, cpu->accumulator);
}
// EOR - Exclusive OR
// An exclusive OR is performed, bit by bit, on the accumulator contents using the
contents of a byte of memory.
static void cpu_EOR(NES nes, Address address) {
  assert(nes != NULL);
  CPU cpu = nes_getCPU(nes);
  assert(cpu != NULL);
  debug_printf("EOR\n");
  Byte data = nes_readCPUMemory(nes, address);
  cpu->accumulator ^= data;
  cpu_updateZero(cpu, cpu->accumulator);
  cpu_updateNegative(cpu, cpu->accumulator);
}
static void cpu_setOverflow(CPU cpu, Boolean state) {
  assert(cpu != NULL);
  if (state == TRUE) {
    cpu->status |= MASK_STATUS_OVERFLOW_ON;
  } else if (state == FALSE) {
    cpu->status &= MASK_STATUS_OVERFLOW_OFF;
  }
}
static Boolean cpu_getOverflow(CPU cpu) {
  assert(cpu != NULL);
  if ((cpu->status & MASK_STATUS_OVERFLOW_ON) == MASK_STATUS_OVERFLOW_ON) {
    return TRUE;
  } else {
    return FALSE;
  }
}
// BIT - Bit Test
// This instructions is used to test if one or more bits are set in a target memory
location.
// The mask pattern in A is ANDed with the value in memory to set or clear the zero flag,
but the result is not kept.
// Bits 7 and 6 of the value from memory are copied into the N and V flags.
static void cpu_BIT(NES nes, Address address) {
  assert(nes != NULL);
  CPU cpu = nes_getCPU(nes);
  assert(cpu != NULL);
  debug_printf("BIT\n");
  Byte data = nes_readCPUMemory(nes, address);
  cpu_updateZero(cpu, cpu->accumulator & data);
  cpu_updateNegative(cpu, data);
  if ((data & MASK_BIT6) == MASK_BIT6) {
    cpu_setOverflow(cpu, TRUE);
  } else {
    cpu_setOverflow(cpu, FALSE);
  }
}
static void cpu_setCarry(CPU cpu, Boolean state) {
  assert(cpu != NULL);
  if (state == TRUE) {
    cpu->status |= MASK_STATUS_CARRY_ON;
  } else if (state == FALSE) {
    cpu->status &= MASK_STATUS_CARRY_OFF;
  }
}
static Boolean cpu_getCarry(CPU cpu) {
  assert(cpu != NULL);
  if ((cpu->status & MASK_STATUS_CARRY_ON) == MASK_STATUS_CARRY_ON) {
    return TRUE;
  } else {
    return FALSE;
  }
}
// set to 0 - if a borrow is required
// set to 1 - if no borrow is required.
static void cpu_updateCarry_subtract(CPU cpu, Byte a, Byte b) {
  assert(cpu != NULL);
  // add a 'borrow' bit to borrow from
  Word aa = a;
  aa += MASK_BIT8;
  Word bb = b;
  Word result = aa-bb;
  if ((result & MASK_BIT8) == MASK_BIT8) {
    // subtraction didn't need to borrow from the 8th bit
    cpu_setCarry(cpu, TRUE);
  } else {
    // needed to borrow from the 8th bit
    cpu_setCarry(cpu, FALSE);
  }
}
// CMP - Compare
// This instruction compares the contents of the accumulator with another memory held
value and sets the zero and carry flags as appropriate.
static void cpu_CMP(NES nes, Address address) {
  assert(nes != NULL);
  CPU cpu = nes_getCPU(nes);
  assert(cpu != NULL);
  debug_printf("CMP\n");
  Byte data = nes_readCPUMemory(nes, address);
  // C         Carry Flag     Set if A >= M
  cpu_updateCarry_subtract(cpu, cpu->accumulator, data);
  // Z Zero Flag       Set if A = M
  cpu_updateZero(cpu, cpu->accumulator - data);
  // N Negative Flag   Set if bit 7 of the result is set
  cpu_updateNegative(cpu, cpu->accumulator - data);
}
// CPX - Compare X Register
// This instruction compares the contents of the X register with another memory held
value and sets the zero and carry flags as appropriate.
static void cpu_CPX(NES nes, Address address) {
  assert(nes != NULL);
  CPU cpu = nes_getCPU(nes);
  assert(cpu != NULL);
  debug_printf("CPX\n");
  Byte data = nes_readCPUMemory(nes, address);
  // C         Carry Flag     Set if X >= M
  cpu_updateCarry_subtract(cpu, cpu->indexX, data);
  // Z Zero Flag       Set if X = M
  cpu_updateZero(cpu, cpu->indexX - data);
  // N Negative Flag   Set if bit 7 of the result is set
  cpu_updateNegative(cpu, cpu->indexX - data);
}
// CPY - Compare Y Register
// This instruction compares the contents of the Y register with another memory held
value and sets the zero and carry flags as appropriate.
static void cpu_CPY(NES nes, Address address) {
  assert(nes != NULL);
  CPU cpu = nes_getCPU(nes);
  assert(cpu != NULL);
  debug_printf("CPY\n");
  Byte data = nes_readCPUMemory(nes, address);
  // C         Carry Flag     Set if Y >= M
  cpu_updateCarry_subtract(cpu, cpu->indexY, data);
  // Z Zero Flag       Set if Y = M
  cpu_updateZero(cpu, cpu->indexY - data);
  // N Negative Flag   Set if bit 7 of the result is set
  cpu_updateNegative(cpu, cpu->indexY - data);
}
// this looks correct to me, but doesn't work on one of the test cases, so i use a
different method now
static void cpu_updateOverflow(CPU cpu, Byte a, Byte b, Byte c) {
  assert(cpu != NULL);
              // positive                    // positive                     // negative
  if ( ((a & MASK_BIT7) == MASK_BIT7) && ((b & MASK_BIT7) == MASK_BIT7) && ((c &
MASK_BIT7) == 0) ) {
    cpu_setOverflow(cpu, TRUE);
               // negative                 // negative                 // positive
 } else if ( ((a & MASK_BIT7) == 0) && ((b & MASK_BIT7) == 0) && ((c & MASK_BIT7) ==
MASK_BIT7) ) {
    cpu_setOverflow(cpu, TRUE);
  } else {
    cpu_setOverflow(cpu, FALSE);
  }
}
// ADC - Add with Carry
// This instruction adds the contents of a memory location to the accumulator together
with the carry bit.
// If overflow occurs the carry bit is set, this enables multiple byte addition to be
performed.
static void cpu_ADC(NES nes, Address address) {
  assert(nes != NULL);
  CPU cpu = nes_getCPU(nes);
  assert(cpu != NULL);
  debug_printf("ADC\n");
  // A + M + C
  Byte data = nes_readCPUMemory(nes, address);
  Word result = cpu->accumulator + data;
  if (cpu_getCarry(cpu) == TRUE) {
    result++;
  }
  // C  Carry Flag     Set if overflow in bit 7
  if ((result & MASK_BIT8) == MASK_BIT8) {
    cpu_setCarry(cpu, TRUE);
  } else {
    cpu_setCarry(cpu, FALSE);
  }
  // V Overflow Flag   Set if sign bit is incorrect
  // based on http://nesdev.parodius.com/6502.txt
  if (((cpu->accumulator ^ result) & 0x80) && !((cpu->accumulator ^ data) & 0x80)) {
    cpu_setOverflow(cpu, TRUE);
  } else {
    cpu_setOverflow(cpu, FALSE);
  }
  cpu->accumulator = result;
  // Z Zero Flag       Set if A = 0
  cpu_updateZero(cpu, cpu->accumulator);
  // N Negative Flag   Set if bit 7 set
  cpu_updateNegative(cpu, cpu->accumulator);
}
// SBC - Subtract with Carry
// This instruction subtracts the contents of a memory location to the accumulator
together with the not of the carry bit.
// If overflow occurs the carry bit is clear, this enables multiple byte subtraction to
be performed.
static void cpu_SBC(NES nes, Address address) {
  assert(nes != NULL);
  CPU cpu = nes_getCPU(nes);
  assert(cpu != NULL);
  debug_printf("SBC\n");
  // A,Z,C,N = A-M-(1-C)
  Byte data = nes_readCPUMemory(nes, address);
  Word result = cpu->accumulator - data;
  if (cpu_getCarry(cpu) == FALSE) {
    result--;
  }
  // C         Carry Flag     Clear if overflow in bit 7
  if ((result & MASK_BIT8) == MASK_BIT8) {
    cpu_setCarry(cpu, FALSE);
  } else {
    cpu_setCarry(cpu, TRUE);
  }
  // V Overflow Flag   Set if sign bit is incorrect
  // based on http://nesdev.parodius.com/6502.txt
  if (((cpu->accumulator ^ result) & 0x80) && ((cpu->accumulator ^ data) & 0x80)) {
    cpu_setOverflow(cpu, TRUE);
  } else {
    cpu_setOverflow(cpu, FALSE);
  }
  cpu->accumulator = result;
  // Z Zero Flag       Set if A = 0
  cpu_updateZero(cpu, cpu->accumulator);
  // N Negative Flag   Set if bit 7 set
  cpu_updateNegative(cpu, cpu->accumulator);
}
// ASL - Arithmetic Shift Left
// This operation shifts all the bits of the memory contents one bit left.
// Bit 0 is set to 0 and bit 7 is placed in the carry flag.
// The effect of this operation is to multiply the memory contents by 2 (ignoring 2's
complement considerations),
// setting the carry if the result will not fit in 8 bits.
static void cpu_ASL_memory(NES nes, Address address) {
  assert(nes != NULL);
  CPU cpu = nes_getCPU(nes);
  assert(cpu != NULL);
  debug_printf("ASL_memory\n");
  // A,Z,C,N = M*2 or M,Z,C,N = M*2
  Byte data = nes_readCPUMemory(nes, address);
  // dummy write, cycle wasted here
  nes_writeCPUMemory(nes, address, data);
  Word result = data << 1;
  // C Carry Flag      Set to contents of old bit 7
  if ( (data & MASK_BIT7) == MASK_BIT7) {
    cpu_setCarry(cpu, TRUE);
  } else {
    cpu_setCarry(cpu, FALSE);
  }
  // Z Zero Flag       Set if A = 0
  cpu_updateZero(cpu, result);
  // N Negative Flag   Set if bit 7 of the result is set
  cpu_updateNegative(cpu, result);
  nes_writeCPUMemory(nes, address, result);
}
// LSR - Logical Shift Right
// Each of the bits in A or M is shift one place to the right.
// The bit that was in bit 0 is shifted into the carry flag.
// Bit 7 is set to zero.
static void cpu_LSR_memory(NES nes, Address address) {
  assert(nes != NULL);
  CPU cpu = nes_getCPU(nes);
  assert(cpu != NULL);
  debug_printf("LSR_memory\n");
  // A,C,Z,N = A/2 or M,C,Z,N = M/2
  Byte data = nes_readCPUMemory(nes, address);
  // dummy write, cycle wasted here
  nes_writeCPUMemory(nes, address, data);
  Word result = data >> 1;
  // C         Carry Flag     Set to contents of old bit 0
  if ( (data & MASK_BIT0) == MASK_BIT0) {
    cpu_setCarry(cpu, TRUE);
  } else {
    cpu_setCarry(cpu, FALSE);
  }
  // Z Zero Flag       Set if result = 0
  cpu_updateZero(cpu, result);
  // N Negative Flag   Set if bit 7 of the result is set
  cpu_updateNegative(cpu, result);
  nes_writeCPUMemory(nes, address, result);
}
// ROL - Rotate Left
// Move each of the bits in either A or M one place to the left.
// Bit 0 is filled with the current value of the carry flag whilst the old bit 7 becomes
the new carry flag value.
static void cpu_ROL_memory(NES nes, Address address) {
  assert(nes != NULL);
  CPU cpu = nes_getCPU(nes);
  assert(cpu != NULL);
  debug_printf("ROL_memory\n");
  Byte data = nes_readCPUMemory(nes, address);
  // dummy write, cycle wasted here
  nes_writeCPUMemory(nes, address, data);
  Word result  = data << 1;
  if (cpu_getCarry(cpu) == TRUE) {
    result |= MASK_BIT0;
  }
  // C         Carry Flag     Set to contents of old bit 7
  if ((data & MASK_BIT7) == MASK_BIT7) {
    cpu_setCarry(cpu, TRUE);
  } else {
    cpu_setCarry(cpu, FALSE);
  }
  // Z Zero Flag       Set if A = 0
  cpu_updateZero(cpu, result);
  // N Negative Flag   Set if bit 7 of the result is set
  cpu_updateNegative(cpu, result);
  nes_writeCPUMemory(nes, address, result);
}
// ROR - Rotate Right
// Move each of the bits in either A or M one place to the right.
// Bit 7 is filled with the current value of the carry flag whilst the old bit 0 becomes
the new carry flag value.
static void cpu_ROR_memory(NES nes, Address address) {
  assert(nes != NULL);
  CPU cpu = nes_getCPU(nes);
  assert(cpu != NULL);
  debug_printf("ROR_memory\n");
  Byte data = nes_readCPUMemory(nes, address);
  // dummy write, cycle wasted here
  nes_writeCPUMemory(nes, address, data);
  Word result = data >> 1;
  if (cpu_getCarry(cpu) == TRUE) {
    result |= MASK_BIT7;
  }
  // C         Carry Flag      Set to contents of old bit 0
  if ((data & MASK_BIT0) == MASK_BIT0) {
    cpu_setCarry(cpu, TRUE);
  } else {
    cpu_setCarry(cpu, FALSE);
  }
  // Z Zero Flag       Set if A = 0
  cpu_updateZero(cpu, result);
  // N Negative Flag   Set if bit 7 of the result is set
  cpu_updateZero(cpu, result);
  nes_writeCPUMemory(nes, address, result);
}
// INC - Increment Memory
// Adds one to the value held at a specified memory location setting the zero and
negative flags as appropriate.
static void cpu_INC(NES nes, Address address) {
  assert(nes != NULL);
  CPU cpu = nes_getCPU(nes);
  assert(cpu != NULL);
  debug_printf("INC\n");
  Byte data = nes_readCPUMemory(nes, address);
  // dummy write, cycle wasted here
  nes_writeCPUMemory(nes, address, data);
  data++;
  // Z Zero Flag       Set if result is zero
  cpu_updateZero(cpu, data);
  // N Negative Flag   Set if bit 7 of the result is set
  cpu_updateNegative(cpu, data);
  nes_writeCPUMemory(nes, address, data);
}
// DEC - Decrement Memory
// Subtracts one from the value held at a specified memory location setting the zero and
negative flags as appropriate.
static void cpu_DEC(NES nes, Address address) {
  assert(nes != NULL);
  CPU cpu = nes_getCPU(nes);
  assert(cpu != NULL);
  debug_printf("DEC\n");
  Byte data = nes_readCPUMemory(nes, address);
  // dummy write, cycle wasted here
  nes_writeCPUMemory(nes, address, data);
  data--;
  // Z Zero Flag       Set if result is zero
  cpu_updateZero(cpu, data);
  // N Negative Flag   Set if bit 7 of the result is set
  cpu_updateNegative(cpu, data);
  nes_writeCPUMemory(nes, address, data);
}
// The processor supports a 256 byte stack located between $0100 and $01FF.
// The stack is located at memory locations $0100-$01FF.
// The stack pointer is an 8-bit register which serves as an offset from $0100.
// The stack pointer is an 8 bit register and holds the low 8 bits of the next free
location on the stack.
// Pushing bytes to the stack causes the stack pointer to be decremented. Conversely
pulling bytes causes it to be incremented.
// There is no detection of stack overflow and the stack pointer will just wrap around
from $00 to $FF.
static void cpu_pushStack(NES nes, Byte data) {
  assert(nes != NULL);
  CPU cpu = nes_getCPU(nes);
  assert(cpu != NULL);
  Address stackAddress = GET_STACK_ADDRESS(cpu->stackPointer);
  VALIDATE_STACK_ADDRESS(stackAddress);
  nes_writeCPUMemory(nes, stackAddress, data);
  cpu->stackPointer--;
  VALIDATE_STACK_POINTER(cpu->stackPointer);
}
static Byte cpu_popStack(NES nes) {
  assert(nes != NULL);
  CPU cpu = nes_getCPU(nes);
  assert(cpu != NULL);
  VALIDATE_STACK_POINTER(cpu->stackPointer);
  cpu->stackPointer++;
  VALIDATE_STACK_POINTER(cpu->stackPointer);
  Address stackAddress = GET_STACK_ADDRESS(cpu->stackPointer);
  VALIDATE_STACK_ADDRESS(stackAddress);
  Byte data = nes_readCPUMemory(nes, stackAddress);
  return data;
}
// JSR - Jump to Subroutine
// The JSR instruction pushes the address (minus one) of the return point on to the
stack and then sets the program counter to the target memory address.
static void cpu_JSR(NES nes, Address address) {
  assert(nes != NULL);
  CPU cpu = nes_getCPU(nes);
  assert(cpu != NULL);
  debug_printf("JSR\n");
  // During a JSR, the address pushed onto the stack is that of the 3rd byte of the
instruction - that is, 1 byte BEFORE the next instruction.
  // This is because it pushes the program counter onto the stack BEFORE it fetches the
final byte of the opcode
  // (and, as such, before it can increment the PC past this point).
  // To compensate for this, the RTS opcode increments the program counter during its
6th instruction cycle.
  // JSR takes 6 cycles, waste a cycle here
  nes_cpuCycled(nes);
  Address returnAddress = cpu->programCounter - 1;
  cpu_pushStack(nes, GET_ADDRESS_HIGH_BYTE(returnAddress));
  cpu_pushStack(nes, GET_ADDRESS_LOW_BYTE(returnAddress));
  cpu->programCounter = address;
}
// ASL - Arithmetic Shift Left
// This operation shifts all the bits of the accumulator one bit left.
// Bit 0 is set to 0 and bit 7 is placed in the carry flag.
// The effect of this operation is to multiply the memory contents by 2
// (ignoring 2's complement considerations), setting the carry if the result will not
fit in 8 bits.
static void cpu_ASL(CPU cpu) {
  assert(cpu != NULL);
  debug_printf("ASL\n");
  Word result = cpu->accumulator << 1;
  // C         Carry Flag      Set to contents of old bit 7
  if ((cpu->accumulator & MASK_BIT7) == MASK_BIT7) {
    cpu_setCarry(cpu, TRUE);
  } else {
    cpu_setCarry(cpu, FALSE);
  }
  cpu->accumulator = result;
  // Z Zero Flag       Set if A = 0
  cpu_updateZero(cpu, cpu->accumulator);
  // N Negative Flag   Set if bit 7 of the result is set
  cpu_updateNegative(cpu, cpu->accumulator);
}
// LSR - Logical Shift Right
// Each of the bits in A is shift one place to the right.
// The bit that was in bit 0 is shifted into the carry flag. Bit 7 is set to zero.
static void cpu_LSR(CPU cpu) {
  assert(cpu != NULL);
  debug_printf("LSR\n");
  Word result = cpu->accumulator >> 1;
  // C         Carry Flag      Set to contents of old bit 0
  if ( (cpu->accumulator & MASK_BIT0) == MASK_BIT0 ) {
    cpu_setCarry(cpu, TRUE);
  } else {
    cpu_setCarry(cpu, FALSE);
  }
  cpu->accumulator = result;
  // Z Zero Flag       Set if result = 0
  cpu_updateZero(cpu, cpu->accumulator);
  // N Negative Flag   Set if bit 7 of the result is set
  cpu_updateNegative(cpu, cpu->accumulator);
}
// ROL - Rotate Left
// Move each of the bits in either A or M one place to the left.
// Bit 0 is filled with the current value of the carry flag whilst the old bit 7 becomes
the new carry flag value.
static void cpu_ROL(CPU cpu) {
  assert(cpu != NULL);
  debug_printf("ROL\n");
  Word result = cpu->accumulator << 1;
  if (cpu_getCarry(cpu) == TRUE) {
    result |= MASK_BIT0;
  }
  if ( (cpu->accumulator & MASK_BIT7) == MASK_BIT7) {
    cpu_setCarry(cpu, TRUE);
  } else {
    cpu_setCarry(cpu, FALSE);
  }
  // Z Zero Flag       Set if A = 0
  cpu->accumulator = result;
  cpu_updateZero(cpu, cpu->accumulator);
  // N Negative Flag   Set if bit 7 of the result is set
  cpu_updateNegative(cpu, cpu->accumulator);
}
// ROR - Rotate Right
// Move each of the bits in either A or M one place to the right.
// Bit 7 is filled with the current value of the carry flag whilst the old bit 0 becomes
the new carry flag value.
static void cpu_ROR(CPU cpu) {
  assert(cpu != NULL);
  debug_printf("ROR\n");
  Word result = cpu->accumulator >> 1;
  if (cpu_getCarry(cpu) == TRUE) {
    result |= MASK_BIT7;
  }
  // C         Carry Flag     Set to contents of old bit 0
  if ( (cpu->accumulator & MASK_BIT0) == MASK_BIT0) {
    cpu_setCarry(cpu, TRUE);
  } else {
    cpu_setCarry(cpu, FALSE);
  }
  cpu->accumulator = result;
  // Z Zero Flag       Set if A = 0
  cpu_updateZero(cpu, cpu->accumulator);
  // N Negative Flag   Set if bit 7 of the result is set
  cpu_updateNegative(cpu, cpu->accumulator);
}
static void cpu_setInterruptDisable(CPU cpu, Boolean state) {
  assert(cpu != NULL);
  if (state == TRUE) {
    cpu->status |= MASK_STATUS_INTERRUPT_ON;
  } else if (state == FALSE) {
    cpu->status &= MASK_STATUS_INTERRUPT_OFF;
  }
}
Boolean cpu_getInterruptDisable(CPU cpu) {
  assert(cpu != NULL);
  if ((cpu->status & MASK_STATUS_INTERRUPT_ON) == MASK_STATUS_INTERRUPT_ON) {
    return TRUE;
  } else {
    return FALSE;
  }
}
static void cpu_setBreak(CPU cpu, Boolean state) {
  assert(cpu != NULL);
  if (state == TRUE) {
    cpu->status |= MASK_STATUS_BREAK_ON;
  } else if (state == FALSE) {
    cpu->status &= MASK_STATUS_BREAK_OFF;
  }
}
static Boolean cpu_getBreak(CPU cpu) {
  assert(cpu != NULL);
  if ( (cpu->status & MASK_STATUS_BREAK_ON) == MASK_STATUS_BREAK_ON) {
    return TRUE;
  } else {
    return FALSE;
  }
}
void cpu_handleInterrupt(NES nes, Address handlerLowByte, Boolean fromBRK) {
  assert(nes != NULL);
  CPU cpu = nes_getCPU(nes);
  assert(cpu != NULL);
  debug_printf("handleInterrupt\n");
  cpu_setInterruptDisable(cpu, TRUE);
  if (fromBRK == TRUE) {
    cpu_setBreak(cpu, TRUE);
  } else {
    cpu_setBreak(cpu, FALSE);
  }
  cpu_pushStack(nes, GET_ADDRESS_HIGH_BYTE(cpu->programCounter));
  cpu_pushStack(nes, GET_ADDRESS_LOW_BYTE(cpu->programCounter));
  // 1 at all times
  cpu->status |= MASK_BIT5;
  cpu_pushStack(nes, cpu->status);
  Address address = nes_readCPUMemory(nes, handlerLowByte);
  address += nes_readCPUMemory(nes, handlerLowByte+1) << BITS_PER_BYTE;
  debug_printf("Address is %d\n", address);
  cpu->programCounter = address;
}
// BRK - Force Interrupt
// The BRK instruction forces the generation of an interrupt request.
// The program counter and processor status are pushed on the stack
// then the IRQ interrupt vector at $FFFE/F is loaded into the PC
// and the break flag in the status set to one.
static void cpu_BRK(NES nes) {
  assert(nes != NULL);
  CPU cpu = nes_getCPU(nes);
  assert(cpu != NULL);
  debug_printf("BRK\n");
  // the instruction has a "useless" second byte we need to skip
  // (added for debugging purposes apparently)
  cpu->programCounter++;
  // B         Break Command          Set to 1
  cpu_handleInterrupt(nes, CPU_IRQ_VECTOR_LOWER_ADDRESS, TRUE);
}
// RTS - Return from Subroutine
// The RTS instruction is used at the end of a subroutine to return to the calling
routine.
// It pulls the program counter (minus one) from the stack.
static void cpu_RTS(NES nes) {
  assert(nes != NULL);
  CPU cpu = nes_getCPU(nes);
  assert(cpu != NULL);
  debug_printf("RTS\n");
  // waste a cycle for the stack pointer increment that is about to happen in popStack
  nes_cpuCycled(nes);
  Address address = cpu_popStack(nes);
  address += cpu_popStack(nes) << BITS_PER_BYTE;
  // waste a cycle to increment the program counter
  nes_cpuCycled(nes);
  address++;
  cpu->programCounter = address;
}
// RTI - Return from Interrupt
// The RTI instruction is used at the end of an interrupt processing routine.
// It pulls the processor flags from the stack followed by the program counter.
static void cpu_RTI(NES nes) {
  assert(nes != NULL);
  CPU cpu = nes_getCPU(nes);
  assert(cpu != NULL);
  debug_printf("RTI\n");
  // waste a cycle for the initial stack pointer increment that is about to happen in
popStack
  nes_cpuCycled(nes);
  cpu->status = cpu_popStack(nes);
  // 1 at all times
  cpu->status |= MASK_BIT5;
  Address address = cpu_popStack(nes);
  address += cpu_popStack(nes) << BITS_PER_BYTE;
  cpu->programCounter = address;
}
// PHP - Push Processor Status
// Pushes a copy of the status flags on to the stack.
static void cpu_PHP(NES nes) {
  assert(nes != NULL);
  CPU cpu = nes_getCPU(nes);
  assert(cpu != NULL);
  debug_printf("PHP\n");
  // The status bits pushed on the stack by PHP have the breakpoint bit set.
  // 1 at all times
  cpu->status |= MASK_BIT5;
  Byte data = cpu->status;
  data |= MASK_STATUS_BREAK_ON;
  //printf("PHP 0x%x 0x%x\n", cpu->status, data);
  cpu_pushStack(nes, data);
}
// PLP - Pull Processor Status
// Pulls an 8 bit value from the stack and into the processor flags.
// The flags will take on new states as determined by the value pulled.
static void cpu_PLP(NES nes) {
  assert(nes != NULL);
  CPU cpu = nes_getCPU(nes);
  assert(cpu != NULL);
  debug_printf("PLP\n");
  // waste a cycle for the stack pointer increment that is about to happen in popStack
  nes_cpuCycled(nes);
  Byte data = cpu_popStack(nes);
  //printf("PLP 0x%x 0x%x\n", cpu->status, data);
  cpu->status = data;
  // 1 at all times
  cpu->status |= MASK_BIT5;
}
// PHA - Push Accumulator
// Pushes a copy of the accumulator on to the stack.
static void cpu_PHA(NES nes) {
  assert(nes != NULL);
  CPU cpu = nes_getCPU(nes);
  assert(cpu != NULL);
  debug_printf("PHA\n");
  cpu_pushStack(nes, cpu->accumulator);
}
// PLA - Pull Accumulator
// Pulls an 8 bit value from the stack and into the accumulator.
// The zero and negative flags are set as appropriate.
static void cpu_PLA(NES nes) {
  assert(nes != NULL);
  CPU cpu = nes_getCPU(nes);
  assert(cpu != NULL);
  debug_printf("PLA\n");
  // waste a cycle for the stack pointer increment that is about to happen in popStack
  nes_cpuCycled(nes);
  cpu->accumulator = cpu_popStack(nes);
  // Z Zero Flag       Set if A = 0
  cpu_updateZero(cpu, cpu->accumulator);
  // N Negative Flag Set if bit 7 of A is set
  cpu_updateNegative(cpu, cpu->accumulator);
}
// INX - Increment X Register
// Adds one to the X register setting the zero and negative flags as appropriate.
static void cpu_INX(CPU cpu) {
  assert(cpu != NULL);
  debug_printf("INX\n");
  cpu->indexX++;
  // Z Zero Flag       Set if X is zero
  cpu_updateZero(cpu, cpu->indexX);
  // N Negative Flag   Set if bit 7 of X is set
  cpu_updateNegative(cpu, cpu->indexX);
}
// DEX - Decrement X Register
// Subtracts one from the X register setting the zero and negative flags as appropriate.
static void cpu_DEX(CPU cpu) {
  assert(cpu != NULL);
  debug_printf("DEX\n");
  cpu->indexX--;
  // Z Zero Flag       Set if X is zero
  cpu_updateZero(cpu, cpu->indexX);
  // N Negative Flag   Set if bit 7 of X is set
  cpu_updateNegative(cpu, cpu->indexX);
}
// INY - Increment Y Register
// Adds one to the Y register setting the zero and negative flags as appropriate.
static void cpu_INY(CPU cpu) {
  assert(cpu != NULL);
  debug_printf("INY\n");
  cpu->indexY++;
  // Z Zero Flag       Set if Y is zero
  cpu_updateZero(cpu, cpu->indexY);
  // N Negative Flag   Set if bit 7 of Y is set
  cpu_updateNegative(cpu, cpu->indexY);
}
// DEY - Decrement Y Register
// Subtracts one from the Y register setting the zero and negative flags as appropriate.
static void cpu_DEY(CPU cpu) {
  assert(cpu != NULL);
  debug_printf("DEY\n");
  cpu->indexY--;
  // Z Zero Flag       Set if Y is zero
  cpu_updateZero(cpu, cpu->indexY);
  // N Negative Flag Set if bit 7 of Y is set
  cpu_updateNegative(cpu, cpu->indexY);
}
// TAX - Transfer Accumulator to X
// Copies the current contents of the accumulator into the X register and sets the zero
and negative flags as appropriate.
static void cpu_TAX(CPU cpu) {
  assert(cpu != NULL);
  debug_printf("TAX\n");
  cpu->indexX = cpu->accumulator;
  // Z Zero Flag       Set if X = 0
  cpu_updateZero(cpu, cpu->indexX);
  // N Negative Flag   Set if bit 7 of X is set
  cpu_updateNegative(cpu, cpu->indexX);
}
// TXA - Transfer X to Accumulator
// Copies the current contents of the X register into the accumulator and sets the zero
and negative flags as appropriate.
static void cpu_TXA(CPU cpu) {
  assert(cpu != NULL);
  debug_printf("TXA\n");
  cpu->accumulator = cpu->indexX;
  // Z Zero Flag       Set if A = 0
  cpu_updateZero(cpu, cpu->accumulator);
  // N Negative Flag Set if bit 7 of A is set
  cpu_updateNegative(cpu, cpu->accumulator);
}
// TYA - Transfer Y to Accumulator
// Copies the current contents of the Y register into the accumulator and sets the zero
and negative flags as appropriate.
static void cpu_TYA(CPU cpu) {
  assert(cpu != NULL);
  debug_printf("TYA\n");
  cpu->accumulator = cpu->indexY;
  // Z Zero Flag       Set if A = 0
  cpu_updateZero(cpu, cpu->accumulator);
  // N Negative Flag Set if bit 7 of A is set
  cpu_updateNegative(cpu, cpu->accumulator);
}
// TSX - Transfer Stack Pointer to X
// Copies the current contents of the stack register into the X register and sets the
zero and negative flags as appropriate.
static void cpu_TSX(CPU cpu) {
  assert(cpu != NULL);
  debug_printf("TSX\n");
  cpu->indexX = cpu->stackPointer;
  // Z Zero Flag       Set if X = 0
  cpu_updateZero(cpu, cpu->indexX);
  // N Negative Flag   Set if bit 7 of X is set
  cpu_updateNegative(cpu, cpu->indexX);
}
// TAY - Transfer Accumulator to Y
// Copies the current contents of the accumulator into the Y register and sets the zero
and negative flags as appropriate.
static void cpu_TAY(CPU cpu) {
  assert(cpu != NULL);
  debug_printf("TAY\n");
  cpu->indexY = cpu->accumulator;
  // Z Zero Flag       Set if Y = 0
  cpu_updateZero(cpu, cpu->indexY);
  // N Negative Flag Set if bit 7 of Y is set
  cpu_updateNegative(cpu, cpu->indexY);
}
// TXS - Transfer X to Stack Pointer
// Copies the current contents of the X register into the stack register.
static void cpu_TXS(CPU cpu) {
  assert(cpu != NULL);
  debug_printf("TXS\n");
  cpu->stackPointer = cpu->indexX;
}
// SEI - Set Interrupt Disable
static void cpu_SEI(CPU cpu) {
  assert(cpu != NULL);
  debug_printf("SEI\n");
  // I         Interrupt Disable      Set to 1
  cpu_setInterruptDisable(cpu, TRUE);
}
// CLI - Clear Interrupt Disable
static void cpu_CLI(CPU cpu) {
  assert(cpu != NULL);
  debug_printf("CLI\n");
  // I         Interrupt Disable      Set to 0
  cpu_setInterruptDisable(cpu, FALSE);
}
// SEC - Set Carry Flag
static void cpu_SEC(CPU cpu) {
  assert(cpu != NULL);
  debug_printf("SEC\n");
  cpu_setCarry(cpu, TRUE);
}
// CLC - Clear Carry Flag
static void cpu_CLC(CPU cpu) {
  assert(cpu != NULL);
  debug_printf("CLC\n");
  // C         Carry Flag     Set to 0
  cpu_setCarry(cpu, FALSE);
}
// CLV - Clear Overflow Flag
static void cpu_CLV(CPU cpu) {
  assert(cpu != NULL);
  debug_printf("CLV\n");
  // V         Overflow Flag          Set to 0
  cpu_setOverflow(cpu, FALSE);
}
// BEQ - Branch if Equal
// If the zero flag is set then add the relative displacement to the program counter to
cause a branch to a new location.
static void cpu_BEQ(NES nes, Address address) {
  assert(nes != NULL);
  CPU cpu = nes_getCPU(nes);
  assert(cpu != NULL);
  debug_printf("BEQ\n");
  SignedByte data = nes_readCPUMemory(nes, address);
  if (cpu_getZero(cpu) == TRUE) {
    // +1 cycle if the branch succeeds
    nes_cpuCycled(nes);
    cpu->programCounter += data;
  }
}
// BNE - Branch if Not Equal
// If the zero flag is clear then add the relative displacement to the program counter
to cause a branch to a new location.
static void cpu_BNE(NES nes, Address address) {
  assert(nes != NULL);
  CPU cpu = nes_getCPU(nes);
  assert(cpu != NULL);
  debug_printf("BNE\n");
  SignedByte data = nes_readCPUMemory(nes, address);
  if (cpu_getZero(cpu) == FALSE) {
    // +1 cycle if the branch succeeds
    nes_cpuCycled(nes);
    cpu->programCounter += data;
  }
}
// BMI - Branch if Minus
// If the negative flag is set then add the relative displacement to the program counter
to cause a branch to a new location.
static void cpu_BMI(NES nes, Address address) {
  assert(nes != NULL);
  CPU cpu = nes_getCPU(nes);
  assert(cpu != NULL);
  debug_printf("BMI\n");
  SignedByte data = nes_readCPUMemory(nes, address);
  if (cpu_getNegative(cpu) == TRUE) {
    // +1 cycle if the branch succeeds
    nes_cpuCycled(nes);
    cpu->programCounter += data;
  }
}
// BPL - Branch if Positive
// If the negative flag is clear then add the relative displacement to the program
counter to cause a branch to a new location.
static void cpu_BPL(NES nes, Address address) {
  assert(nes != NULL);
  CPU cpu = nes_getCPU(nes);
  assert(cpu != NULL);
  debug_printf("BPL\n");
  SignedByte data = nes_readCPUMemory(nes, address);
  if (cpu_getNegative(cpu) == FALSE) {
    // +1 cycle if the branch succeeds
    nes_cpuCycled(nes);
    cpu->programCounter += data;
  }
}
// BCC - Branch if Carry Clear
// If the carry flag is clear then add the relative displacement to the program counter
to cause a branch to a new location.
static void cpu_BCC(NES nes, Address address) {
  assert(nes != NULL);
  CPU cpu = nes_getCPU(nes);
  assert(cpu != NULL);
  debug_printf("BCC\n");
  SignedByte data = nes_readCPUMemory(nes, address);
  if (cpu_getCarry(cpu) == FALSE) {
    // +1 cycle if the branch succeeds
    nes_cpuCycled(nes);
    cpu->programCounter += data;
  }
}
// BCS - Branch if Carry Set
// If the carry flag is set then add the relative displacement to the program counter to
cause a branch to a new location.
static void cpu_BCS(NES nes, Address address) {
  assert(nes != NULL);
  CPU cpu = nes_getCPU(nes);
  assert(cpu != NULL);
  debug_printf("BCS\n");
  SignedByte data = nes_readCPUMemory(nes, address);
  if (cpu_getCarry(cpu) == TRUE) {
    // +1 cycle if the branch succeeds
    nes_cpuCycled(nes);
    cpu->programCounter += data;
  }
}
// BVC - Branch if Overflow Clear
// If the overflow flag is clear then add the relative displacement to the program
counter to cause a branch to a new location.
static void cpu_BVC(NES nes, Address address) {
  assert(nes != NULL);
  CPU cpu = nes_getCPU(nes);
  assert(cpu != NULL);
  debug_printf("BVC\n");
  SignedByte data = nes_readCPUMemory(nes, address);
  if (cpu_getOverflow(cpu) == FALSE) {
    // +1 cycle if the branch succeeds
    nes_cpuCycled(nes);
    cpu->programCounter += data;
  }
}
// BVS - Branch if Overflow Set
// If the overflow flag is set then add the relative displacement to the program counter
to cause a branch to a new location.
static void cpu_BVS(NES nes, Address address) {
  assert(nes != NULL);
  CPU cpu = nes_getCPU(nes);
  assert(cpu != NULL);
  debug_printf("BVS\n");
  SignedByte data = nes_readCPUMemory(nes, address);
  if (cpu_getOverflow(cpu) == TRUE) {
    // +1 cycle if the branch succeeds
    nes_cpuCycled(nes);
    cpu->programCounter += data;
  }
}
void cpu_step(NES nes) {
  assert(nes != NULL);
  debug_printf("cpu_step\n");
  CPU cpu = nes_getCPU(nes);
  assert(cpu != NULL);
  Byte instruction = nes_readCPUMemory(nes, cpu->programCounter);
  cpu_increaseProgramCounter(cpu);
  debug_printf("Instruction: 0x%x\n", instruction);
  Address address = 0;
  switch(instruction) {
    case LDA_IMM:
    case LDX_IMM:
    case LDY_IMM:
    case AND_IMM:
    case ORA_IMM:
    case EOR_IMM:
case CMP_IMM:
case CPX_IMM:
case CPY_IMM:
case ADC_IMM:
case SBC_IMM:
  address = cpu->programCounter;
  cpu_increaseProgramCounter(cpu);
  break;
case LDA_ZPAGE:
case STA_ZPAGE:
case LDX_ZPAGE:
case STX_ZPAGE:
case LDY_ZPAGE:
case STY_ZPAGE:
case AND_ZPAGE:
case ORA_ZPAGE:
case EOR_ZPAGE:
case BIT_ZPAGE:
case CMP_ZPAGE:
case CPX_ZPAGE:
case CPY_ZPAGE:
case ADC_ZPAGE:
case SBC_ZPAGE:
case ASL_ZPAGE:
case LSR_ZPAGE:
case ROL_ZPAGE:
case ROR_ZPAGE:
case INC_ZPAGE:
case DEC_ZPAGE:
  address = nes_readCPUMemory(nes, cpu->programCounter);
  cpu_increaseProgramCounter(cpu);
  break;
case LDA_ZPAGEX:
case STA_ZPAGEX:
case LDY_ZPAGEX:
case STY_ZPAGEX:
case AND_ZPAGEX:
case ORA_ZPAGEX:
case EOR_ZPAGEX:
case CMP_ZPAGEX:
case ADC_ZPAGEX:
case SBC_ZPAGEX:
case ASL_ZPAGEX:
case LSR_ZPAGEX:
case ROL_ZPAGEX:
case ROR_ZPAGEX:
case INC_ZPAGEX:
case DEC_ZPAGEX:
  {
    Byte data = nes_readCPUMemory(nes, cpu->programCounter);
    cpu_increaseProgramCounter(cpu);
    data += cpu->indexX;
    address = data;
  }
  break;
case LDA_ABS:
case STA_ABS:
case LDX_ABS:
case STX_ABS:
case LDY_ABS:
case STY_ABS:
case AND_ABS:
case ORA_ABS:
case EOR_ABS:
case BIT_ABS:
case CMP_ABS:
case CPX_ABS:
case CPY_ABS:
case ADC_ABS:
case SBC_ABS:
case ASL_ABS:
case LSR_ABS:
case ROL_ABS:
case ROR_ABS:
case INC_ABS:
case DEC_ABS:
case JSR_ABS:
case JMP_ABS:
  address = nes_readCPUMemory(nes, cpu->programCounter);
  cpu_increaseProgramCounter(cpu);
  address += nes_readCPUMemory(nes, cpu->programCounter) << BITS_PER_BYTE;
  cpu_increaseProgramCounter(cpu);
  break;
case LDA_ABSX:
case STA_ABSX:
case LDY_ABSX:
case AND_ABSX:
case ORA_ABSX:
case EOR_ABSX:
case CMP_ABSX:
case ADC_ABSX:
case SBC_ABSX:
case ASL_ABSX:
case LSR_ABSX:
case ROL_ABSX:
case ROR_ABSX:
case INC_ABSX:
case DEC_ABSX:
  address = nes_readCPUMemory(nes, cpu->programCounter);
  cpu_increaseProgramCounter(cpu);
  address += nes_readCPUMemory(nes, cpu->programCounter) << BITS_PER_BYTE;
  cpu_increaseProgramCounter(cpu);
  address += cpu->indexX;
  break;
case LDA_ABSY:
case STA_ABSY:
case LDX_ABSY:
case AND_ABSY:
case ORA_ABSY:
case EOR_ABSY:
case CMP_ABSY:
case ADC_ABSY:
case SBC_ABSY:
  address = nes_readCPUMemory(nes, cpu->programCounter);
  cpu_increaseProgramCounter(cpu);
  address += nes_readCPUMemory(nes, cpu->programCounter) << BITS_PER_BYTE;
  cpu_increaseProgramCounter(cpu);
  address += cpu->indexY;
  break;
    case LDA_INDX:
    case STA_INDX:
    case AND_INDX:
    case ORA_INDX:
    case EOR_INDX:
    case CMP_INDX:
    case ADC_INDX:
    case SBC_INDX:
      {
        Byte data = nes_readCPUMemory(nes, cpu->programCounter);
        cpu_increaseProgramCounter(cpu);
        data += cpu->indexX;
        Byte lowAddress = data;
        Byte highAddress = data + 1;
        address = nes_readCPUMemory(nes, lowAddress);
        address += nes_readCPUMemory(nes, highAddress) << BITS_PER_BYTE;
      }
      break;
    case LDA_INDY:
    case STA_INDY:
    case AND_INDY:
    case ORA_INDY:
    case EOR_INDY:
    case CMP_INDY:
    case ADC_INDY:
    case SBC_INDY:
      {
        Byte data = nes_readCPUMemory(nes, cpu->programCounter);
        cpu_increaseProgramCounter(cpu);
        Byte lowAddress = data;
        Byte highAddress = data+1;
        address = nes_readCPUMemory(nes, lowAddress);
        address += nes_readCPUMemory(nes, highAddress) << BITS_PER_BYTE;    // is the +1
meant to wraparound to zero page as well?
        address += cpu->indexY;
      }
      break;
    case LDX_ZPAGEY:
    case STX_ZPAGEY:
      {
        Byte data = nes_readCPUMemory(nes, cpu->programCounter);
        cpu_increaseProgramCounter(cpu);
        data += cpu->indexY;
        address = data;
      }
      break;
    case ASL_ACCUM:
    case LSR_ACCUM:
    case ROL_ACCUM:
    case ROR_ACCUM:
      // these take 2 cycles. do a dummy read so that the ppu/apu get to advance
      nes_cpuCycled(nes);
   break;
case JMP_INDIRECT_CODE1:
case JMP_INDIRECT_CODE2:
  {
    Byte directAddressLow = nes_readCPUMemory(nes, cpu->programCounter);
    cpu_increaseProgramCounter(cpu);
    Byte directAddressHigh = nes_readCPUMemory(nes, cpu->programCounter);
    cpu_increaseProgramCounter(cpu);
    Address lowAddress = directAddressLow;
    lowAddress += directAddressHigh << BITS_PER_BYTE;
    directAddressLow++;
    Address highAddress = directAddressLow;
    highAddress += directAddressHigh << BITS_PER_BYTE;
    address = nes_readCPUMemory(nes, lowAddress);
    address += nes_readCPUMemory(nes, highAddress) << BITS_PER_BYTE;
  }
  break;
case BRK:
case RTS:
case RTI:
case PHP:
case PLP:
case PHA:
case PLA:
case INX:
case DEX:
case INY:
case DEY:
case TAX:
case TXA:
case TAY:
case TYA:
case TSX:
case TXS:
case SED:
case CLD:
case SEI:
case CLI:
case SEC:
case CLC:
case CLV:
case NOP:
   // these take 2+ cycles. do a dummy read so that the ppu/apu get to advance
   nes_cpuCycled(nes);
  break;
case BEQ:
case BNE:
case BMI:
case BPL:
case BCC:
case BCS:
case BVC:
case BVS:
  address = cpu->programCounter;
  cpu_increaseProgramCounter(cpu);
   break;
  default:
    printf("Instruction not implemented: 0x%x\n", instruction);
    //debug_printf("Instruction not implemented: 0x%x\n", instruction);
    assert(FALSE);
}
switch(instruction) {
  case LDA_INDY:
  case LDA_IMM:
  case LDA_ZPAGE:
  case LDA_ZPAGEX:
  case LDA_ABS:
  case LDA_ABSX:
  case LDA_ABSY:
  case LDA_INDX:
    cpu_LDA(nes, address);
    break;
  case STA_INDY:
  case STA_ZPAGE:
  case STA_ZPAGEX:
  case STA_ABS:
  case STA_ABSX:
  case STA_ABSY:
  case STA_INDX:
    cpu_STA(nes, address);
    break;
  case LDX_IMM:
  case LDX_ZPAGE:
  case LDX_ABS:
  case LDX_ABSY:
  case LDX_ZPAGEY:
    cpu_LDX(nes, address);
    break;
  case STX_ZPAGE:
  case STX_ABS:
  case STX_ZPAGEY:
    cpu_STX(nes, address);
    break;
  case LDY_IMM:
  case LDY_ZPAGE:
  case LDY_ZPAGEX:
  case LDY_ABS:
  case LDY_ABSX:
    cpu_LDY(nes, address);
    break;
  case STY_ZPAGE:
  case STY_ZPAGEX:
  case STY_ABS:
    cpu_STY(nes, address);
    break;
  case AND_INDY:
  case AND_IMM:
  case AND_ZPAGE:
  case AND_ZPAGEX:
  case AND_ABS:
  case AND_ABSX:
  case AND_ABSY:
  case AND_INDX:
  cpu_AND(nes, address);
  break;
case ORA_INDY:
case ORA_IMM:
case ORA_ZPAGE:
case ORA_ZPAGEX:
case ORA_ABS:
case ORA_ABSX:
case ORA_ABSY:
case ORA_INDX:
  cpu_ORA(nes, address);
  break;
case EOR_INDY:
case EOR_IMM:
case EOR_ZPAGE:
case EOR_ZPAGEX:
case EOR_ABS:
case EOR_ABSX:
case EOR_ABSY:
case EOR_INDX:
  cpu_EOR(nes, address);
  break;
case BIT_ZPAGE:
case BIT_ABS:
  cpu_BIT(nes, address);
  break;
case CMP_INDY:
case CMP_IMM:
case CMP_ZPAGE:
case CMP_ZPAGEX:
case CMP_ABS:
case CMP_ABSX:
case CMP_ABSY:
case CMP_INDX:
  cpu_CMP(nes, address);
  break;
case CPX_IMM:
case CPX_ZPAGE:
case CPX_ABS:
  cpu_CPX(nes, address);
  break;
case CPY_IMM:
case CPY_ZPAGE:
case CPY_ABS:
  cpu_CPY(nes, address);
  break;
case ADC_INDY:
case ADC_IMM:
case ADC_ZPAGE:
case ADC_ZPAGEX:
case ADC_ABS:
case ADC_ABSX:
case ADC_ABSY:
case ADC_INDX:
  cpu_ADC(nes, address);
  break;
case SBC_INDY:
case SBC_IMM:
case SBC_ZPAGE:
case SBC_ZPAGEX:
case SBC_ABS:
case SBC_ABSX:
case SBC_ABSY:
case SBC_INDX:
  cpu_SBC(nes, address);
  break;
case ASL_ZPAGE:
case ASL_ZPAGEX:
case ASL_ABS:
case ASL_ABSX:
  cpu_ASL_memory(nes, address);
  break;
case LSR_ZPAGE:
case LSR_ZPAGEX:
case LSR_ABS:
case LSR_ABSX:
  cpu_LSR_memory(nes, address);
  break;
case ROL_ZPAGE:
case ROL_ZPAGEX:
case ROL_ABS:
case ROL_ABSX:
  cpu_ROL_memory(nes, address);
  break;
case ROR_ZPAGE:
case ROR_ZPAGEX:
case ROR_ABS:
case ROR_ABSX:
  cpu_ROR_memory(nes, address);
  break;
case INC_ZPAGE:
case INC_ZPAGEX:
case INC_ABS:
case INC_ABSX:
  cpu_INC(nes, address);
  break;
case DEC_ZPAGE:
case DEC_ZPAGEX:
case DEC_ABS:
case DEC_ABSX:
  cpu_DEC(nes, address);
  break;
case JSR_ABS:
  cpu_JSR(nes, address);
  break;
case JMP_ABS:
  cpu_JMP(nes, address);
  break;
case ASL_ACCUM:
  cpu_ASL(cpu);
  break;
case LSR_ACCUM:
  cpu_LSR(cpu);
  break;
case ROL_ACCUM:
  cpu_ROL(cpu);
  break;
case ROR_ACCUM:
  cpu_ROR(cpu);
   break;
case JMP_INDIRECT_CODE1:
case JMP_INDIRECT_CODE2:
   cpu_JMP(nes, address);
   break;
 case BRK:
   cpu_BRK(nes);
   break;
 case RTS:
   cpu_RTS(nes);
   break;
 case RTI:
   cpu_RTI(nes);
   break;
 case PHP:
   cpu_PHP(nes);
   break;
 case PLP:
   cpu_PLP(nes);
   break;
 case PHA:
   cpu_PHA(nes);
   break;
 case PLA:
   cpu_PLA(nes);
   break;
 case INX:
   cpu_INX(cpu);
   break;
 case DEX:
   cpu_DEX(cpu);
   break;
 case INY:
   cpu_INY(cpu);
   break;
 case DEY:
   cpu_DEY(cpu);
   break;
 case TAX:
   cpu_TAX(cpu);
   break;
 case TXA:
   cpu_TXA(cpu);
   break;
 case TAY:
   cpu_TAY(cpu);
   break;
 case TYA:
   cpu_TYA(cpu);
   break;
 case TSX:
   cpu_TSX(cpu);
   break;
  case TXS:
    cpu_TXS(cpu);
    break;
  case SED:
    cpu_SED(cpu);
    break;
  case CLD:
    cpu_CLD(cpu);
    break;
  case SEI:
    cpu_SEI(cpu);
    break;
  case CLI:
    cpu_CLI(cpu);
    break;
  case SEC:
    cpu_SEC(cpu);
    break;
  case CLC:
    cpu_CLC(cpu);
    break;
  case CLV:
    cpu_CLV(cpu);
    break;
  case BEQ:
    cpu_BEQ(nes, address);
    break;
  case BNE:
    cpu_BNE(nes, address);
    break;
  case BMI:
    cpu_BMI(nes, address);
    break;
  case BPL:
    cpu_BPL(nes, address);
    break;
  case BCC:
    cpu_BCC(nes, address);
    break;
  case BCS:
    cpu_BCS(nes, address);
    break;
  case BVC:
    cpu_BVC(nes, address);
    break;
  case BVS:
    cpu_BVS(nes, address);
    break;
  case NOP:
    break;
  default:
    printf("Instruction not implemented: 0x%x\n", instruction);
    //debug_printf("Instruction not implemented: 0x%x\n", instruction);
    assert(FALSE);
}
  debug_printf("Address: 0x%0x\n", address);
}
void cpu_tests(void) {
  {
    CPU cpu = cpu_init();
    assert(cpu != NULL);
    cpu_updateZero(cpu, 0);
    assert(cpu_getZero(cpu) == TRUE);
    assert(cpu_getNegative(cpu) == FALSE);
    assert(cpu_getOverflow(cpu) == FALSE);
    assert(cpu_getCarry(cpu) == FALSE);
    cpu_updateZero(cpu, 1);
    assert(cpu_getZero(cpu) == FALSE);
    assert(cpu_getNegative(cpu) == FALSE);
    assert(cpu_getOverflow(cpu) == FALSE);
    assert(cpu_getCarry(cpu) == FALSE);
  }
  {
    CPU cpu = cpu_init();
    assert(cpu != NULL);
    cpu_updateNegative(cpu, 128);
    assert(cpu_getNegative(cpu) == TRUE);
    assert(cpu_getOverflow(cpu) == FALSE);
    assert(cpu_getCarry(cpu) == FALSE);
    cpu_updateNegative(cpu, 127);
    assert(cpu_getNegative(cpu) == FALSE);
    assert(cpu_getOverflow(cpu) == FALSE);
    assert(cpu_getCarry(cpu) == FALSE);
    cpu_updateNegative(cpu, 0);
    assert(cpu_getNegative(cpu) == FALSE);
    assert(cpu_getOverflow(cpu) == FALSE);
    assert(cpu_getCarry(cpu) == FALSE);
    int i;
    for (i=0; i < 128; i++) {
      cpu_updateNegative(cpu, i);
      assert(cpu_getNegative(cpu) == FALSE);
      assert(cpu_getOverflow(cpu) == FALSE);
      assert(cpu_getCarry(cpu) == FALSE);
    }
    for (i=128; i < 256; i++) {
      cpu_updateNegative(cpu, i);
      assert(cpu_getNegative(cpu) == TRUE);
      assert(cpu_getOverflow(cpu) == FALSE);
      assert(cpu_getCarry(cpu) == FALSE);
    }
  }
  {
    CPU cpu = cpu_init();
    assert(cpu != NULL);
    cpu_setCarry(cpu, TRUE);
    assert(cpu_getCarry(cpu) == TRUE);
    assert(cpu_getOverflow(cpu) == FALSE);
  cpu_setCarry(cpu, FALSE);
  assert(cpu_getCarry(cpu) == FALSE);
  assert(cpu_getOverflow(cpu) == FALSE);
  cpu_updateCarry_subtract(cpu, 0, 0);
  assert(cpu_getCarry(cpu) == TRUE);
  cpu_updateCarry_subtract(cpu, 1, 0);
  assert(cpu_getCarry(cpu) == TRUE);
  // 0 if a borrow is required
  cpu_updateCarry_subtract(cpu, -1, 0);
  assert(cpu_getCarry(cpu) == TRUE);
  cpu_updateCarry_subtract(cpu, 1, 2);
  assert(cpu_getCarry(cpu) == FALSE);
  cpu_updateCarry_subtract(cpu, -100, 50);
  assert(cpu_getCarry(cpu) == TRUE);
  cpu_updateCarry_subtract(cpu, 100, 0);
  assert(cpu_getCarry(cpu) == TRUE);
  cpu_updateCarry_subtract(cpu, 100, 99);
  assert(cpu_getCarry(cpu) == TRUE);
  cpu_updateCarry_subtract(cpu, -1, -2);
  assert(cpu_getCarry(cpu) == TRUE);
  cpu_updateCarry_subtract(cpu, -2, -1);
  assert(cpu_getCarry(cpu) == FALSE);
}
{
  CPU cpu = cpu_init();
  assert(cpu != NULL);
  cpu_setOverflow(cpu, TRUE);
  assert(cpu_getOverflow(cpu) == TRUE);
  assert(cpu_getCarry(cpu) == FALSE);
  cpu_setOverflow(cpu, FALSE);
  assert(cpu_getOverflow(cpu) == FALSE);
  assert(cpu_getCarry(cpu) == FALSE);
  cpu_updateOverflow(cpu, -1, -2, 10);
  assert(cpu_getOverflow(cpu) == TRUE);
  cpu_updateOverflow(cpu, -1, -2, -3);
  assert(cpu_getOverflow(cpu) == FALSE);
  cpu_updateOverflow(cpu, 1, 2, 3);
  assert(cpu_getOverflow(cpu) == FALSE);
  cpu_updateOverflow(cpu, 1, 2, -3);
  assert(cpu_getOverflow(cpu) == TRUE);
  cpu_updateOverflow(cpu, -1, 2, -3);
  assert(cpu_getOverflow(cpu) == FALSE);
  cpu_updateOverflow(cpu, 1, -2, -3);
  assert(cpu_getOverflow(cpu) == FALSE);
  cpu_updateOverflow(cpu, -1, 2, 3);
  assert(cpu_getOverflow(cpu) == FALSE);
  cpu_updateOverflow(cpu, 1, -2, 3);
  assert(cpu_getOverflow(cpu) == FALSE);
  cpu_updateOverflow(cpu, 1, 1, 2);
  assert(cpu_getOverflow(cpu) == FALSE);
  cpu_updateOverflow(cpu, 1, -1, 0);
  assert(cpu_getOverflow(cpu) == FALSE);
  cpu_updateOverflow(cpu, 127, 1, 128);
  assert(cpu_getOverflow(cpu) == TRUE);
  cpu_updateOverflow(cpu, -128, -1, 127);
  assert(cpu_getOverflow(cpu) == TRUE);
  cpu_updateOverflow(cpu, 0, -1, -1);
  assert(cpu_getOverflow(cpu) == FALSE);
  cpu_updateOverflow(cpu, 127, 1, 128);
  assert(cpu_getOverflow(cpu) == TRUE);
}
{
  CPU cpu = cpu_init();
  assert(cpu != NULL);
  cpu_setInterruptDisable(cpu, TRUE);
  assert(cpu_getInterruptDisable(cpu) == TRUE);
  assert(cpu_getOverflow(cpu) == FALSE);
  assert(cpu_getCarry(cpu) == FALSE);
  assert(cpu_getNegative(cpu) == FALSE);
  assert(cpu_getZero(cpu) == FALSE);
  cpu_setInterruptDisable(cpu, FALSE);
  assert(cpu_getInterruptDisable(cpu) == FALSE);
  assert(cpu_getOverflow(cpu) == FALSE);
  assert(cpu_getCarry(cpu) == FALSE);
  assert(cpu_getNegative(cpu) == FALSE);
  assert(cpu_getZero(cpu) == FALSE);
}
{
  CPU cpu = cpu_init();
  assert(cpu != NULL);
  cpu_setBreak(cpu, TRUE);
  assert(cpu_getBreak(cpu) == TRUE);
  assert(cpu_getInterruptDisable(cpu) == FALSE);
  assert(cpu_getOverflow(cpu) == FALSE);
  assert(cpu_getCarry(cpu) == FALSE);
  assert(cpu_getNegative(cpu) == FALSE);
  assert(cpu_getZero(cpu) == FALSE);
  assert(cpu_getDecimal(cpu) == FALSE);
  cpu_setBreak(cpu, FALSE);
  assert(cpu_getBreak(cpu) == FALSE);
  assert(cpu_getInterruptDisable(cpu) == FALSE);
  assert(cpu_getOverflow(cpu) == FALSE);
  assert(cpu_getCarry(cpu) == FALSE);
  assert(cpu_getNegative(cpu) == FALSE);
  assert(cpu_getZero(cpu) == FALSE);
  assert(cpu_getDecimal(cpu) == FALSE);
  cpu_setInterruptDisable(cpu, TRUE);
  assert(cpu_getBreak(cpu) == FALSE);
  assert(cpu_getInterruptDisable(cpu) == TRUE);
  assert(cpu_getOverflow(cpu) == FALSE);
  assert(cpu_getCarry(cpu) == FALSE);
  assert(cpu_getNegative(cpu) == FALSE);
assert(cpu_getZero(cpu) == FALSE);
assert(cpu_getDecimal(cpu) == FALSE);
cpu_setInterruptDisable(cpu, FALSE);
assert(cpu_getBreak(cpu) == FALSE);
assert(cpu_getInterruptDisable(cpu) == FALSE);
assert(cpu_getOverflow(cpu) == FALSE);
assert(cpu_getCarry(cpu) == FALSE);
assert(cpu_getNegative(cpu) == FALSE);
assert(cpu_getZero(cpu) == FALSE);
assert(cpu_getDecimal(cpu) == FALSE);
cpu_setOverflow(cpu, TRUE);
assert(cpu_getBreak(cpu) == FALSE);
assert(cpu_getInterruptDisable(cpu) == FALSE);
assert(cpu_getOverflow(cpu) == TRUE);
assert(cpu_getCarry(cpu) == FALSE);
assert(cpu_getNegative(cpu) == FALSE);
assert(cpu_getZero(cpu) == FALSE);
assert(cpu_getDecimal(cpu) == FALSE);
cpu_setOverflow(cpu, FALSE);
assert(cpu_getBreak(cpu) == FALSE);
assert(cpu_getInterruptDisable(cpu) == FALSE);
assert(cpu_getOverflow(cpu) == FALSE);
assert(cpu_getCarry(cpu) == FALSE);
assert(cpu_getNegative(cpu) == FALSE);
assert(cpu_getZero(cpu) == FALSE);
assert(cpu_getDecimal(cpu) == FALSE);
cpu_setCarry(cpu, TRUE);
assert(cpu_getBreak(cpu) == FALSE);
assert(cpu_getInterruptDisable(cpu) == FALSE);
assert(cpu_getOverflow(cpu) == FALSE);
assert(cpu_getCarry(cpu) == TRUE);
assert(cpu_getNegative(cpu) == FALSE);
assert(cpu_getZero(cpu) == FALSE);
assert(cpu_getDecimal(cpu) == FALSE);
cpu_setCarry(cpu, FALSE);
assert(cpu_getBreak(cpu) == FALSE);
assert(cpu_getInterruptDisable(cpu) == FALSE);
assert(cpu_getOverflow(cpu) == FALSE);
assert(cpu_getCarry(cpu) == FALSE);
assert(cpu_getNegative(cpu) == FALSE);
assert(cpu_getZero(cpu) == FALSE);
assert(cpu_getDecimal(cpu) == FALSE);
cpu_setNegative(cpu, TRUE);
assert(cpu_getBreak(cpu) == FALSE);
assert(cpu_getInterruptDisable(cpu) == FALSE);
assert(cpu_getOverflow(cpu) == FALSE);
assert(cpu_getCarry(cpu) == FALSE);
assert(cpu_getNegative(cpu) == TRUE);
assert(cpu_getZero(cpu) == FALSE);
assert(cpu_getDecimal(cpu) == FALSE);
cpu_setNegative(cpu, FALSE);
assert(cpu_getBreak(cpu) == FALSE);
assert(cpu_getInterruptDisable(cpu) == FALSE);
assert(cpu_getOverflow(cpu) == FALSE);
assert(cpu_getCarry(cpu) == FALSE);
assert(cpu_getNegative(cpu) == FALSE);
assert(cpu_getZero(cpu) == FALSE);
assert(cpu_getDecimal(cpu) == FALSE);
cpu_setZero(cpu, TRUE);
assert(cpu_getBreak(cpu) == FALSE);
assert(cpu_getInterruptDisable(cpu) == FALSE);
assert(cpu_getOverflow(cpu) == FALSE);
assert(cpu_getCarry(cpu) == FALSE);
assert(cpu_getNegative(cpu) == FALSE);
    assert(cpu_getZero(cpu) == TRUE);
    assert(cpu_getDecimal(cpu) == FALSE);
    cpu_setZero(cpu, FALSE);
    assert(cpu_getBreak(cpu) == FALSE);
    assert(cpu_getInterruptDisable(cpu) == FALSE);
    assert(cpu_getOverflow(cpu) == FALSE);
    assert(cpu_getCarry(cpu) == FALSE);
    assert(cpu_getNegative(cpu) == FALSE);
    assert(cpu_getZero(cpu) == FALSE);
    assert(cpu_getDecimal(cpu) == FALSE);
    cpu_setDecimal(cpu, TRUE);
    assert(cpu_getBreak(cpu) == FALSE);
    assert(cpu_getInterruptDisable(cpu) == FALSE);
    assert(cpu_getOverflow(cpu) == FALSE);
    assert(cpu_getCarry(cpu) == FALSE);
    assert(cpu_getNegative(cpu) == FALSE);
    assert(cpu_getZero(cpu) == FALSE);
    assert(cpu_getDecimal(cpu) == TRUE);
    cpu_setDecimal(cpu, FALSE);
    assert(cpu_getBreak(cpu) == FALSE);
    assert(cpu_getInterruptDisable(cpu) == FALSE);
    assert(cpu_getOverflow(cpu) == FALSE);
    assert(cpu_getCarry(cpu) == FALSE);
    assert(cpu_getNegative(cpu) == FALSE);
    assert(cpu_getZero(cpu) == FALSE);
    assert(cpu_getDecimal(cpu) == FALSE);
  }
}
