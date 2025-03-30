#include "cpu.h"
#include "bus.h"
#include "emu.h"

// static cpu_context ctx;
extern cpu_context ctx;

// fetchea data de la instruction
void fetch_data() {
    ctx.mem_dest = 0;
    ctx.dest_is_mem = false;

    if (ctx.cur_inst == NULL) {
        return;
    }

    switch (ctx.cur_inst->mode) {
        case AM_IMP: return;

        case AM_R:
            ctx.fetched_data = cpu_read_reg(ctx.cur_inst->reg1);
            return;

        case AM_R_R:
            ctx.fetched_data = cpu_read_reg(ctx.cur_inst->reg2);
            return;

        case AM_R_D8:
            ctx.fetched_data = bus_read(ctx.regs.pc); // lee el bit inmediato
            emu_cycles(1);
            ctx.regs.pc++;
            return;

        case AM_R_D16: //same as below
        case AM_D16: {
            u16 lo = bus_read(ctx.regs.pc);
            emu_cycles(1); // thisis just for syncronization

            u16 hi = bus_read(ctx.regs.pc + 1);
            emu_cycles(1);

            ctx.fetched_data = lo | (hi << 8);

            ctx.regs.pc += 2;
            return;
        }

        case AM_MR_R: // load a register into a memory region
            ctx.fetched_data = cpu_read_reg(ctx.cur_inst->reg2);
            ctx.mem_dest = cpu_read_reg(ctx.cur_inst->reg1);
            ctx.dest_is_mem = true;

            // this special case comes from the pdf gb complete technical
            // is a special case????
            if (ctx.cur_inst->reg1 == RT_C) {
                ctx.mem_dest |= 0xFF00;
            }
            return;

        case AM_R_MR: {
            u16 addr = cpu_read_reg(ctx.cur_inst->reg2);

            if (ctx.cur_inst->reg1 == RT_C) {
                addr |= 0xFF00;
            }

            ctx.fetched_data = bus_read(addr);
            emu_cycles(1);
            } return;

        // lee la direccion que tiene HL e incrementalo
        case AM_R_HLI: // or HL+ in table
            ctx.fetched_data = bus_read(cpu_read_reg(ctx.cur_inst->reg2));
            emu_cycles(1);
            cpu_set_reg(RT_HL, cpu_read_reg(RT_HL + 1));
        return;

        case AM_HLI_R:
            ctx.fetched_data = cpu_read_reg(ctx.cur_inst->reg2);
            ctx.mem_dest = cpu_read_reg(ctx.cur_inst->reg1);
            ctx.dest_is_mem = true;
            cpu_set_reg(RT_HL, cpu_read_reg(RT_HL + 1));
            break;

        case AM_HLD_R:
            ctx.fetched_data = cpu_read_reg(ctx.cur_inst->reg2);
            ctx.mem_dest = cpu_read_reg(ctx.cur_inst->reg1);
            ctx.dest_is_mem = true;
            cpu_set_reg(RT_HL, cpu_read_reg(RT_HL - 1));
            break;

        // same but decrement
        case AM_R_HLD: // or HL+ in table
            ctx.fetched_data = bus_read(cpu_read_reg(ctx.cur_inst->reg2));
            emu_cycles(1);
            cpu_set_reg(RT_HL, cpu_read_reg(RT_HL - 1));
        return;

        case AM_R_A8:
            ctx.fetched_data = bus_read(ctx.regs.pc);
            emu_cycles(1);
            ctx.regs.pc++;
            break;

        case AM_A8_R:
            ctx.mem_dest = bus_read(ctx.regs.pc) | 0xFF00;
            ctx.dest_is_mem = true;
            emu_cycles(1);
            ctx.regs.pc++;
            return;

        case AM_HL_SPR: // special case for one instruction (SP+r8)
            ctx.fetched_data = bus_read(ctx.regs.pc);
            emu_cycles(1);
            ctx.regs.pc++;
            return;

        case AM_D8:
            ctx.fetched_data = bus_read(ctx.regs.pc);
            emu_cycles(1);
            ctx.regs.pc++;
            return;

        case AM_A16_R: // same
        case AM_D16_R: {
            u16 lo = bus_read(ctx.regs.pc);
            emu_cycles(1); // thisis just for syncronization

            u16 hi = bus_read(ctx.regs.pc + 1);
            emu_cycles(1);

            ctx.mem_dest = lo | (hi << 8);
            ctx.dest_is_mem = true;
            ctx.regs.pc += 2;
            ctx.fetched_data = cpu_read_reg(ctx.cur_inst->reg2);
        } return;

        case AM_MR_D8:
            ctx.fetched_data = bus_read(ctx.regs.pc);
            emu_cycles(1);
            ctx.regs.pc++;
            ctx.mem_dest = cpu_read_reg(ctx.cur_inst->reg1);
            ctx.dest_is_mem = true;
            return;

        case AM_MR:
            ctx.mem_dest = cpu_read_reg(ctx.cur_inst->reg1);
            ctx.dest_is_mem = true;
            ctx.fetched_data = bus_read(cpu_read_reg(ctx.cur_inst->reg1));
            emu_cycles(1);
            return;

        case AM_R_A16: {
            u16 lo = bus_read(ctx.regs.pc);
            emu_cycles(1); // thisis just for syncronization

            u16 hi = bus_read(ctx.regs.pc + 1);
            emu_cycles(1);

            u16 addr = lo | (hi << 8);
            ctx.regs.pc += 2;
            ctx.fetched_data = bus_read(addr);
            emu_cycles(1);
        } return;

        default:
            printf("Unknown Addressing Mode: %d (%02X)\n", ctx.cur_inst->mode, ctx.cur_opcode);
            exit(-7);
            return;
    }
}