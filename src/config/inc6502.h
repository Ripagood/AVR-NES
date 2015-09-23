/*
 * IncFile1.h
 *
 * Created: 11/5/2014 11:56:40 AM
 *  Author: Admin
 */ 



#ifndef INC6502_H
#define INC6502_H


//6502 CPU registers
// uint16_t pc;
// uint8_t sp, a, x, y, status;

//externally supplied functions
uint8_t read6502(uint16_t address);
void write6502(uint16_t address, uint8_t value);

unsigned char*	MEMORY;
unsigned char	PPU_register[8];
unsigned char	IO_register[32];
uint8_t readROM(uint16_t address);
void push16(uint16_t pushval);
uint16_t pull16(void);
uint8_t pull8(void);
void reset6502(void);
/*static void imp(void);
static void acc(void);
static void imm(void);
static void zp(void);
static void zpx(void);
static void zpy(void);
static void rel(void);
static void abso(void);
static void absx(void);
static void absy(void);
static void ind(void);
static void indx(void);
static void indy(void);
static uint16_t getvalue(void);
static uint16_t getvalue16(void);
static void putvalue(uint16_t saveval);
static void adc(void);
static void and(void);
static void asl(void);
static void bcc(void);
static void bcs(void);
static void beq(void);
static void bit(void);
static void bmi(void);
static void bne(void);
static void bpl(void);
static void brk(void);
static void bvc(void);
static void bvs(void);
static void clc(void);
static void cld(void);
static void cli(void);
static void clv(void);
static void cmp(void);
static void cpx(void);
static void cpy(void);
static void dec(void);
static void dex(void);
static void dey(void);
static void eor(void);
static void inc(void);
static void inx(void);
static void iny(void);
static void jmp(void);
static void jsr(void);
static void lda(void);
static void ldx(void);
static void ldy(void);
static void lsr(void);
static void ora(void);
static void pha(void);
static void php(void);
static void pla(void);
static void plp(void);
static void rol(void);
static void ror(void);
static void rti(void);
static void rts(void);
static void sbc(void);
static void sec(void);
static void sed(void);
static void sei(void);
static void sta(void);
static void stx(void);
static void sty(void);
static void tax(void);
static void tay(void);
static void tsx(void);
static void txa(void);
static void txs(void);
static void tya(void);
*/
void nmi6502(void);
void irq6502(void);
void exec6502(uint32_t tickcount);
void step6502(void);
void hookexternal(void *funcptr);
//static void nop1(void);
#endif 