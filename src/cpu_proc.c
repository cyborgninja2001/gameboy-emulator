// this file is for processing the cpu instructions

#include "cpu.h"
#include "emu.h"
#include "bus.h"
#include "stack.h"

void cpu_set_flags(cpu_context *ctx, char z, char n, char h, char c); // para que no tire error

// process CPU instructions ...

// in case we hit something we haven't implemented yet
static void proc_none(cpu_context *ctx) {
    printf("INVALID INSTRUCTION\n");
    exit(-7);
}

static void proc_nop(cpu_context *ctx) {
    // it doesn't do anything
}

static void proc_ld(cpu_context *ctx) {
    // special case:
    if (ctx->dest_is_mem) {
        // LD (BC), A for instance ...
        if (ctx->cur_inst->reg2 >= RT_AF) {
            // if 16bit register
            emu_cycles(1);
            bus_write16(ctx->mem_dest, ctx->fetched_data);
        } else {
            // regular bus write 8bit
            bus_write(ctx->mem_dest, ctx->fetched_data);
        }
        return;
    }

    // special case:
    if (ctx->cur_inst->mode == AM_HL_SPR) {
        u8 hflag = (cpu_read_reg(ctx->cur_inst->reg2) & 0xF) + (ctx->fetched_data & 0xF) >= 0x10;
        u8 cflag = (cpu_read_reg(ctx->cur_inst->reg2) & 0xFF) + (ctx->fetched_data & 0xFF) >= 0x100;

        cpu_set_flags(ctx, 0, 0, hflag, cflag);
        // casting to char because fetched data is unsigned but there could be a negative number there
        cpu_set_reg(ctx->cur_inst->reg1, ctx->cur_inst->reg2 + (char)ctx->fetched_data);
    }

    // main case:
    cpu_set_reg(ctx->cur_inst->reg1, ctx->fetched_data);
}

static bool check_cond(cpu_context *ctx) {
    bool z = CPU_FLAG_Z;
    bool c = CPU_FLAG_C;

    switch(ctx->cur_inst->cond) {
        case CT_NONE: return true;
        case CT_C: return c;
        case CT_NC: return !c;
        case CT_Z: return z;
        case CT_NZ: return !z;
    }
}

// generic jump?
static void goto_addr(cpu_context* ctx, u16 addr, bool pushpc) {
    if (check_cond(ctx)) { // si se cumple la cond de salto, muevo el pc
        if (pushpc) {
            emu_cycles(2);
            stack_push16(ctx->regs.pc);
        }
        ctx->regs.pc = addr;
        emu_cycles(1); // to sync ppu & timer
    }
}

static void proc_jp(cpu_context *ctx) {
    goto_addr(ctx, ctx->fetched_data, false);
}

// jump relative
static void proc_jr(cpu_context *ctx) {
    char rel = (char)(ctx->fetched_data & 0xFF); // cast becasuse it might be negative
    u16 addr = ctx->regs.pc + rel;
    goto_addr(ctx, addr, false);
}


static void proc_call(cpu_context *ctx) {
    goto_addr(ctx, ctx->fetched_data, true);
}

// interrupted jump to a especifically location?
static void proc_rst(cpu_context *ctx) {
    goto_addr(ctx, ctx->cur_inst->param, true);
}

static void proc_ret(cpu_context *ctx) {
    // special case?
    if (ctx->cur_inst->cond != CT_NONE) {
        emu_cycles(1);
    }

    if (check_cond(ctx)) {
        // you could do pop16 but it does this to keep cycle acurate
        u16 lo = stack_pop();
        emu_cycles(1);
        u16 hi = stack_pop();
        emu_cycles(1);

        u16 n = (hi << 8) | lo;
        ctx->regs.pc = n;

        emu_cycles(1);
    }
}

//similar but returned from an interrupt
static void proc_reti(cpu_context *ctx) {
    ctx->int_master_enable = true; // re-enable master interrupt flag
    proc_ret(ctx);
}

static void proc_pop(cpu_context *ctx) {
    u16 lo = stack_pop();
    emu_cycles(1);
    u16 hi = stack_pop();
    emu_cycles(1);

    u16 n = (hi << 8) | lo;

    cpu_set_reg(ctx->cur_inst->reg1, n);

    // special case
    if (ctx->cur_inst->reg1 == RT_AF) {
        cpu_set_reg(ctx->cur_inst->reg1, n & 0xFFF0);
    }
}

static void proc_push(cpu_context *ctx) {
    u16 hi = (cpu_read_reg(ctx->cur_inst->reg1) >> 8) & 0xFF;
    emu_cycles(1);
    stack_push(hi);

    u16 lo = cpu_read_reg(ctx->cur_inst->reg2) & 0xFF;
    emu_cycles(1);
    stack_push(lo);

    emu_cycles(1);
}

static void proc_di(cpu_context *ctx) {
    ctx->int_master_enable = false;
}

// this allow us to run multiple cpu at the same time
void cpu_set_flags(cpu_context *ctx, char z, char n, char h, char c) {
    if (z != -1) {
        BIT_SET(ctx->regs.f, 7, z);
    }

    if (n != -1) {
        BIT_SET(ctx->regs.f, 6, n);
    }

    if (h != -1) {
        BIT_SET(ctx->regs.f, 5, h);
    }

    if (c != -1) {
        BIT_SET(ctx->regs.f, 4, c);
    }
}

// load in the hram (high ram)
static void proc_ldh(cpu_context *ctx) {
    if (ctx->cur_inst->reg1 == RT_A) {
        cpu_set_reg(ctx->cur_inst->reg1, bus_read(0xFF00 | ctx->fetched_data));
    } else {
        bus_write(0xFF00 | ctx->fetched_data, ctx->cur_inst->reg2);
    }

    emu_cycles(1);
}

static void proc_xor(cpu_context *ctx) {
    ctx->regs.a ^= ctx->fetched_data & 0xFF;
    cpu_set_flags(ctx, ctx->regs.a == 0, 0, 0, 0);
}

// array of function pointers (can think is a hashmap)
static IN_PROC processors[] = {
    [IN_NONE] = proc_none,
    [IN_NOP] = proc_nop,
    [IN_LD] = proc_ld,
    [IN_LDH] = proc_ldh,
    [IN_JP] = proc_jp,
    [IN_DI] = proc_di, // disable interrupts
    [IN_POP] = proc_pop,
    [IN_PUSH] = proc_push,
    [IN_JR] = proc_jr,
    [IN_CALL] = proc_call,
    [IN_RET] = proc_ret,
    [IN_RST] = proc_rst,
    [IN_RETI] = proc_reti,
    [IN_XOR] = proc_xor,
};

IN_PROC inst_get_processor(in_type type) {
    return processors[type];
}
