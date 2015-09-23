#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "globals.h"
//#include "cpu1.h"
#include "nes.h"
#include "instructions.h"
#include "cpuMemory.h"
#include "usart.h"
#include "cpu.h"
#include "gpio.h"
// many comments below are based on operational details fromhttp://www.obelisk.demon.co.uk/6502/
struct cpu {
	Address programCounter;
	Byte stackPointer;
	Byte status;
	Byte indexX;
	Byte indexY;
	Byte accumulator;
};
CPU cpu_init(void) {
	CPU cpu1 = (CPU) malloc(sizeof(struct cpu));
	//assert(cpu1 != NULL);
	cpu1->programCounter = 0;
	cpu1->status = CPU_STATUS_REGISTER_INITIAL_VALUE;
	cpu1->indexX = 0;
	cpu1->indexY = 0;
	cpu1->accumulator = 0;
	cpu1->stackPointer = CPU_STACK_POINTER_INITIAL_VALUE;
	return cpu1;
}
void cpu_destroy(CPU cpu1) {
	//assert(cpu1 != NULL);
	free(cpu1);
}
Address cpu_getProgramCounter(CPU cpu1) {
	//assert(cpu1 != NULL);
	return cpu1->programCounter;
}
Byte cpu_getStackPointer(CPU cpu1) {
	//assert(cpu1 != NULL);
	return cpu1->stackPointer;
}
Byte cpu_getStatus(CPU cpu1) {
	//assert(cpu1 != NULL);
	// 1 at all times
	cpu1->status |= MASK_BIT5;
	return cpu1->status;
}
Byte cpu_getIndexX(CPU cpu1) {
	//assert(cpu1 != NULL);
	return cpu1->indexX;
}
Byte cpu_getIndexY(CPU cpu1) {
	//assert(cpu1 != NULL);
	return cpu1->indexY;
}
Byte cpu_getAccumulator(CPU cpu1) {
	//assert(cpu1 != NULL);
	return cpu1->accumulator;
}
void cpu_setProgramCounter(CPU cpu1, Address programCounter) {
	//assert(cpu1 != NULL);
	cpu1->programCounter = programCounter;
}
void cpu_setStackPointer(CPU cpu1, Byte stackPointer) {
	//assert(cpu1 != NULL);
	cpu1->stackPointer = stackPointer;
}
void cpu_setStatus(CPU cpu1, Byte status) {
	//assert(cpu1 != NULL);
	// 1 at all times
	status |= MASK_BIT5;
	cpu1->status = status;
}
void cpu_setIndexX(CPU cpu1, Byte indexX) {
	//assert(cpu1 != NULL);
	cpu1->indexX = indexX;
}
void cpu_setIndexY(CPU cpu1, Byte indexY) {
	//assert(cpu1 != NULL);
	cpu1->indexY = indexY;
}
void inline cpu_setAccumulator(CPU cpu1, Byte accumulator) {
	//assert(cpu1 != NULL);
	cpu1->accumulator = accumulator;
}
static inline void cpu_increaseProgramCounter(CPU cpu1) {
	//assert(cpu1 != NULL);
	cpu1->programCounter++;
}
static inline void cpu_JMP(NES nes, Address address) {
	//assert(nes != NULL);
	CPU cpu1 = nes_getCPU(nes);
	//assert(cpu1 != NULL);
	//usart_write_line(&AVR32_USART0,"JMP\n");
	cpu1->programCounter = address;
}
static Bool cpu_getZero(CPU cpu1) {
	//assert(cpu1 != NULL);
	if ((cpu1->status & MASK_STATUS_ZERO_ON) == MASK_STATUS_ZERO_ON) {
		return TRUE;
		} else {
		return FALSE;
	}
}
static inline void cpu_setZero(CPU cpu1, Bool state) {
	//assert(cpu1 != NULL);
	if (state == TRUE) {
		cpu1->status |= MASK_STATUS_ZERO_ON;
		} else if (state == FALSE) {
		cpu1->status &= MASK_STATUS_ZERO_OFF;
	}
}
//Z     Zero Flag      Set if A = 0
static inline void cpu_updateZero(CPU cpu1, Byte data) {
	//assert(cpu1 != NULL);
	if (data == 0) {
		cpu_setZero(cpu1, TRUE);
		} else {
		cpu_setZero(cpu1, FALSE);
	}
}
static void cpu_setDecimal(CPU cpu1, Bool state) {
	//assert(cpu1 != NULL);
	if (state == TRUE) {
		cpu1->status |= MASK_STATUS_DECIMAL_ON;
		} else if (state == FALSE) {
		cpu1->status &= MASK_STATUS_DECIMAL_OFF;
	}
}
static Bool cpu_getDecimal(CPU cpu1) {
	//assert(cpu1 != NULL);
	if ((cpu1->status & MASK_STATUS_DECIMAL_ON) == MASK_STATUS_DECIMAL_ON) {
		return TRUE;
		} else {
		return FALSE;
	}
}
// SED - Set Decimal Flag
static void cpu_SED(CPU cpu1) {
	//assert(cpu1 != NULL);
	cpu_setDecimal(cpu1, TRUE);
}
// CLD - Clear Decimal Mode
static void cpu_CLD(CPU cpu1) {
	//assert(cpu1 != NULL);
	cpu_setDecimal(cpu1, FALSE);
}
static Bool cpu_getNegative(CPU cpu1) {
	//assert(cpu1 != NULL);
	if ((cpu1->status & MASK_STATUS_NEGATIVE_ON) == MASK_STATUS_NEGATIVE_ON) {
		return TRUE;
		} else {
		return FALSE;
	}
}
static void cpu_setNegative(CPU cpu1, Bool state) {
	//assert(cpu1 != NULL);
	if (state == TRUE) {
		cpu1->status |= MASK_STATUS_NEGATIVE_ON;
		} else if (state == FALSE) {
		cpu1->status &= MASK_STATUS_NEGATIVE_OFF;
	}
}
//N     Negative Flag Set if bit 7 of A is set
static void cpu_updateNegative(CPU cpu1, Byte data) {
	//assert(cpu1 != NULL);
	if ((data & MASK_BIT7) == MASK_BIT7) {
		cpu_setNegative(cpu1, TRUE);
		} else {
		cpu_setNegative(cpu1, FALSE);
	}
}
// LDA - Load Accumulator
// Loads a byte of memory into the accumulator setting the zero and negative flags as appropriate.
static void cpu_LDA(NES nes, Address address) {
	//assert(nes != NULL);
	CPU cpu1 = nes_getCPU(nes);
	//assert(cpu1 != NULL);
	//usart_write_line(&AVR32_USART0,"LDA\n");
	Byte data = nes_readCPUMemory(nes, address);
	cpu_updateZero(cpu1, data);
	cpu_updateNegative(cpu1, data);
	cpu_setAccumulator(cpu1, data);
}
// STA - Store Accumulator
// Stores the contents of the accumulator into memory.
static void cpu_STA(NES nes, Address address) {
	//assert(nes != NULL);
	CPU cpu1 = nes_getCPU(nes);
	//assert(cpu1 != NULL);
	//usart_write_line(&AVR32_USART0,"STA\n");
	nes_writeCPUMemory(nes, address, cpu1->accumulator);
}
// LDX - Load X Register
// Loads a byte of memory into the X register setting the zero and negative flags as appropriate.
static void cpu_LDX(NES nes, Address address) {
	//assert(nes != NULL);
	CPU cpu1 = nes_getCPU(nes);
	//assert(cpu1 != NULL);
	//usart_write_line(&AVR32_USART0,"LDX\n");
	Byte data = nes_readCPUMemory(nes, address);
	cpu_updateZero(cpu1, data);
	cpu_updateNegative(cpu1, data);
	cpu_setIndexX(cpu1, data);
}
// STX - Store X Register
// Stores the contents of the X register into memory.
static void cpu_STX(NES nes, Address address) {
	//assert(nes != NULL);
	CPU cpu1 = nes_getCPU(nes);
	//assert(cpu1 != NULL);
	//usart_write_line(&AVR32_USART0,"STX\n");
	nes_writeCPUMemory(nes, address, cpu1->indexX);
}
// LDY - Load Y Register
// Loads a byte of memory into the Y register setting the zero and negative flags as appropriate.
static void cpu_LDY(NES nes, Address address) {
	//assert(nes != NULL);
	CPU cpu1 = nes_getCPU(nes);
	//assert(cpu1 != NULL);
	//usart_write_line(&AVR32_USART0,"LDY\n");
	Byte data = nes_readCPUMemory(nes, address);
	cpu_updateZero(cpu1, data);
	cpu_updateNegative(cpu1, data);
	cpu_setIndexY(cpu1, data);
}
// STY - Store Y Register
// Stores the contents of the Y register into memory.
static void cpu_STY(NES nes, Address address) {
	//assert(nes != NULL);
	CPU cpu1 = nes_getCPU(nes);
	//assert(cpu1 != NULL);
	//usart_write_line(&AVR32_USART0,"STY\n");
	nes_writeCPUMemory(nes, address, cpu1->indexY);
}
// AND - Logical AND
// A logical AND is performed, bit by bit, on the accumulator contents using the contents of a byte of memory.
static void cpu_AND(NES nes, Address address) {
	//assert(nes != NULL);
	CPU cpu1 = nes_getCPU(nes);
	//assert(cpu1 != NULL);
	//usart_write_line(&AVR32_USART0,"AND\n");
	Byte data = nes_readCPUMemory(nes, address);
	cpu1->accumulator &= data;
	cpu_updateZero(cpu1, cpu1->accumulator);
	cpu_updateNegative(cpu1, cpu1->accumulator);
}
// ORA - Logical Inclusive OR
// An inclusive OR is performed, bit by bit, on the accumulator contents using the contents of a byte of memory.
static void cpu_ORA(NES nes, Address address) {
	//assert(nes != NULL);
	CPU cpu1 = nes_getCPU(nes);
	//assert(cpu1 != NULL);
	//usart_write_line(&AVR32_USART0,"ORA\n");
	Byte data = nes_readCPUMemory(nes, address);
	cpu1->accumulator |= data;
	cpu_updateZero(cpu1, cpu1->accumulator);
	cpu_updateNegative(cpu1, cpu1->accumulator);
}
// EOR - Exclusive OR
// An exclusive OR is performed, bit by bit, on the accumulator contents using the ontents of a byte of memory.
static void cpu_EOR(NES nes, Address address) {
	//assert(nes != NULL);
	CPU cpu1 = nes_getCPU(nes);
	//assert(cpu1 != NULL);
	//usart_write_line(&AVR32_USART0,"EOR\n");
	Byte data = nes_readCPUMemory(nes, address);
	cpu1->accumulator ^= data;
	cpu_updateZero(cpu1, cpu1->accumulator);
	cpu_updateNegative(cpu1, cpu1->accumulator);
}
static void cpu_setOverflow(CPU cpu1, Bool state) {
	//assert(cpu1 != NULL);
	if (state == TRUE) {
		cpu1->status |= MASK_STATUS_OVERFLOW_ON;
		} else if (state == FALSE) {
		cpu1->status &= MASK_STATUS_OVERFLOW_OFF;
	}
}
static Bool cpu_getOverflow(CPU cpu1) {
	//assert(cpu1 != NULL);
	if ((cpu1->status & MASK_STATUS_OVERFLOW_ON) == MASK_STATUS_OVERFLOW_ON) {
		return TRUE;
		} else {
		return FALSE;
	}
}
// BIT - Bit Test
// This instructions is used to test if one or more bits are set in a target memory location.
// The mask pattern in A is ANDed with the value in memory to set or clear the zero flag, but the result is not kept.
// Bits 7 and 6 of the value from memory are copied into the N and V flags.
static void cpu_BIT(NES nes, Address address) {
	//assert(nes != NULL);
	CPU cpu1 = nes_getCPU(nes);
	//assert(cpu1 != NULL);
	//usart_write_line(&AVR32_USART0,"BIT\n");
	Byte data = nes_readCPUMemory(nes, address);
	cpu_updateZero(cpu1, cpu1->accumulator & data);
	cpu_updateNegative(cpu1, data);
	if ((data & MASK_BIT6) == MASK_BIT6) {
		cpu_setOverflow(cpu1, TRUE);
		} else {
		cpu_setOverflow(cpu1, FALSE);
	}
}
static void cpu_setCarry(CPU cpu1, Bool state) {
	//assert(cpu1 != NULL);
	if (state == TRUE) {
		cpu1->status |= MASK_STATUS_CARRY_ON;
		} else if (state == FALSE) {
		cpu1->status &= MASK_STATUS_CARRY_OFF;
	}
}
static Bool cpu_getCarry(CPU cpu1) {
	//assert(cpu1 != NULL);
	if ((cpu1->status & MASK_STATUS_CARRY_ON) == MASK_STATUS_CARRY_ON) {
		return TRUE;
		} else {
		return FALSE;
	}
}
// set to 0 - if a borrow is required
// set to 1 - if no borrow is required.
static void cpu_updateCarry_subtract(CPU cpu1, Byte a, Byte b) {
	//assert(cpu1 != NULL);
	// add a 'borrow' bit to borrow from
	Word aa = a;
	aa += MASK_BIT8;
	Word bb = b;
	Word result = aa-bb;
	if ((result & MASK_BIT8) == MASK_BIT8) {
		// subtraction didn't need to borrow from the 8th bit
		cpu_setCarry(cpu1, TRUE);
		} else {
		// needed to borrow from the 8th bit
		cpu_setCarry(cpu1, FALSE);
	}
}
// CMP - Compare
// This instruction compares the contents of the accumulator with another memory held value and sets the zero and carry flags as appropriate.
static void cpu_CMP(NES nes, Address address) {
	//assert(nes != NULL);
	CPU cpu1 = nes_getCPU(nes);
	//assert(cpu1 != NULL);
	//usart_write_line(&AVR32_USART0,"CMP\n");
	Byte data = nes_readCPUMemory(nes, address);
	// C         Carry Flag     Set if A >= M
	cpu_updateCarry_subtract(cpu1, cpu1->accumulator, data);
	// Z Zero Flag       Set if A = M
	cpu_updateZero(cpu1, cpu1->accumulator - data);
	// N Negative Flag   Set if bit 7 of the result is set
	cpu_updateNegative(cpu1, cpu1->accumulator - data);
}
// CPX - Compare X Register
// This instruction compares the contents of the X register with another memory held value and sets the zero and carry flags as appropriate.
static void cpu_CPX(NES nes, Address address) {
	//assert(nes != NULL);
	CPU cpu1 = nes_getCPU(nes);
	//assert(cpu1 != NULL);
	//usart_write_line(&AVR32_USART0,"CPX\n");
	Byte data = nes_readCPUMemory(nes, address);
	// C         Carry Flag     Set if X >= M
	cpu_updateCarry_subtract(cpu1, cpu1->indexX, data);
	// Z Zero Flag       Set if X = M
	cpu_updateZero(cpu1, cpu1->indexX - data);
	// N Negative Flag   Set if bit 7 of the result is set
	cpu_updateNegative(cpu1, cpu1->indexX - data);
}
// CPY - Compare Y Register
// This instruction compares the contents of the Y register with another memory held value and sets the zero and carry flags as appropriate.
static void cpu_CPY(NES nes, Address address) {
	//assert(nes != NULL);
	CPU cpu1 = nes_getCPU(nes);
	//assert(cpu1 != NULL);
	//usart_write_line(&AVR32_USART0,"CPY\n");
	Byte data = nes_readCPUMemory(nes, address);
	// C         Carry Flag     Set if Y >= M
	cpu_updateCarry_subtract(cpu1, cpu1->indexY, data);
	// Z Zero Flag       Set if Y = M
	cpu_updateZero(cpu1, cpu1->indexY - data);
	// N Negative Flag   Set if bit 7 of the result is set
	cpu_updateNegative(cpu1, cpu1->indexY - data);
}
// this looks correct to me, but doesn't work on one of the test cases, so i use a different method now
static void cpu_updateOverflow(CPU cpu1, Byte a, Byte b, Byte c) {
	//assert(cpu1 != NULL);
	// positive                    // positive                     // negative
	if ( ((a & MASK_BIT7) == MASK_BIT7) && ((b & MASK_BIT7) == MASK_BIT7) && ((c &
	MASK_BIT7) == 0) ) {
		cpu_setOverflow(cpu1, TRUE);
		// negative                 // negative                 // positive
	} else if ( ((a & MASK_BIT7) == 0) && ((b & MASK_BIT7) == 0) && ((c & MASK_BIT7) ==
	MASK_BIT7) ) {
		cpu_setOverflow(cpu1, TRUE);
		} else {
		cpu_setOverflow(cpu1, FALSE);
	}
}
// ADC - Add with Carry
// This instruction adds the contents of a memory location to the accumulator together with the carry bit.
// If overflow occurs the carry bit is set, this enables multiple byte addition to be performed.
static void cpu_ADC(NES nes, Address address) {
	//assert(nes != NULL);
	CPU cpu1 = nes_getCPU(nes);
	//assert(cpu1 != NULL);
	//usart_write_line(&AVR32_USART0,"ADC\n");
	// A + M + C
	Byte data = nes_readCPUMemory(nes, address);
	Word result = cpu1->accumulator + data;
	if (cpu_getCarry(cpu1) == TRUE) {
		result++;
	}
	// C  Carry Flag     Set if overflow in bit 7
	if ((result & MASK_BIT8) == MASK_BIT8) {
		cpu_setCarry(cpu1, TRUE);
		} else {
		cpu_setCarry(cpu1, FALSE);
	}
	// V Overflow Flag   Set if sign bit is incorrect
	// based on http://nesdev.parodius.com/6502.txt
	if (((cpu1->accumulator ^ result) & 0x80) && !((cpu1->accumulator ^ data) & 0x80)) {
		cpu_setOverflow(cpu1, TRUE);
		} else {
		cpu_setOverflow(cpu1, FALSE);
	}
	cpu1->accumulator = result;
	// Z Zero Flag       Set if A = 0
	cpu_updateZero(cpu1, cpu1->accumulator);
	// N Negative Flag   Set if bit 7 set
	cpu_updateNegative(cpu1, cpu1->accumulator);
}
// SBC - Subtract with Carry
// This instruction subtracts the contents of a memory location to the accumulator together with the not of the carry bit.
// If overflow occurs the carry bit is clear, this enables multiple byte subtraction to be performed.
static void cpu_SBC(NES nes, Address address) {
	//assert(nes != NULL);
	CPU cpu1 = nes_getCPU(nes);
	//assert(cpu1 != NULL);
	//usart_write_line(&AVR32_USART0,"SBC\n");
	// A,Z,C,N = A-M-(1-C)
	Byte data = nes_readCPUMemory(nes, address);
	Word result = cpu1->accumulator - data;
	if (cpu_getCarry(cpu1) == FALSE) {
		result--;
	}
	// C         Carry Flag     Clear if overflow in bit 7
	if ((result & MASK_BIT8) == MASK_BIT8) {
		cpu_setCarry(cpu1, FALSE);
		} else {
		cpu_setCarry(cpu1, TRUE);
	}
	// V Overflow Flag   Set if sign bit is incorrect
	// based on http://nesdev.parodius.com/6502.txt
	if (((cpu1->accumulator ^ result) & 0x80) && ((cpu1->accumulator ^ data) & 0x80)) {
		cpu_setOverflow(cpu1, TRUE);
		} else {
		cpu_setOverflow(cpu1, FALSE);
	}
	cpu1->accumulator = result;
	// Z Zero Flag       Set if A = 0
	cpu_updateZero(cpu1, cpu1->accumulator);
	// N Negative Flag   Set if bit 7 set
	cpu_updateNegative(cpu1, cpu1->accumulator);
}
// ASL - Arithmetic Shift Left
// This operation shifts all the bits of the memory contents one bit left.
// Bit 0 is set to 0 and bit 7 is placed in the carry flag.
// The effect of this operation is to multiply the memory contents by 2 (ignoring 2's complement considerations),
// setting the carry if the result will not fit in 8 bits.
static void cpu_ASL_memory(NES nes, Address address) {
	//assert(nes != NULL);
	CPU cpu1 = nes_getCPU(nes);
	//assert(cpu1 != NULL);
	//usart_write_line(&AVR32_USART0,"ASL_memory\n");
	// A,Z,C,N = M*2 or M,Z,C,N = M*2
	Byte data = nes_readCPUMemory(nes, address);
	// dummy write, cycle wasted here
	nes_writeCPUMemory(nes, address, data);
	Word result = data << 1;
	// C Carry Flag      Set to contents of old bit 7
	if ( (data & MASK_BIT7) == MASK_BIT7) {
		cpu_setCarry(cpu1, TRUE);
		} else {
		cpu_setCarry(cpu1, FALSE);
	}
	// Z Zero Flag       Set if A = 0
	cpu_updateZero(cpu1, result);
	// N Negative Flag   Set if bit 7 of the result is set
	cpu_updateNegative(cpu1, result);
	nes_writeCPUMemory(nes, address, result);
}
// LSR - Logical Shift Right
// Each of the bits in A or M is shift one place to the right.
// The bit that was in bit 0 is shifted into the carry flag.
// Bit 7 is set to zero.
static void cpu_LSR_memory(NES nes, Address address) {
	//assert(nes != NULL);
	CPU cpu1 = nes_getCPU(nes);
	//assert(cpu1 != NULL);
	//usart_write_line(&AVR32_USART0,"LSR_memory\n");
	// A,C,Z,N = A/2 or M,C,Z,N = M/2
	Byte data = nes_readCPUMemory(nes, address);
	// dummy write, cycle wasted here
	nes_writeCPUMemory(nes, address, data);
	Word result = data >> 1;
	// C         Carry Flag     Set to contents of old bit 0
	if ( (data & MASK_BIT0) == MASK_BIT0) {
		cpu_setCarry(cpu1, TRUE);
		} else {
		cpu_setCarry(cpu1, FALSE);
	}
	// Z Zero Flag       Set if result = 0
	cpu_updateZero(cpu1, result);
	// N Negative Flag   Set if bit 7 of the result is set
	cpu_updateNegative(cpu1, result);
	nes_writeCPUMemory(nes, address, result);
}
// ROL - Rotate Left
// Move each of the bits in either A or M one place to the left.
// Bit 0 is filled with the current value of the carry flag whilst the old bit 7 becomes the new carry flag value.
static void cpu_ROL_memory(NES nes, Address address) {
	//assert(nes != NULL);
	CPU cpu1 = nes_getCPU(nes);
	//assert(cpu1 != NULL);
	//usart_write_line(&AVR32_USART0,"ROL_memory\n");
	Byte data = nes_readCPUMemory(nes, address);
	// dummy write, cycle wasted here
	nes_writeCPUMemory(nes, address, data);
	Word result  = data << 1;
	if (cpu_getCarry(cpu1) == TRUE) {
		result |= MASK_BIT0;
	}
	// C         Carry Flag     Set to contents of old bit 7
	if ((data & MASK_BIT7) == MASK_BIT7) {
		cpu_setCarry(cpu1, TRUE);
		} else {
		cpu_setCarry(cpu1, FALSE);
	}
	// Z Zero Flag       Set if A = 0
	cpu_updateZero(cpu1, result);
	// N Negative Flag   Set if bit 7 of the result is set
	cpu_updateNegative(cpu1, result);
	nes_writeCPUMemory(nes, address, result);
}
// ROR - Rotate Right
// Move each of the bits in either A or M one place to the right.
// Bit 7 is filled with the current value of the carry flag whilst the old bit 0 becomes the new carry flag value.
static void cpu_ROR_memory(NES nes, Address address) {
	//assert(nes != NULL);
	CPU cpu1 = nes_getCPU(nes);
	//assert(cpu1 != NULL);
	//usart_write_line(&AVR32_USART0,"ROR_memory\n");
	Byte data = nes_readCPUMemory(nes, address);
	// dummy write, cycle wasted here
	nes_writeCPUMemory(nes, address, data);
	Word result = data >> 1;
	if (cpu_getCarry(cpu1) == TRUE) {
		result |= MASK_BIT7;
	}
	// C         Carry Flag      Set to contents of old bit 0
	if ((data & MASK_BIT0) == MASK_BIT0) {
		cpu_setCarry(cpu1, TRUE);
		} else {
		cpu_setCarry(cpu1, FALSE);
	}
	// Z Zero Flag       Set if A = 0
	cpu_updateZero(cpu1, result);
	// N Negative Flag   Set if bit 7 of the result is set
	cpu_updateZero(cpu1, result);
	nes_writeCPUMemory(nes, address, result);
}
// INC - Increment Memory
// Adds one to the value held at a specified memory location setting the zero and negative flags as appropriate.
static void cpu_INC(NES nes, Address address) {
	//assert(nes != NULL);
	CPU cpu1 = nes_getCPU(nes);
	//assert(cpu1 != NULL);
	//usart_write_line(&AVR32_USART0,"INC\n");
	Byte data = nes_readCPUMemory(nes, address);
	// dummy write, cycle wasted here
	nes_writeCPUMemory(nes, address, data);
	data++;
	// Z Zero Flag       Set if result is zero
	cpu_updateZero(cpu1, data);
	// N Negative Flag   Set if bit 7 of the result is set
	cpu_updateNegative(cpu1, data);
	nes_writeCPUMemory(nes, address, data);
}
// DEC - Decrement Memory
// Subtracts one from the value held at a specified memory location setting the zero and negative flags as appropriate.
static void cpu_DEC(NES nes, Address address) {
	//assert(nes != NULL);
	CPU cpu1 = nes_getCPU(nes);
	//assert(cpu1 != NULL);
	//usart_write_line(&AVR32_USART0,"DEC\n");
	Byte data = nes_readCPUMemory(nes, address);
	// dummy write, cycle wasted here
	nes_writeCPUMemory(nes, address, data);
	data--;
	// Z Zero Flag       Set if result is zero
	cpu_updateZero(cpu1, data);
	// N Negative Flag   Set if bit 7 of the result is set
	cpu_updateNegative(cpu1, data);
	nes_writeCPUMemory(nes, address, data);
}
// The processor supports a 256 byte stack located between $0100 and $01FF.
// The stack is located at memory locations $0100-$01FF.
// The stack pointer is an 8-bit register which serves as an offset from $0100.
// The stack pointer is an 8 bit register and holds the low 8 bits of the next freelocation on the stack.
// Pushing bytes to the stack causes the stack pointer to be decremented. Conversely pulling bytes causes it to be incremented.
// There is no detection of stack overflow and the stack pointer will just wrap around from $00 to $FF.
static void cpu_pushStack(NES nes, Byte data) {
	//assert(nes != NULL);
	CPU cpu1 = nes_getCPU(nes);
	//assert(cpu1 != NULL);
	Address stackAddress = GET_STACK_ADDRESS(cpu1->stackPointer);
	//VALIDATE_STACK_ADDRESS(stackAddress);
	nes_writeCPUMemory(nes, stackAddress, data);
	cpu1->stackPointer--;
	//VALIDATE_STACK_POINTER(cpu1->stackPointer);
}
static Byte cpu_popStack(NES nes) {
	//assert(nes != NULL);
	CPU cpu1 = nes_getCPU(nes);
	//assert(cpu1 != NULL);
	//VALIDATE_STACK_POINTER(cpu1->stackPointer);
	cpu1->stackPointer++;
	//VALIDATE_STACK_POINTER(cpu1->stackPointer);
	Address stackAddress = GET_STACK_ADDRESS(cpu1->stackPointer);
	//VALIDATE_STACK_ADDRESS(stackAddress);
	Byte data = nes_readCPUMemory(nes, stackAddress);
	return data;
}
// JSR - Jump to Subroutine
// The JSR instruction pushes the address (minus one) of the return point on to the stack and then sets the program counter to the target memory address.
static void cpu_JSR(NES nes, Address address) {
	//assert(nes != NULL);
	CPU cpu1 = nes_getCPU(nes);
	//assert(cpu1 != NULL);
	//usart_write_line(&AVR32_USART0,"JSR\n");
	// During a JSR, the address pushed onto the stack is that of the 3rd byte of th	instruction - that is, 1 byte BEFORE the next instruction.
	// This is because it pushes the program counter onto the stack BEFORE it fetches the final byte of the opcode
	// (and, as such, before it can increment the PC past this point).
	// To compensate for this, the RTS opcode increments the program counter during its 6th instruction cycle.
	// JSR takes 6 cycles, waste a cycle here
	//nes_cpuCycled(nes);
	Address returnAddress = cpu1->programCounter - 1;
	cpu_pushStack(nes, GET_ADDRESS_HIGH_BYTE(returnAddress));
	cpu_pushStack(nes, GET_ADDRESS_LOW_BYTE(returnAddress));
	cpu1->programCounter = address;
}
// ASL - Arithmetic Shift Left
// This operation shifts all the bits of the accumulator one bit left.
// Bit 0 is set to 0 and bit 7 is placed in the carry flag.
// The effect of this operation is to multiply the memory contents by 2
// (ignoring 2's complement considerations), setting the carry if the result will not fit in 8 bits.
static void cpu_ASL(CPU cpu1) {
	//assert(cpu1 != NULL);
	//usart_write_line(&AVR32_USART0,"ASL\n");
	Word result = cpu1->accumulator << 1;
	// C         Carry Flag      Set to contents of old bit 7
	if ((cpu1->accumulator & MASK_BIT7) == MASK_BIT7) {
		cpu_setCarry(cpu1, TRUE);
		} else {
		cpu_setCarry(cpu1, FALSE);
	}
	cpu1->accumulator = result;
	// Z Zero Flag       Set if A = 0
	cpu_updateZero(cpu1, cpu1->accumulator);
	// N Negative Flag   Set if bit 7 of the result is set
	cpu_updateNegative(cpu1, cpu1->accumulator);
}
// LSR - Logical Shift Right
// Each of the bits in A is shift one place to the right.
// The bit that was in bit 0 is shifted into the carry flag. Bit 7 is set to zero.
static void cpu_LSR(CPU cpu1) {
	//assert(cpu1 != NULL);
	//usart_write_line(&AVR32_USART0,"LSR\n");
	Word result = cpu1->accumulator >> 1;
	// C         Carry Flag      Set to contents of old bit 0
	if ( (cpu1->accumulator & MASK_BIT0) == MASK_BIT0 ) {
		cpu_setCarry(cpu1, TRUE);
		} else {
		cpu_setCarry(cpu1, FALSE);
	}
	cpu1->accumulator = result;
	// Z Zero Flag       Set if result = 0
	cpu_updateZero(cpu1, cpu1->accumulator);
	// N Negative Flag   Set if bit 7 of the result is set
	cpu_updateNegative(cpu1, cpu1->accumulator);
}
// ROL - Rotate Left
// Move each of the bits in either A or M one place to the left.
// Bit 0 is filled with the current value of the carry flag whilst the old bit 7 becomes the new carry flag value.
static void cpu_ROL(CPU cpu1) {
	//assert(cpu1 != NULL);
	//usart_write_line(&AVR32_USART0,"ROL\n");
	Word result = cpu1->accumulator << 1;
	if (cpu_getCarry(cpu1) == TRUE) {
		result |= MASK_BIT0;
	}
	if ( (cpu1->accumulator & MASK_BIT7) == MASK_BIT7) {
		cpu_setCarry(cpu1, TRUE);
		} else {
		cpu_setCarry(cpu1, FALSE);
	}
	// Z Zero Flag       Set if A = 0
	cpu1->accumulator = result;
	cpu_updateZero(cpu1, cpu1->accumulator);
	// N Negative Flag   Set if bit 7 of the result is set
	cpu_updateNegative(cpu1, cpu1->accumulator);
}
// ROR - Rotate Right
// Move each of the bits in either A or M one place to the right.
// Bit 7 is filled with the current value of the carry flag whilst the old bit 0 becomes the new carry flag value.
static void cpu_ROR(CPU cpu1) {
	//assert(cpu1 != NULL);
	//usart_write_line(&AVR32_USART0,"ROR\n");
	Word result = cpu1->accumulator >> 1;
	if (cpu_getCarry(cpu1) == TRUE) {
		result |= MASK_BIT7;
	}
	// C         Carry Flag     Set to contents of old bit 0
	if ( (cpu1->accumulator & MASK_BIT0) == MASK_BIT0) {
		cpu_setCarry(cpu1, TRUE);
		} else {
		cpu_setCarry(cpu1, FALSE);
	}
	cpu1->accumulator = result;
	// Z Zero Flag       Set if A = 0
	cpu_updateZero(cpu1, cpu1->accumulator);
	// N Negative Flag   Set if bit 7 of the result is set
	cpu_updateNegative(cpu1, cpu1->accumulator);
}
static void cpu_setInterruptDisable(CPU cpu1, Bool state) {
	//assert(cpu1 != NULL);
	if (state == TRUE) {
		cpu1->status |= MASK_STATUS_INTERRUPT_ON;
		} else if (state == FALSE) {
		cpu1->status &= MASK_STATUS_INTERRUPT_OFF;
	}
}
Bool cpu_getInterruptDisable(CPU cpu1) {
	//assert(cpu1 != NULL);
	if ((cpu1->status & MASK_STATUS_INTERRUPT_ON) == MASK_STATUS_INTERRUPT_ON) {
		return TRUE;
		} else {
		return FALSE;
	}
}
static void cpu_setBreak(CPU cpu1, Bool state) {
	//assert(cpu1 != NULL);
	if (state == TRUE) {
		cpu1->status |= MASK_STATUS_BREAK_ON;
		} else if (state == FALSE) {
		cpu1->status &= MASK_STATUS_BREAK_OFF;
	}
}
static Bool cpu_getBreak(CPU cpu1) {
	//assert(cpu1 != NULL);
	if ( (cpu1->status & MASK_STATUS_BREAK_ON) == MASK_STATUS_BREAK_ON) {
		return TRUE;
		} else {
		return FALSE;
	}
}
void cpu_handleInterrupt(NES nes, Address handlerLowByte, Bool fromBRK) {
	//assert(nes != NULL);
	CPU cpu1 = nes_getCPU(nes);
	//assert(cpu1 != NULL);
	usart_write_line(&AVR32_USART0,"handleInterrupt\n");
	cpu_setInterruptDisable(cpu1, TRUE);
	if (fromBRK == TRUE) {
		cpu_setBreak(cpu1, TRUE);
		} else {
		cpu_setBreak(cpu1, FALSE);
	}
	cpu_pushStack(nes, GET_ADDRESS_HIGH_BYTE(cpu1->programCounter));
	cpu_pushStack(nes, GET_ADDRESS_LOW_BYTE(cpu1->programCounter));
	// 1 at all times
	cpu1->status |= MASK_BIT5;
	cpu_pushStack(nes, cpu1->status);
	//0xFFFC para el reset
	Address address = nes_readCPUMemory(nes, handlerLowByte);
	address += nes_readCPUMemory(nes, handlerLowByte+1) << BITS_PER_BYTE;
	
    //usart_write_line(&AVR32_USART0,address);
	cpu1->programCounter = address;
}
// BRK - Force Interrupt
// The BRK instruction forces the generation of an interrupt request.
// The program counter and processor status are pushed on the stack
// then the IRQ interrupt vector at $FFFE/F is loaded into the PC
// and the break flag in the status set to one.
static void cpu_BRK(NES nes) {
	//assert(nes != NULL);
	CPU cpu1 = nes_getCPU(nes);
	//assert(cpu1 != NULL);
	//usart_write_line(&AVR32_USART0,"BRK\n");
	// the instruction has a "useless" second byte we need to skip
	// (added for debugging purposes apparently)
	cpu1->programCounter++;
	// B         Break Command          Set to 1
	cpu_handleInterrupt(nes, CPU_IRQ_VECTOR_LOWER_ADDRESS, TRUE);
}
// RTS - Return from Subroutine
// The RTS instruction is used at the end of a subroutine to return to the calling routine.
// It pulls the program counter (minus one) from the stack.
static void cpu_RTS(NES nes) {
	//assert(nes != NULL);
	CPU cpu1 = nes_getCPU(nes);
	//assert(cpu1 != NULL);
	//usart_write_line(&AVR32_USART0,"RTS\n");
	// waste a cycle for the stack pointer increment that is about to happen in popStack
	//nes_cpuCycled(nes);
	Address address = cpu_popStack(nes);
	address += cpu_popStack(nes) << BITS_PER_BYTE;
	// waste a cycle to increment the program counter
	//nes_cpuCycled(nes);
	address++;
	cpu1->programCounter = address;
}
// RTI - Return from Interrupt
// The RTI instruction is used at the end of an interrupt processing routine.
// It pulls the processor flags from the stack followed by the program counter.
static void cpu_RTI(NES nes) {
	//assert(nes != NULL);
	CPU cpu1 = nes_getCPU(nes);
	//assert(cpu1 != NULL);
	//usart_write_line(&AVR32_USART0,"RTI\n");
	// waste a cycle for the initial stack pointer increment that is about to happen in popStack
	//nes_cpuCycled(nes);
	cpu1->status = cpu_popStack(nes);
	// 1 at all times
	cpu1->status |= MASK_BIT5;
	Address address = cpu_popStack(nes);
	address += cpu_popStack(nes) << BITS_PER_BYTE;
	cpu1->programCounter = address;
}
// PHP - Push Processor Status
// Pushes a copy of the status flags on to the stack.
static void cpu_PHP(NES nes) {
	//assert(nes != NULL);
	CPU cpu1 = nes_getCPU(nes);
	//assert(cpu1 != NULL);
	//usart_write_line(&AVR32_USART0,"PHP\n");
	// The status bits pushed on the stack by PHP have the breakpoint bit set.
	// 1 at all times
	cpu1->status |= MASK_BIT5;
	Byte data = cpu1->status;
	data |= MASK_STATUS_BREAK_ON;
	//printf("PHP 0x%x 0x%x\n", cpu1->status, data);
	cpu_pushStack(nes, data);
}
// PLP - Pull Processor Status
// Pulls an 8 bit value from the stack and into the processor flags.
// The flags will take on new states as determined by the value pulled.
static void cpu_PLP(NES nes) {
	//assert(nes != NULL);
	CPU cpu1 = nes_getCPU(nes);
	//assert(cpu1 != NULL);
	//usart_write_line(&AVR32_USART0,"PLP\n");
	// waste a cycle for the stack pointer increment that is about to happen in popStack
	//nes_cpuCycled(nes);
	Byte data = cpu_popStack(nes);
	//printf("PLP 0x%x 0x%x\n", cpu1->status, data);
	cpu1->status = data;
	// 1 at all times
	cpu1->status |= MASK_BIT5;
}
// PHA - Push Accumulator
// Pushes a copy of the accumulator on to the stack.
static void cpu_PHA(NES nes) {
	//assert(nes != NULL);
	CPU cpu1 = nes_getCPU(nes);
	//assert(cpu1 != NULL);
	//usart_write_line(&AVR32_USART0,"PHA\n");
	cpu_pushStack(nes, cpu1->accumulator);
}
// PLA - Pull Accumulator
// Pulls an 8 bit value from the stack and into the accumulator.
// The zero and negative flags are set as appropriate.
static void cpu_PLA(NES nes) {
	//assert(nes != NULL);
	CPU cpu1 = nes_getCPU(nes);
	//assert(cpu1 != NULL);
	//usart_write_line(&AVR32_USART0,"PLA\n");
	// waste a cycle for the stack pointer increment that is about to happen in popStack
	//nes_cpuCycled(nes);
	cpu1->accumulator = cpu_popStack(nes);
	// Z Zero Flag       Set if A = 0
	cpu_updateZero(cpu1, cpu1->accumulator);
	// N Negative Flag Set if bit 7 of A is set
	cpu_updateNegative(cpu1, cpu1->accumulator);
}
// INX - Increment X Register
// Adds one to the X register setting the zero and negative flags as appropriate.
static void cpu_INX(CPU cpu1) {
	//assert(cpu1 != NULL);
	//usart_write_line(&AVR32_USART0,"INX\n");
	cpu1->indexX++;
	// Z Zero Flag       Set if X is zero
	cpu_updateZero(cpu1, cpu1->indexX);
	// N Negative Flag   Set if bit 7 of X is set
	cpu_updateNegative(cpu1, cpu1->indexX);
}
// DEX - Decrement X Register
// Subtracts one from the X register setting the zero and negative flags as appropriate.
static void cpu_DEX(CPU cpu1) {
	//assert(cpu1 != NULL);
	//usart_write_line(&AVR32_USART0,"DEX\n");
	cpu1->indexX--;
	// Z Zero Flag       Set if X is zero
	cpu_updateZero(cpu1, cpu1->indexX);
	// N Negative Flag   Set if bit 7 of X is set
	cpu_updateNegative(cpu1, cpu1->indexX);
}
// INY - Increment Y Register
// Adds one to the Y register setting the zero and negative flags as appropriate.
static void cpu_INY(CPU cpu1) {
	//assert(cpu1 != NULL);
	//usart_write_line(&AVR32_USART0,"INY\n");
	cpu1->indexY++;
	// Z Zero Flag       Set if Y is zero
	cpu_updateZero(cpu1, cpu1->indexY);
	// N Negative Flag   Set if bit 7 of Y is set
	cpu_updateNegative(cpu1, cpu1->indexY);
}
// DEY - Decrement Y Register
// Subtracts one from the Y register setting the zero and negative flags as appropriate.
static void cpu_DEY(CPU cpu1) {
	//assert(cpu1 != NULL);
	//usart_write_line(&AVR32_USART0,"DEY\n");
	cpu1->indexY--;
	// Z Zero Flag       Set if Y is zero
	cpu_updateZero(cpu1, cpu1->indexY);
	// N Negative Flag Set if bit 7 of Y is set
	cpu_updateNegative(cpu1, cpu1->indexY);
}
// TAX - Transfer Accumulator to X
// Copies the current contents of the accumulator into the X register and sets the zero and negative flags as appropriate.
static void cpu_TAX(CPU cpu1) {
	//assert(cpu1 != NULL);
	//usart_write_line(&AVR32_USART0,"TAX\n");
	cpu1->indexX = cpu1->accumulator;
	// Z Zero Flag       Set if X = 0
	cpu_updateZero(cpu1, cpu1->indexX);
	// N Negative Flag   Set if bit 7 of X is set
	cpu_updateNegative(cpu1, cpu1->indexX);
}
// TXA - Transfer X to Accumulator
// Copies the current contents of the X register into the accumulator and sets the zero and negative flags as appropriate.
static void cpu_TXA(CPU cpu1) {
	//assert(cpu1 != NULL);
	//usart_write_line(&AVR32_USART0,"TXA\n");
	cpu1->accumulator = cpu1->indexX;
	// Z Zero Flag       Set if A = 0
	cpu_updateZero(cpu1, cpu1->accumulator);
	// N Negative Flag Set if bit 7 of A is set
	cpu_updateNegative(cpu1, cpu1->accumulator);
}
// TYA - Transfer Y to Accumulator
// Copies the current contents of the Y register into the accumulator and sets the zero and negative flags as appropriate.
static void cpu_TYA(CPU cpu1) {
	//assert(cpu1 != NULL);
	//usart_write_line(&AVR32_USART0,"TYA\n");
	cpu1->accumulator = cpu1->indexY;
	// Z Zero Flag       Set if A = 0
	cpu_updateZero(cpu1, cpu1->accumulator);
	// N Negative Flag Set if bit 7 of A is set
	cpu_updateNegative(cpu1, cpu1->accumulator);
}
// TSX - Transfer Stack Pointer to X
// Copies the current contents of the stack register into the X register and sets the zero and negative flags as appropriate.
static void cpu_TSX(CPU cpu1) {
	//assert(cpu1 != NULL);
	//usart_write_line(&AVR32_USART0,"TSX\n");
	cpu1->indexX = cpu1->stackPointer;
	// Z Zero Flag       Set if X = 0
	cpu_updateZero(cpu1, cpu1->indexX);
	// N Negative Flag   Set if bit 7 of X is set
	cpu_updateNegative(cpu1, cpu1->indexX);
}
// TAY - Transfer Accumulator to Y
// Copies the current contents of the accumulator into the Y register and sets the zero and negative flags as appropriate.
static void cpu_TAY(CPU cpu1) {
	//assert(cpu1 != NULL);
	//usart_write_line(&AVR32_USART0,"TAY\n");
	cpu1->indexY = cpu1->accumulator;
	// Z Zero Flag       Set if Y = 0
	cpu_updateZero(cpu1, cpu1->indexY);
	// N Negative Flag Set if bit 7 of Y is set
	cpu_updateNegative(cpu1, cpu1->indexY);
}
// TXS - Transfer X to Stack Pointer
// Copies the current contents of the X register into the stack register.
static void cpu_TXS(CPU cpu1) {
	//assert(cpu1 != NULL);
	//usart_write_line(&AVR32_USART0,"TXS\n");
	cpu1->stackPointer = cpu1->indexX;
}
// SEI - Set Interrupt Disable
static void cpu_SEI(CPU cpu1) {
	//assert(cpu1 != NULL);
	//usart_write_line(&AVR32_USART0,"SEI\n");
	// I         Interrupt Disable      Set to 1
	cpu_setInterruptDisable(cpu1, TRUE);
}
// CLI - Clear Interrupt Disable
static void cpu_CLI(CPU cpu1) {
	//assert(cpu1 != NULL);
	//usart_write_line(&AVR32_USART0,"CLI\n");
	// I         Interrupt Disable      Set to 0
	cpu_setInterruptDisable(cpu1, FALSE);
}
// SEC - Set Carry Flag
static void cpu_SEC(CPU cpu1) {
	//assert(cpu1 != NULL);
	//usart_write_line(&AVR32_USART0,"SEC\n");
	cpu_setCarry(cpu1, TRUE);
}
// CLC - Clear Carry Flag
static void cpu_CLC(CPU cpu1) {
	//assert(cpu1 != NULL);
	//usart_write_line(&AVR32_USART0,"CLC\n");
	// C         Carry Flag     Set to 0
	cpu_setCarry(cpu1, FALSE);
}
// CLV - Clear Overflow Flag
static void cpu_CLV(CPU cpu1) {
	//assert(cpu1 != NULL);
	//usart_write_line(&AVR32_USART0,"CLV\n");
	// V         Overflow Flag          Set to 0
	cpu_setOverflow(cpu1, FALSE);
}
// BEQ - Branch if Equal
// If the zero flag is set then add the relative displacement to the program counter to ause a branch to a new location.
static void cpu_BEQ(NES nes, Address address) {
	//assert(nes != NULL);
	CPU cpu1 = nes_getCPU(nes);
	//assert(cpu1 != NULL);
	//usart_write_line(&AVR32_USART0,"BEQ\n");
	SignedByte data = nes_readCPUMemory(nes, address);
	if (cpu_getZero(cpu1) == TRUE) {
		// +1 cycle if the branch succeeds
		//nes_cpuCycled(nes);
		cpu1->programCounter += data;
	}
}
// BNE - Branch if Not Equal
// If the zero flag is clear then add the relative displacement to the program counter o cause a branch to a new location.
static void cpu_BNE(NES nes, Address address) {
	//assert(nes != NULL);
	CPU cpu1 = nes_getCPU(nes);
	//assert(cpu1 != NULL);
	//usart_write_line(&AVR32_USART0,"BNE\n");
	SignedByte data = nes_readCPUMemory(nes, address);
	if (cpu_getZero(cpu1) == FALSE) {
		// +1 cycle if the branch succeeds
		//nes_cpuCycled(nes);
		cpu1->programCounter += data;
	}
}
// BMI - Branch if Minus
// If the negative flag is set then add the relative displacement to the program counter to cause a branch to a new location.
static void cpu_BMI(NES nes, Address address) {
	//assert(nes != NULL);
	CPU cpu1 = nes_getCPU(nes);
	//assert(cpu1 != NULL);
	//usart_write_line(&AVR32_USART0,"BMI\n");
	SignedByte data = nes_readCPUMemory(nes, address);
	if (cpu_getNegative(cpu1) == TRUE) {
		// +1 cycle if the branch succeeds
		//nes_cpuCycled(nes);
		cpu1->programCounter += data;
	}
}
// BPL - Branch if Positive
// If the negative flag is clear then add the relative displacement to the program counter to cause a branch to a new location.
static void cpu_BPL(NES nes, Address address) {
	//assert(nes != NULL);
	CPU cpu1 = nes_getCPU(nes);
	//assert(cpu1 != NULL);
	//usart_write_line(&AVR32_USART0,"BPL\n");
	SignedByte data = nes_readCPUMemory(nes, address);
	if (cpu_getNegative(cpu1) == FALSE) {
		// +1 cycle if the branch succeeds
		//nes_cpuCycled(nes);
		cpu1->programCounter += data;
	}
}
// BCC - Branch if Carry Clear
// If the carry flag is clear then add the relative displacement to the program counter to cause a branch to a new location.
static void cpu_BCC(NES nes, Address address) {
	//assert(nes != NULL);
	CPU cpu1 = nes_getCPU(nes);
	//assert(cpu1 != NULL);
	//usart_write_line(&AVR32_USART0,"BCC\n");
	SignedByte data = nes_readCPUMemory(nes, address);
	if (cpu_getCarry(cpu1) == FALSE) {
		// +1 cycle if the branch succeeds
		//nes_cpuCycled(nes);
		cpu1->programCounter += data;
	}
}
// BCS - Branch if Carry Set
// If the carry flag is set then add the relative displacement to the program counter to cause a branch to a new location.
static void cpu_BCS(NES nes, Address address) {
	//assert(nes != NULL);
	CPU cpu1 = nes_getCPU(nes);
	//assert(cpu1 != NULL);
	//usart_write_line(&AVR32_USART0,"BCS\n");
	SignedByte data = nes_readCPUMemory(nes, address);
	if (cpu_getCarry(cpu1) == TRUE) {
		// +1 cycle if the branch succeeds
		//nes_cpuCycled(nes);
		cpu1->programCounter += data;
	}
}
// BVC - Branch if Overflow Clear
// If the overflow flag is clear then add the relative displacement to the program counter to cause a branch to a new location.
static void cpu_BVC(NES nes, Address address) {
	//assert(nes != NULL);
	CPU cpu1 = nes_getCPU(nes);
	//assert(cpu1 != NULL);
	//usart_write_line(&AVR32_USART0,"BVC\n");
	SignedByte data = nes_readCPUMemory(nes, address);
	if (cpu_getOverflow(cpu1) == FALSE) {
		// +1 cycle if the branch succeeds
		//nes_cpuCycled(nes);
		cpu1->programCounter += data;
	}
}
// BVS - Branch if Overflow Set
// If the overflow flag is set then add the relative displacement to the program counter to cause a branch to a new location.
static void cpu_BVS(NES nes, Address address) {
	//assert(nes != NULL);
	CPU cpu1 = nes_getCPU(nes);
	//assert(cpu1 != NULL);
	//usart_write_line(&AVR32_USART0,"BVS\n");
	SignedByte data = nes_readCPUMemory(nes, address);
	if (cpu_getOverflow(cpu1) == TRUE) {
		// +1 cycle if the branch succeeds
		//nes_cpuCycled(nes);
		cpu1->programCounter += data;
	}
}
void cpu_step(NES nes) {
	//assert(nes != NULL);
	//usart_write_line(&AVR32_USART0,"cpu_step\n");
	//gpio_toggle_pin(60);
	CPU cpu1 = nes_getCPU(nes);
	//assert(cpu1 != NULL);
	Byte instruction = nes_readCPUMemory(nes, cpu1->programCounter);
	//usart_write_char(&AVR32_USART0,instruction);
	cpu_increaseProgramCounter(cpu1);
//	//usart_write_line(&AVR32_USART0,"Instruction: 0x%x\n", instruction);
	Address address = 0;
	
	if (instruction==0x4C)
	{
		//usart_write_char(&AVR32_USART0,instruction);
		uint16_t add1,add2;
		add1 = nes_readCPUMemory(nes, cpu1->programCounter++);
		//usart_write_char(&AVR32_USART0,add1);
		add2= nes_readCPUMemory(nes, cpu1->programCounter);
		//usart_write_char(&AVR32_USART0,add2);
		cpu1->programCounter = ((add2 <<8)+add1);
		return;
	}
	
	
	
	switch(instruction) {
		case LDA_IMM: goto Immediate;
		case LDX_IMM: goto Immediate;
		case LDY_IMM: goto Immediate;
		case AND_IMM: goto Immediate;
		case ORA_IMM: goto Immediate;
		case EOR_IMM: goto Immediate;
		case CMP_IMM: goto Immediate;
		case CPX_IMM: goto Immediate;
		case CPY_IMM: goto Immediate;
		case ADC_IMM: goto Immediate;
		case SBC_IMM: 
		Immediate:
		address = cpu1->programCounter;
		cpu_increaseProgramCounter(cpu1);
		break;
		case LDA_ZPAGE:goto ZPAGE;
		case STA_ZPAGE:goto ZPAGE;
		case LDX_ZPAGE:goto ZPAGE;
		case STX_ZPAGE:goto ZPAGE;
		case LDY_ZPAGE:goto ZPAGE;
		case STY_ZPAGE:goto ZPAGE;
		case AND_ZPAGE:goto ZPAGE;
		case ORA_ZPAGE:goto ZPAGE;
		case EOR_ZPAGE:goto ZPAGE;
		case BIT_ZPAGE:goto ZPAGE;
		case CMP_ZPAGE:goto ZPAGE;
		case CPX_ZPAGE:goto ZPAGE;
		case CPY_ZPAGE:goto ZPAGE;
		case ADC_ZPAGE:goto ZPAGE;
		case SBC_ZPAGE:goto ZPAGE;
		case ASL_ZPAGE:goto ZPAGE;
		case LSR_ZPAGE:goto ZPAGE;
		case ROL_ZPAGE:goto ZPAGE;
		case ROR_ZPAGE:goto ZPAGE;
		case INC_ZPAGE:goto ZPAGE;
		case DEC_ZPAGE:
		ZPAGE:
		address = nes_readCPUMemory(nes, cpu1->programCounter);
		cpu_increaseProgramCounter(cpu1);
		break;
		case LDA_ZPAGEX: goto ZPAGEX;
		case STA_ZPAGEX:goto ZPAGEX;
		case LDY_ZPAGEX:goto ZPAGEX;
		case STY_ZPAGEX:goto ZPAGEX;
		case AND_ZPAGEX:goto ZPAGEX;
		case ORA_ZPAGEX:goto ZPAGEX;
		case EOR_ZPAGEX:goto ZPAGEX;
		case CMP_ZPAGEX:goto ZPAGEX;
		case ADC_ZPAGEX:goto ZPAGEX;
		case SBC_ZPAGEX:goto ZPAGEX;
		case ASL_ZPAGEX:goto ZPAGEX;
		case LSR_ZPAGEX:goto ZPAGEX;
		case ROL_ZPAGEX:goto ZPAGEX;
		case ROR_ZPAGEX:goto ZPAGEX;
		case INC_ZPAGEX: ZPAGEX:
		case DEC_ZPAGEX:
		{
			Byte data = nes_readCPUMemory(nes, cpu1->programCounter);
			cpu_increaseProgramCounter(cpu1);
			data += cpu1->indexX;
			address = data;
		}
		break;
		case LDA_ABS: goto ABSOLUTE;
		case STA_ABS:goto ABSOLUTE;
		case LDX_ABS:goto ABSOLUTE;
		case STX_ABS:goto ABSOLUTE;
		case LDY_ABS:goto ABSOLUTE;
		case STY_ABS:goto ABSOLUTE;
		case AND_ABS:goto ABSOLUTE;
		case ORA_ABS:goto ABSOLUTE;
		case EOR_ABS:goto ABSOLUTE;
		case BIT_ABS:goto ABSOLUTE;
		case CMP_ABS:goto ABSOLUTE;
		case CPX_ABS:goto ABSOLUTE;
		case CPY_ABS:goto ABSOLUTE;
		case ADC_ABS:goto ABSOLUTE;
		case SBC_ABS:goto ABSOLUTE;
		case ASL_ABS:goto ABSOLUTE;
		case LSR_ABS:goto ABSOLUTE;
		case ROL_ABS:goto ABSOLUTE;
		case ROR_ABS:goto ABSOLUTE;
		case INC_ABS:goto ABSOLUTE;
		case DEC_ABS:goto ABSOLUTE;
		case JSR_ABS:goto ABSOLUTE;
		case JMP_ABS: ABSOLUTE:
		address = nes_readCPUMemory(nes, cpu1->programCounter);
		cpu_increaseProgramCounter(cpu1);
		address += nes_readCPUMemory(nes, cpu1->programCounter) << BITS_PER_BYTE;
		cpu_increaseProgramCounter(cpu1);
		break;
		case LDA_ABSX: goto ABSOLUTEX;
		case STA_ABSX:goto ABSOLUTEX;
		case LDY_ABSX:goto ABSOLUTEX;
		case AND_ABSX:goto ABSOLUTEX;
		case ORA_ABSX:goto ABSOLUTEX;
		case EOR_ABSX:goto ABSOLUTEX;
		case CMP_ABSX:goto ABSOLUTEX;
		case ADC_ABSX:goto ABSOLUTEX;
		case SBC_ABSX:goto ABSOLUTEX;
		case ASL_ABSX:goto ABSOLUTEX;
		case LSR_ABSX:goto ABSOLUTEX;
		case ROL_ABSX:goto ABSOLUTEX;
		case ROR_ABSX:goto ABSOLUTEX;
		case INC_ABSX:goto ABSOLUTEX;
		case DEC_ABSX: ABSOLUTEX:
		address = nes_readCPUMemory(nes, cpu1->programCounter);
		cpu_increaseProgramCounter(cpu1);
		address += nes_readCPUMemory(nes, cpu1->programCounter) << BITS_PER_BYTE;
		cpu_increaseProgramCounter(cpu1);
		address += cpu1->indexX;
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
		address = nes_readCPUMemory(nes, cpu1->programCounter);
		cpu_increaseProgramCounter(cpu1);
		address += nes_readCPUMemory(nes, cpu1->programCounter) << BITS_PER_BYTE;
		cpu_increaseProgramCounter(cpu1);
		address += cpu1->indexY;
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
			Byte data = nes_readCPUMemory(nes, cpu1->programCounter);
			cpu_increaseProgramCounter(cpu1);
			data += cpu1->indexX;
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
			Byte data = nes_readCPUMemory(nes, cpu1->programCounter);
			cpu_increaseProgramCounter(cpu1);
			Byte lowAddress = data;
			Byte highAddress = data+1;
			address = nes_readCPUMemory(nes, lowAddress);
			address += nes_readCPUMemory(nes, highAddress) << BITS_PER_BYTE;    // is the +1 meant to wraparound to zero page as well?
			address += cpu1->indexY;
		}
		break;
		case LDX_ZPAGEY:
		case STX_ZPAGEY:
		{
			Byte data = nes_readCPUMemory(nes, cpu1->programCounter);
			cpu_increaseProgramCounter(cpu1);
			data += cpu1->indexY;
			address = data;
		}
		break;
		case ASL_ACCUM:
		case LSR_ACCUM:
		case ROL_ACCUM:
		case ROR_ACCUM:
		// these take 2 cycles. do a dummy read so that the ppu/apu get to advance
		//nes_cpuCycled(nes);
		break;
		case JMP_INDIRECT_CODE1:
		case JMP_INDIRECT_CODE2:
		{
			Byte directAddressLow = nes_readCPUMemory(nes, cpu1->programCounter);
			cpu_increaseProgramCounter(cpu1);
			Byte directAddressHigh = nes_readCPUMemory(nes, cpu1->programCounter);
			cpu_increaseProgramCounter(cpu1);
			Address lowAddress = directAddressLow;
			lowAddress += directAddressHigh << BITS_PER_BYTE;
			directAddressLow++;
			Address highAddress = directAddressLow;
			highAddress += directAddressHigh << BITS_PER_BYTE;
			address = nes_readCPUMemory(nes, lowAddress);
			address += nes_readCPUMemory(nes, highAddress) << BITS_PER_BYTE;
		}
		break;
		case BRK: break;
		case RTS:break;
		case RTI:break;
		case PHP:break;
		case PLP:break;
		case PHA:break;
		case PLA:break;
		case INX:break;
		case DEX:break;
		case INY:break;
		case DEY:break;
		case TAX:break;
		case TXA:break;
		case TAY:break;
		case TYA:break;
		case TSX:break;
		case TXS:break;
		case SED:break;
		case CLD:break;
		case SEI:break;
		case CLI:break;
		case SEC:break;
		case CLC:break;
		case CLV:break;
		case NOP:
		// these take 2+ cycles. do a dummy read so that the ppu/apu get to advance
		//nes_cpuCycled(nes);
		break;
		case BEQ:
		case BNE:
		case BMI:
		case BPL:
		case BCC:
		case BCS:
		case BVC:
		case BVS:
		address = cpu1->programCounter;
		cpu_increaseProgramCounter(cpu1);
		break;
		default:
		break;
		//printf("Instruction not implemented: 0x%x\n", instruction);
		////usart_write_line(&AVR32_USART0,"Instruction not implemented: 0x%x\n", instruction);
		//assert(FALSE);
	}
	switch(instruction) {
		case LDA_INDY: cpu_LDA(nes, address); break;
		case LDA_IMM:cpu_LDA(nes, address); break;
		case LDA_ZPAGE:cpu_LDA(nes, address); break;
		case LDA_ZPAGEX:cpu_LDA(nes, address); break;
		case LDA_ABS:cpu_LDA(nes, address); break;
		case LDA_ABSX:cpu_LDA(nes, address); break;
		case LDA_ABSY:cpu_LDA(nes, address); break;
		case LDA_INDX:
		cpu_LDA(nes, address);
		break;
		case STA_INDY: cpu_STA(nes, address); break;
		case STA_ZPAGE:cpu_STA(nes, address); break;
		case STA_ZPAGEX:cpu_STA(nes, address); break;
		case STA_ABS:cpu_STA(nes, address); break;
		case STA_ABSX:cpu_STA(nes, address); break;
		case STA_ABSY:cpu_STA(nes, address); break;
		case STA_INDX:
		cpu_STA(nes, address);
		break;
		case LDX_IMM: cpu_LDX(nes, address); break;
		case LDX_ZPAGE:  cpu_LDX(nes, address); break; 
		case LDX_ABS: cpu_LDX(nes, address); break;
		case LDX_ABSY: cpu_LDX(nes, address); break;
		case LDX_ZPAGEY:
		cpu_LDX(nes, address);
		break;
		case STX_ZPAGE: cpu_STX(nes, address); break;
		case STX_ABS: cpu_STX(nes, address); break;
		case STX_ZPAGEY: cpu_STX(nes, address); break;
		cpu_STX(nes, address);
		break;
		case LDY_IMM: cpu_LDY(nes, address); break;
		case LDY_ZPAGE: cpu_LDY(nes, address); break;
		case LDY_ZPAGEX: cpu_LDY(nes, address); break;
		case LDY_ABS: cpu_LDY(nes, address); break;
		case LDY_ABSX:
		cpu_LDY(nes, address);
		break;
		case STY_ZPAGE:
		case STY_ZPAGEX:
		case STY_ABS:
		cpu_STY(nes, address);
		break;
		case AND_INDY: cpu_AND(nes, address); break;
		case AND_IMM: cpu_AND(nes, address); break;
		case AND_ZPAGE: cpu_AND(nes, address); break;
		case AND_ZPAGEX: cpu_AND(nes, address); break;
		case AND_ABS: cpu_AND(nes, address); break;
		case AND_ABSX: cpu_AND(nes, address); break;
		case AND_ABSY: cpu_AND(nes, address); break;
		case AND_INDX:
		cpu_AND(nes, address);
		break;
		case ORA_INDY: cpu_ORA(nes, address); break;
		case ORA_IMM: cpu_ORA(nes, address); break; 
		case ORA_ZPAGE: cpu_ORA(nes, address); break; 
		case ORA_ZPAGEX: cpu_ORA(nes, address); break;
		case ORA_ABS: cpu_ORA(nes, address); break;
		case ORA_ABSX: cpu_ORA(nes, address); break;
		case ORA_ABSY: cpu_ORA(nes, address); break;
		case ORA_INDX:
		cpu_ORA(nes, address);
		break;
		case EOR_INDY: cpu_EOR(nes, address); break;
		case EOR_IMM:  cpu_EOR(nes, address); break;
		case EOR_ZPAGE:  cpu_EOR(nes, address); break;
		case EOR_ZPAGEX:  cpu_EOR(nes, address); break;
		case EOR_ABS:  cpu_EOR(nes, address); break;
		case EOR_ABSX:  cpu_EOR(nes, address); break;
		case EOR_ABSY:  cpu_EOR(nes, address); break;
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
		cpu_ASL(cpu1);
		break;
		case LSR_ACCUM:
		cpu_LSR(cpu1);
		break;
		case ROL_ACCUM:
		cpu_ROL(cpu1);
		break;
		case ROR_ACCUM:
		cpu_ROR(cpu1);
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
		cpu_INX(cpu1);
		break;
		case DEX:
		cpu_DEX(cpu1);
		break;
		case INY:
		cpu_INY(cpu1);
		break;
		case DEY:
		cpu_DEY(cpu1);
		break;
		case TAX:
		cpu_TAX(cpu1);
		break;
		case TXA:
		cpu_TXA(cpu1);
		break;
		case TAY:
		cpu_TAY(cpu1);
		break;
		case TYA:
		cpu_TYA(cpu1);
		break;
		case TSX:
		cpu_TSX(cpu1);
		break;
		case TXS:
		cpu_TXS(cpu1);
		break;
		case SED:
		cpu_SED(cpu1);
		break;
		case CLD:
		cpu_CLD(cpu1);
		break;
		case SEI:
		cpu_SEI(cpu1);
		break;
		case CLI:
		cpu_CLI(cpu1);
		break;
		case SEC:
		cpu_SEC(cpu1);
		break;
		case CLC:
		cpu_CLC(cpu1);
		break;
		case CLV:
		cpu_CLV(cpu1);
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
		break;
		//printf("Instruction not implemented: 0x%x\n", instruction);
		////usart_write_line(&AVR32_USART0,"Instruction not implemented: 0x%x\n", instruction);
		//assert(FALSE);
	}
