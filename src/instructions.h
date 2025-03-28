#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include "common.h"


// R: register
// MR: Memory location of the register (that hold the register)
// D8: 8 bits inmediatos?
typedef enum { // AM: Adressing Mode
    AM_IMP, // implicito (no requieren operandos)
    AM_R_D16,
    AM_R_R,
    AM_MR_R,
    AM_R,
    AM_R_D8,
    AM_R_MR,
    AM_R_HLI,
    AM_R_HLD,
    AM_HLI_R,
    AM_HLD_R,
    AM_R_A8,
    AM_A8_R,
    AM_HL_SPR,
    AM_D16,
    AM_D8,
    AM_D16_R,
    AM_MR_D8,
    AM_MR,
    AM_A16_R,
    AM_R_A16
} addr_mode;

typedef enum { // RT: Register Type
    RT_NONE, // none register nedded
    RT_A,
    RT_F,
    RT_B,
    RT_C,
    RT_D,
    RT_E,
    RT_H,
    RT_L,
    RT_AF,
    RT_BC,
    RT_DE,
    RT_HL,
    RT_SP,
    RT_PC
} reg_type;

typedef enum { // IN: instruction _ abreviacion inst
    IN_NONE, // to avoid errors (se incializa automaticamente cone sta inst)
    IN_NOP,
    IN_LD,
    IN_INC,
    IN_DEC,
    IN_RLCA,
    IN_ADD,
    IN_RRCA,
    IN_STOP,
    IN_RLA,
    IN_JR,
    IN_RRA,
    IN_DAA,
    IN_CPL,
    IN_SCF,
    IN_CCF,
    IN_HALT,
    IN_ADC,
    IN_SUB,
    IN_SBC,
    IN_AND,
    IN_XOR,
    IN_OR,
    IN_CP,
    IN_POP,
    IN_JP,
    IN_PUSH,
    IN_RET,
    IN_CB,
    IN_CALL,
    IN_RETI,
    IN_LDH,
    IN_JPHL,
    IN_DI,
    IN_EI,
    IN_RST,
    IN_ERR,
    //CB instructions...
    IN_RLC,
    IN_RRC,
    IN_RL,
    IN_RR,
    IN_SLA,
    IN_SRA,
    IN_SWAP,
    IN_SRL,
    IN_BIT,
    IN_RES,
    IN_SET
} in_type;


typedef enum {
    CT_NONE, CT_NZ, CT_Z, CT_NC, CT_C
} cond_type;

typedef struct {
    in_type type;
    addr_mode mode; // mode you fetch data from that instruction
    reg_type reg1;
    reg_type reg2;
    cond_type cond; // most of the cases there's not gonna be a condition (ej: jump if zero flag isn't set)
    u8 param; // for special cb isntructions
} instruction;

instruction *instruction_by_opcode(u8 opcode);

#endif