#include "cpu.h"
#include "bus.h"
#include "emu.h"

cpu_context ctx = {0};

void cpu_init() {
    ctx.regs.pc = 0x100;
    ctx.regs.a = 0x01;
}

static void  fetch_instruction() {
    // fetch the opcode & increment by 1 the pc
    ctx.cur_opcode = bus_read(ctx.regs.pc++); // reads addr from ctx.regs.pc and then pc++
    ctx.cur_inst = instruction_by_opcode(ctx.cur_opcode);
}

static void execute() {
    // we get the function for the instruction
    IN_PROC proc = inst_get_processor(ctx.cur_inst->type);

    if (!proc) { // we exit the app?
        NO_IMPLEMENTED
    }

    // otherwise we found the function & we execute it
    proc(&ctx);
}

bool cpu_step() {
    if (!ctx.halted) {
        u16 pc = ctx.regs.pc;
        fetch_instruction(); // fetch
        fetch_data();        // decode

        // for logging
        printf("%04X:   %-7s (%02X %02X %02X) A: %02X B: %02X C: %02X\n",
            pc, inst_name(ctx.cur_inst->type), ctx.cur_opcode,
            bus_read(pc + 1), bus_read(pc + 2), ctx.regs.a, ctx.regs.b, ctx.regs.c
        );

        // error checking:
        if (ctx.cur_inst == NULL) {
            printf("Unknown Instruction: %02X\n", ctx.cur_inst);
            exit(-7);
        }

        execute();
    }
    return true;
}