//	//usart_write_line(&AVR32_USART0,"Address: 0x%0x\n", address);
/*
usart_write_char(&AVR32_USART0,instruction);
usart_write_char(&AVR32_USART0,address);
usart_write_char(&AVR32_USART0,address>>8);*/
}
void cpu_tests(void) {
	{
		CPU cpu1 = cpu_init();
		//assert(cpu1 != NULL);
		cpu_updateZero(cpu1, 0);
		//assert(cpu_getZero(cpu1) == TRUE);
		//assert(cpu_getNegative(cpu1) == FALSE);
		//assert(cpu_getOverflow(cpu1) == FALSE);
		//assert(cpu_getCarry(cpu1) == FALSE);
		cpu_updateZero(cpu1, 1);
		//assert(cpu_getZero(cpu1) == FALSE);
		//assert(cpu_getNegative(cpu1) == FALSE);
		//assert(cpu_getOverflow(cpu1) == FALSE);
		//assert(cpu_getCarry(cpu1) == FALSE);
	}
	{
		CPU cpu1 = cpu_init();
		//assert(cpu1 != NULL);
		cpu_updateNegative(cpu1, 128);
		//assert(cpu_getNegative(cpu1) == TRUE);
		//assert(cpu_getOverflow(cpu1) == FALSE);
		//assert(cpu_getCarry(cpu1) == FALSE);
		cpu_updateNegative(cpu1, 127);
		//assert(cpu_getNegative(cpu1) == FALSE);
		//assert(cpu_getOverflow(cpu1) == FALSE);
		//assert(cpu_getCarry(cpu1) == FALSE);
		cpu_updateNegative(cpu1, 0);
		//assert(cpu_getNegative(cpu1) == FALSE);
		//assert(cpu_getOverflow(cpu1) == FALSE);
		//assert(cpu_getCarry(cpu1) == FALSE);
		int i;
		for (i=0; i < 128; i++) {
			cpu_updateNegative(cpu1, i);
			//assert(cpu_getNegative(cpu1) == FALSE);
			//assert(cpu_getOverflow(cpu1) == FALSE);
			//assert(cpu_getCarry(cpu1) == FALSE);
		}
		for (i=128; i < 256; i++) {
			cpu_updateNegative(cpu1, i);
			//assert(cpu_getNegative(cpu1) == TRUE);
			//assert(cpu_getOverflow(cpu1) == FALSE);
			//assert(cpu_getCarry(cpu1) == FALSE);
		}
	}
	{
		CPU cpu1 = cpu_init();
		//assert(cpu1 != NULL);
		cpu_setCarry(cpu1, TRUE);
		//assert(cpu_getCarry(cpu1) == TRUE);
		//assert(cpu_getOverflow(cpu1) == FALSE);
		cpu_setCarry(cpu1, FALSE);
		//assert(cpu_getCarry(cpu1) == FALSE);
		//assert(cpu_getOverflow(cpu1) == FALSE);
		cpu_updateCarry_subtract(cpu1, 0, 0);
		//assert(cpu_getCarry(cpu1) == TRUE);
		cpu_updateCarry_subtract(cpu1, 1, 0);
		//assert(cpu_getCarry(cpu1) == TRUE);
		// 0 if a borrow is required
		cpu_updateCarry_subtract(cpu1, -1, 0);
		//assert(cpu_getCarry(cpu1) == TRUE);
		cpu_updateCarry_subtract(cpu1, 1, 2);
		//assert(cpu_getCarry(cpu1) == FALSE);
		cpu_updateCarry_subtract(cpu1, -100, 50);
		//assert(cpu_getCarry(cpu1) == TRUE);
		cpu_updateCarry_subtract(cpu1, 100, 0);
		//assert(cpu_getCarry(cpu1) == TRUE);
		cpu_updateCarry_subtract(cpu1, 100, 99);
		//assert(cpu_getCarry(cpu1) == TRUE);
		cpu_updateCarry_subtract(cpu1, -1, -2);
		//assert(cpu_getCarry(cpu1) == TRUE);
		cpu_updateCarry_subtract(cpu1, -2, -1);
		//assert(cpu_getCarry(cpu1) == FALSE);
	}
	{
		CPU cpu1 = cpu_init();
		//assert(cpu1 != NULL);
		cpu_setOverflow(cpu1, TRUE);
		//assert(cpu_getOverflow(cpu1) == TRUE);
		//assert(cpu_getCarry(cpu1) == FALSE);
		cpu_setOverflow(cpu1, FALSE);
		//assert(cpu_getOverflow(cpu1) == FALSE);
		//assert(cpu_getCarry(cpu1) == FALSE);
		cpu_updateOverflow(cpu1, -1, -2, 10);
		//assert(cpu_getOverflow(cpu1) == TRUE);
		cpu_updateOverflow(cpu1, -1, -2, -3);
		//assert(cpu_getOverflow(cpu1) == FALSE);
		cpu_updateOverflow(cpu1, 1, 2, 3);
		//assert(cpu_getOverflow(cpu1) == FALSE);
		cpu_updateOverflow(cpu1, 1, 2, -3);
		//assert(cpu_getOverflow(cpu1) == TRUE);
		cpu_updateOverflow(cpu1, -1, 2, -3);
		//assert(cpu_getOverflow(cpu1) == FALSE);
		cpu_updateOverflow(cpu1, 1, -2, -3);
		//assert(cpu_getOverflow(cpu1) == FALSE);
		cpu_updateOverflow(cpu1, -1, 2, 3);
		//assert(cpu_getOverflow(cpu1) == FALSE);
		cpu_updateOverflow(cpu1, 1, -2, 3);
		//assert(cpu_getOverflow(cpu1) == FALSE);
		cpu_updateOverflow(cpu1, 1, 1, 2);
		//assert(cpu_getOverflow(cpu1) == FALSE);
		cpu_updateOverflow(cpu1, 1, -1, 0);
		//assert(cpu_getOverflow(cpu1) == FALSE);
		cpu_updateOverflow(cpu1, 127, 1, 128);
		//assert(cpu_getOverflow(cpu1) == TRUE);
		cpu_updateOverflow(cpu1, -128, -1, 127);
		//assert(cpu_getOverflow(cpu1) == TRUE);
		cpu_updateOverflow(cpu1, 0, -1, -1);
		//assert(cpu_getOverflow(cpu1) == FALSE);
		cpu_updateOverflow(cpu1, 127, 1, 128);
		//assert(cpu_getOverflow(cpu1) == TRUE);
	}
	{
		CPU cpu1 = cpu_init();
		//assert(cpu1 != NULL);
		cpu_setInterruptDisable(cpu1, TRUE);
		//assert(cpu_getInterruptDisable(cpu1) == TRUE);
		//assert(cpu_getOverflow(cpu1) == FALSE);
		//assert(cpu_getCarry(cpu1) == FALSE);
		//assert(cpu_getNegative(cpu1) == FALSE);
		//assert(cpu_getZero(cpu1) == FALSE);
		cpu_setInterruptDisable(cpu1, FALSE);
		//assert(cpu_getInterruptDisable(cpu1) == FALSE);
		//assert(cpu_getOverflow(cpu1) == FALSE);
		//assert(cpu_getCarry(cpu1) == FALSE);
		//assert(cpu_getNegative(cpu1) == FALSE);
		//assert(cpu_getZero(cpu1) == FALSE);
	}
	{
		CPU cpu1 = cpu_init();
		//assert(cpu1 != NULL);
		cpu_setBreak(cpu1, TRUE);
		//assert(cpu_getBreak(cpu1) == TRUE);
		//assert(cpu_getInterruptDisable(cpu1) == FALSE);
		//assert(cpu_getOverflow(cpu1) == FALSE);
		//assert(cpu_getCarry(cpu1) == FALSE);
		//assert(cpu_getNegative(cpu1) == FALSE);
		//assert(cpu_getZero(cpu1) == FALSE);
		//assert(cpu_getDecimal(cpu1) == FALSE);
		cpu_setBreak(cpu1, FALSE);
		//assert(cpu_getBreak(cpu1) == FALSE);
		//assert(cpu_getInterruptDisable(cpu1) == FALSE);
		//assert(cpu_getOverflow(cpu1) == FALSE);
		//assert(cpu_getCarry(cpu1) == FALSE);
		//assert(cpu_getNegative(cpu1) == FALSE);
		//assert(cpu_getZero(cpu1) == FALSE);
		//assert(cpu_getDecimal(cpu1) == FALSE);
		cpu_setInterruptDisable(cpu1, TRUE);
		//assert(cpu_getBreak(cpu1) == FALSE);
		//assert(cpu_getInterruptDisable(cpu1) == TRUE);
		//assert(cpu_getOverflow(cpu1) == FALSE);
		//assert(cpu_getCarry(cpu1) == FALSE);
		//assert(cpu_getNegative(cpu1) == FALSE);
		//assert(cpu_getZero(cpu1) == FALSE);
		//assert(cpu_getDecimal(cpu1) == FALSE);
		cpu_setInterruptDisable(cpu1, FALSE);
		//assert(cpu_getBreak(cpu1) == FALSE);
		//assert(cpu_getInterruptDisable(cpu1) == FALSE);
		//assert(cpu_getOverflow(cpu1) == FALSE);
		//assert(cpu_getCarry(cpu1) == FALSE);
		//assert(cpu_getNegative(cpu1) == FALSE);
		//assert(cpu_getZero(cpu1) == FALSE);
		//assert(cpu_getDecimal(cpu1) == FALSE);
		cpu_setOverflow(cpu1, TRUE);
		//assert(cpu_getBreak(cpu1) == FALSE);
		//assert(cpu_getInterruptDisable(cpu1) == FALSE);
		//assert(cpu_getOverflow(cpu1) == TRUE);
		//assert(cpu_getCarry(cpu1) == FALSE);
		//assert(cpu_getNegative(cpu1) == FALSE);
		//assert(cpu_getZero(cpu1) == FALSE);
		//assert(cpu_getDecimal(cpu1) == FALSE);
		cpu_setOverflow(cpu1, FALSE);
		//assert(cpu_getBreak(cpu1) == FALSE);
		//assert(cpu_getInterruptDisable(cpu1) == FALSE);
		//assert(cpu_getOverflow(cpu1) == FALSE);
		//assert(cpu_getCarry(cpu1) == FALSE);
		//assert(cpu_getNegative(cpu1) == FALSE);
		//assert(cpu_getZero(cpu1) == FALSE);
		//assert(cpu_getDecimal(cpu1) == FALSE);
		cpu_setCarry(cpu1, TRUE);
		//assert(cpu_getBreak(cpu1) == FALSE);
		//assert(cpu_getInterruptDisable(cpu1) == FALSE);
		//assert(cpu_getOverflow(cpu1) == FALSE);
		//assert(cpu_getCarry(cpu1) == TRUE);
		//assert(cpu_getNegative(cpu1) == FALSE);
		//assert(cpu_getZero(cpu1) == FALSE);
		//assert(cpu_getDecimal(cpu1) == FALSE);
		cpu_setCarry(cpu1, FALSE);
		//assert(cpu_getBreak(cpu1) == FALSE);
		//assert(cpu_getInterruptDisable(cpu1) == FALSE);
		//assert(cpu_getOverflow(cpu1) == FALSE);
		//assert(cpu_getCarry(cpu1) == FALSE);
		//assert(cpu_getNegative(cpu1) == FALSE);
		//assert(cpu_getZero(cpu1) == FALSE);
		//assert(cpu_getDecimal(cpu1) == FALSE);
		cpu_setNegative(cpu1, TRUE);
		//assert(cpu_getBreak(cpu1) == FALSE);
		//assert(cpu_getInterruptDisable(cpu1) == FALSE);
		//assert(cpu_getOverflow(cpu1) == FALSE);
		//assert(cpu_getCarry(cpu1) == FALSE);
		//assert(cpu_getNegative(cpu1) == TRUE);
		//assert(cpu_getZero(cpu1) == FALSE);
		//assert(cpu_getDecimal(cpu1) == FALSE);
		cpu_setNegative(cpu1, FALSE);
		//assert(cpu_getBreak(cpu1) == FALSE);
		//assert(cpu_getInterruptDisable(cpu1) == FALSE);
		//assert(cpu_getOverflow(cpu1) == FALSE);
		//assert(cpu_getCarry(cpu1) == FALSE);
		//assert(cpu_getNegative(cpu1) == FALSE);
		//assert(cpu_getZero(cpu1) == FALSE);
		//assert(cpu_getDecimal(cpu1) == FALSE);
		cpu_setZero(cpu1, TRUE);
		//assert(cpu_getBreak(cpu1) == FALSE);
		//assert(cpu_getInterruptDisable(cpu1) == FALSE);
		//assert(cpu_getOverflow(cpu1) == FALSE);
		//assert(cpu_getCarry(cpu1) == FALSE);
		//assert(cpu_getNegative(cpu1) == FALSE);
		//assert(cpu_getZero(cpu1) == TRUE);
		//assert(cpu_getDecimal(cpu1) == FALSE);
		cpu_setZero(cpu1, FALSE);
		//assert(cpu_getBreak(cpu1) == FALSE);
		//assert(cpu_getInterruptDisable(cpu1) == FALSE);
		//assert(cpu_getOverflow(cpu1) == FALSE);
		//assert(cpu_getCarry(cpu1) == FALSE);
		//assert(cpu_getNegative(cpu1) == FALSE);
		//assert(cpu_getZero(cpu1) == FALSE);
		//assert(cpu_getDecimal(cpu1) == FALSE);
		cpu_setDecimal(cpu1, TRUE);
		//assert(cpu_getBreak(cpu1) == FALSE);
		//assert(cpu_getInterruptDisable(cpu1) == FALSE);
		//assert(cpu_getOverflow(cpu1) == FALSE);
		//assert(cpu_getCarry(cpu1) == FALSE);
		//assert(cpu_getNegative(cpu1) == FALSE);
		//assert(cpu_getZero(cpu1) == FALSE);
		//assert(cpu_getDecimal(cpu1) == TRUE);
		cpu_setDecimal(cpu1, FALSE);
		//assert(cpu_getBreak(cpu1) == FALSE);
		//assert(cpu_getInterruptDisable(cpu1) == FALSE);
		//assert(cpu_getOverflow(cpu1) == FALSE);
		//assert(cpu_getCarry(cpu1) == FALSE);
		//assert(cpu_getNegative(cpu1) == FALSE);
		//assert(cpu_getZero(cpu1) == FALSE);
		//assert(cpu_getDecimal(cpu1) == FALSE);
	}
}
