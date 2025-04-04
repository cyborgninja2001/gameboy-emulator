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

reg_type rt_lookup[] = {
    RT_B,
    RT_C,
    RT_D,
    RT_E,
    RT_H,
    RT_L,
    RT_HL,
    RT_A,
};

reg_type decode_reg(u8 reg) {
    if (reg > 0b111) {
        return RT_NONE;
    }

    return rt_lookup[reg];
}

static void proc_cb(cpu_context *ctx) {
    // we're going to decode that second byte that comes right after the cb
    u8 op = ctx->fetched_data;
    reg_type reg = decode_reg(op & 0b111); // mask the bottom 3 bits
    u8 bit = (op >> 3) & 0b111; // now we need to figure out which bit is being used
    u8 bit_op = (op >> 6) & 0b11; // we get the operation
    u8 reg_val = cpu_read_reg8(reg);

    emu_cycles(1);

    if (reg == RT_HL) { // special case??
        emu_cycles(2);
    }

    switch(op) {
        case 1:
            //BIT
            // it gets whether or not the bit is set on that register
            cpu_set_flags(ctx, !(reg_val & (1 << bit)), 0, 1, -1);
            return;

        case 2:
            //RST (reset)
            //reset the bit on that register
            reg_val &= ~(1 << bit);
            cpu_set_reg8(reg, reg_val);
            return;

        case 3:
            //SET
            reg_val |= (1 << bit);
            cpu_set_reg8(reg, reg_val);
            return;
    }

    // if it isn't one of those 3 values: we move on to the next section ->
    bool flagC = CPU_FLAG_C;

    // switch the bit that's being worked on
    switch(bit) {
        case 0: {
            //RLC (rotate left all bit 7 to the carry flag)
            bool setC = false;
            u8 result = (reg_val << 1) & 0xFF;

            if ((reg_val & (1 << 7)) != 0) {
                // if bit seven is not set on the reg_val
                result |= 1;
                setC = true;
            }

            cpu_set_reg8(reg, result);
            cpu_set_flags(ctx, result == 0, false, false, setC);
        } return;

        case 1: {
            //RRC (rotate right all bit 7 to carry flag)
            u8 old = reg_val;
            reg_val >>= 1;
            reg_val |= (old << 7);
            cpu_set_reg8(reg, reg_val);
            cpu_set_flags(ctx, !reg_val, false, false, old & 1);
        } return;

        case 2: {
            //RL (rotate left)
            u8 old = reg_val;
            reg_val <<= 1;
            reg_val |= flagC;

            cpu_set_reg8(reg, reg_val);
            cpu_set_flags(ctx, !reg_val, false, false, !!(old & 0x80));
        } return;

        case 3: {
            //RR (rotate right)
            u8 old = reg_val;
            reg_val >>= 1;

            reg_val |= (flagC << 7);

            cpu_set_reg8(reg, reg_val);
            cpu_set_flags(ctx, !reg_val, false, false, old & 1);
        } return;

        case 4: {
            //SLA (shift left and carry)
            u8 old = reg_val;
            reg_val <<= 1;

            cpu_set_reg8(reg, reg_val);
            cpu_set_flags(ctx, !reg_val, false, false, !!(old & 0x80));
        } return;

        case 5: {
            //SRA
            u8 u = (int8_t)reg_val >> 1;
            cpu_set_reg8(reg, u);
            cpu_set_flags(ctx, !u, 0, 0, reg_val & 1);
        } return;

        case 6: {
            //SWAP (swap the 2 nibbles of the byte? o algo asi)
            reg_val = ((reg_val & 0xF0) >> 4) | ((reg_val & 0xF) << 4);
            cpu_set_reg8(reg, reg_val);
            cpu_set_flags(ctx, reg_val == 0, false, false, false);
        } return;

        case 7: {
            //SRL (most significant bit is set to zero?)
            u8 u = reg_val >> 1;
            cpu_set_reg8(reg, u);
            cpu_set_flags(ctx, !u, 0, 0, reg_val & 1);
        } return;
    }

    // he said that he thinks this is not possible pero por las dudas
    fprintf(stderr, "ERROR: INVALID CB; %02X", op);
    NO_IMPLEMENTED
}

static void proc_rlca(cpu_context * ctx) {
    u8 u = ctx->regs.a;
    bool c = (u >> 7) & 1;
    u = (u << 1) | c;
    ctx->regs.a = u;

    cpu_set_flags(ctx, 0, 0, 0, c);
}

static void proc_rrca(cpu_context * ctx) {
    u8 b = ctx->regs.a & 1;
    ctx->regs.a >>= 1;
    ctx->regs.a |= (b << 7);

    cpu_set_flags(ctx, 0, 0, 0, b);
}

static void proc_rla(cpu_context * ctx) {
    u8 u = ctx->regs.a;
    u8 cf = CPU_FLAG_C;
    u8 c = (u >> 7) & 1;

    ctx->regs.a = (u << 1) | cf;
    cpu_set_flags(ctx, 0, 0, 0 ,c);
}

