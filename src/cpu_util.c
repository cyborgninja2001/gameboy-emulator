#include "cpu.h"

extern cpu_context ctx;

// reverse high byte & low byte
u16 reverse(u16 n) {
    return ((n & 0xFF00) >> 8) | ((n & 0x00FF) << 8);
}

u16 cpu_read_reg(reg_type rt) {
    switch(rt) {
        case RT_A: return ctx.regs.a;
        case RT_F: return ctx.regs.f;
        case RT_B: return ctx.regs.b;
        case RT_C: return ctx.regs.c;
        case RT_D: return ctx.regs.d;
        case RT_E: return ctx.regs.e;
        case RT_H: return ctx.regs.h;
        case RT_L: return ctx.regs.l;

        // EXPLICACION:
        // agarra el registro, devuelve su addr de memoria y
        // lo castea a un puntero de u16.
        // Luego por la forma en que esta implementado
        // regs (a y luego f) entonces estan pegados en memoria
        // (uno al lado del otro), entonces
        // luego vuelvo a usar el operador de
        // desrreferenciacion * y entonces devuelvo
        // justamente af

        // lo mismo con bc, de y hl

        // lo castea a u16 pq cada registro ocupa u8
        case RT_AF: return reverse(*((u16 *)&ctx.regs.a));
        case RT_BC: return reverse(*((u16 *)&ctx.regs.b));
        case RT_DE: return reverse(*((u16 *)&ctx.regs.d));
        case RT_HL: return reverse(*((u16 *)&ctx.regs.h));

        case RT_PC: return ctx.regs.pc;
        case RT_SP: return ctx.regs.sp;
        default: return 0;
    }
}
