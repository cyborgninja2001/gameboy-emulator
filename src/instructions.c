#include "instructions.h"
#include "cpu.h"

// array of all diferent instructions types
instruction instructions[0x100] = {
    [0x00] = {IN_NOP, AM_IMP},

    [0x05] = {IN_DEC, AM_R, RT_B},

    [0x0E] = {IN_LD, AM_R_D8, RT_C},

    [0xAF] = {IN_XOR, AM_R, RT_A},

    [0xC3] = {IN_JP, AM_D16},
};

// returns the address of the instruction opcode
instruction *instruction_by_opcode(u8 opcode) {
    // las instrucciones no creadas se inicializan auto con el campo IN_NONE
    if (instructions[opcode].type == IN_NONE) {
        return NULL; // an error (pq todavia no esta implementada creo)
    }

    return &instructions[opcode];
}