// eve the guy low devel level doesn't undestand this but he read that from other
// implementations
static void proc_daa(cpu_context * ctx) {
    u8 u = 0;
    int fc = 0;

    if (CPU_FLAG_C || (!CPU_FLAG_N && ((ctx->regs.a & 0xF) > 9))) {
        u = 6;
    }

    if (CPU_FLAG_C || (!CPU_FLAG_N && ctx->regs.a > 0x99)) {
        u |= 0x60;
        fc = 1;
    }

    ctx->regs.a += CPU_FLAG_N ? -u : u;

    cpu_set_flags(ctx, ctx->regs.a == 0, -1, 0, fc);
}

static void proc_cpl(cpu_context * ctx) {
    ctx->regs.a = ~ctx->regs.a;
    cpu_set_flags(ctx, -1, 1, 1, -1);
}

static void proc_scf(cpu_context * ctx) {
    cpu_set_flags(ctx, -1, 0, 0, 1);
}

static void proc_ccf(cpu_context * ctx) {
    cpu_set_flags(ctx, -1, 0, 0, CPU_FLAG_C ^ 1);
}

static void proc_halt(cpu_context * ctx) {
    ctx->halted = true;
}

static void proc_stop(cpu_context * ctx) {
    fprintf(stderr, "STOPPING!\n");
    NO_IMPLEMENTED
}

static void proc_rra(cpu_context * ctx) {
    u8 carry = CPU_FLAG_C;
    u8 new_c = ctx->regs.a & 1;

    ctx->regs.a >>=  1;
    ctx->regs.a |= (carry << 7);

    cpu_set_flags(ctx, 0, 0, 0, new_c);
}




static void proc_and(cpu_context *ctx) {
    // all and operations are being done on a
    ctx->regs.a &= ctx->fetched_data;
    cpu_set_flags(ctx, ctx->regs.a == 0, 0, 1, 0);

}

static void proc_or(cpu_context *ctx) {
    // all xor operations are being done on a
    ctx->regs.a |= ctx->fetched_data & 0xFF;
    cpu_set_flags(ctx, ctx->regs.a == 0, 0, 0, 0);
}

static void proc_cp(cpu_context *ctx) {
    int n = (int)ctx->regs.a - (int)ctx->fetched_data;

    cpu_set_flags(ctx, n == 0, 1, ((int)ctx->regs.a & 0x0F) - ((int)ctx->fetched_data & 0x0F) < 0, n < 0);
}

static bool is_16_bit(reg_type rt) {
    return rt >= RT_AF;
}

static void proc_ld(cpu_context *ctx) {
    // special case:
    if (ctx->dest_is_mem) {
        // LD (BC), A for instance ...
        if (is_16_bit(ctx->cur_inst->reg2)) {
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

static void proc_ei(cpu_context *ctx) {
    ctx->enabling_ime = true;
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

static void proc_inc(cpu_context *ctx) {
    u16 val = cpu_read_reg(ctx->cur_inst->reg1) + 1;

    if (is_16_bit(ctx->cur_inst->reg1)) {
        emu_cycles(1);
    }

    if (ctx->cur_inst->reg1 == RT_HL && ctx->cur_inst->mode == AM_MR) {
        val = bus_read(cpu_read_reg(RT_HL)) + 1;
        val &= 0xFF;
        bus_write(cpu_read_reg(RT_HL), val);
    } else {
        cpu_set_reg(ctx->cur_inst->reg1, val);
        val = cpu_read_reg(ctx->cur_inst->reg1); // re-read the value afterwards why??
    }

    if ((ctx->cur_opcode & 0x03) == 0x03) {
        return; // si el opcode termina en 0x...3 salimos pq estos no setean flags
    }

    cpu_set_flags(ctx, val == 0, 0, (val & 0x0F) == 0, -1);
}

static void proc_dec(cpu_context *ctx) {
    u16 val = cpu_read_reg(ctx->cur_inst->reg1) - 1;

    if (is_16_bit(ctx->cur_inst->reg1)) {
        emu_cycles(1);
    }

    if (ctx->cur_inst->reg1 == RT_HL && ctx->cur_inst->mode == AM_MR) {
        val = bus_read(cpu_read_reg(RT_HL)) - 1;
        bus_write(cpu_read_reg(RT_HL), val);
    } else {
        cpu_set_reg(ctx->cur_inst->reg1, val);
        val = cpu_read_reg(ctx->cur_inst->reg1); // re-read the value afterwards why??
    }

    if ((ctx->cur_opcode & 0x0B) == 0x0B) {
        return; // si el opcode termina en 0x...B salimos pq estos no setean flags
    }

    cpu_set_flags(ctx, val == 0, 1, (val & 0x0F) == 0x0F, -1);
}

static void proc_sub(cpu_context *ctx) {
    u16 val = cpu_read_reg(ctx->cur_inst->reg1) - ctx->fetched_data;

    int z = val == 0;
    int h = ((int)cpu_read_reg(ctx->cur_inst->reg1) & 0xF) - ((int) ctx->fetched_data & 0xF) < 0;
    int c = ((int)cpu_read_reg(ctx->cur_inst->reg1) & 0xF) - ((int) ctx->fetched_data & 0xF) < 0;

    cpu_set_reg(ctx->cur_inst->reg1, val);
    cpu_set_flags(ctx, z, 1, h, c);
}

static void proc_sbc(cpu_context *ctx) {
    u8 val = ctx->fetched_data + CPU_FLAG_C;

    int z = cpu_read_reg(ctx->cur_inst->reg1) - val == 0;
    int h = ((int)cpu_read_reg(ctx->cur_inst->reg1) & 0xF)
        - ((int) ctx->fetched_data & 0xF) - ((int)CPU_FLAG_C) < 0;
    int c = ((int)cpu_read_reg(ctx->cur_inst->reg1) & 0xF)
        - ((int) ctx->fetched_data & 0xF) - ((int)CPU_FLAG_C) < 0;

    cpu_set_reg(ctx->cur_inst->reg1, cpu_read_reg(ctx->cur_inst->reg1) - val);
    cpu_set_flags(ctx, z, 1, h, c);
}

static void proc_adc(cpu_context *ctx) {
    // these are all oging to be 8bit registers
    u16 u = ctx->fetched_data;
    u16 a = ctx->regs.a;
    u16 c = CPU_FLAG_C;

    ctx->regs.a = (u + a + c) & 0xFF; // el resultado debe ser de 8 bits
    cpu_set_flags(ctx, ctx->regs.a = 0, 0,
        (a & 0xF) + (u & 0xF) + c > 0xF, // half carry
        a + u + c > 0xFF);
}

static void proc_add(cpu_context *ctx) {
    // usa u32 pq puede llegar a sumar valores de 16 bits y hay que chequear si hubo overflow
    u32 val = cpu_read_reg(ctx->cur_inst->reg1) + ctx->fetched_data;

    bool is_16bit = is_16_bit(ctx->cur_inst->reg1);

    if (is_16bit) {
        emu_cycles(1);
    }

    if (ctx->cur_inst->reg1 == RT_SP) {
        val = cpu_read_reg(ctx->cur_inst->reg1) + (char)ctx->fetched_data; // it might be negative
    }

    int z = (val & 0xFF) == 0;
    int h = (cpu_read_reg(ctx->cur_inst->reg1) & 0xF) + (ctx->fetched_data & 0xF) >= 0x10;
    int c = (int)(cpu_read_reg(ctx->cur_inst->reg1) & 0xFF) + (int)(ctx->fetched_data & 0xFF) >= 0x100;

    if (is_16_bit) {
        z = -1;
        h = (cpu_read_reg(ctx->cur_inst->reg1) & 0xFFF) + (ctx->fetched_data & 0xFFF) >= 0x1000;
        u32 n = ((u32)cpu_read_reg(ctx->cur_inst->reg1)) + ((u32)ctx->fetched_data);
        c = n >= 0x10000;
    }

    // another special case
    if (ctx->cur_inst->reg1 == RT_SP) {
        z = 0;
        h = (cpu_read_reg(ctx->cur_inst->reg1) & 0xF) + (ctx->fetched_data & 0xF) >= 0x10;
        c = (int)(cpu_read_reg(ctx->cur_inst->reg1) & 0xFF) + (int)(ctx->fetched_data & 0xFF) > 0x100;
    }

    cpu_set_reg(ctx->cur_inst->reg1, val & 0xFFFF);
    cpu_set_flags(ctx, z, 0, h, c);
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
    [IN_DEC] = proc_dec,
    [IN_INC] = proc_inc,
    [IN_ADD] = proc_add,
    [IN_ADC] = proc_adc,
    [IN_SUB] = proc_sub,
    [IN_SBC] = proc_sbc,
    [IN_AND] = proc_and,
    [IN_OR] = proc_or,
    [IN_CP] = proc_cp,
    [IN_CB] = proc_cb,
    [IN_RRCA] = proc_rrca,
    [IN_RLCA] = proc_rlca,
    [IN_RLA] = proc_rla,
    [IN_RRA] = proc_rra,
    [IN_STOP] = proc_stop,
    [IN_RETI] = proc_reti,
    [IN_HALT] = proc_halt,
    [IN_DAA] = proc_daa,
    [IN_CPL] = proc_cpl,
    [IN_SCF] = proc_scf,
    [IN_CCF] = proc_ccf,
    [IN_EI] = proc_ei,
    [IN_XOR] = proc_xor,
};

IN_PROC inst_get_processor(in_type type) {
    return processors[type];
}
