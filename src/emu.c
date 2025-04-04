#include "emu.h"
#include "cart.h"
#include "cpu.h"
#include "ui.h"
#include <stdio.h>

// TODO add windows alternative
#include <pthread.h>
#include <unistd.h>

/*
  Emu components:

  |Cart|        (load cartdrigde & read data from it & in some cases write data to it)
  |CPU|         (instructions & registers)
  |Address Bus| (reading & writing to two addresses)
  |PPU|         (pixel procesing unit)
  |Timer|       (keeps the timer process for the thing in general)

*/

static emu_context ctx; // static so it's only accesible inside this file

emu_context *emu_get_context() {
    return &ctx;
}

// the main thread for the cpu
// we want our cpu running on separate thread than out ui,
// so we can pause it and all that without affecting the ui
void *cpu_run(void *p) {
    cpu_init();
    ctx.running = true;
    ctx.paused = false;
    ctx.ticks = 0;

    while(ctx.running) {
        if (ctx.paused) {
            delay(10);
            continue;
        }

        if (!cpu_step()) {
            printf("**cpu stoped!**");
            return 0;
        }

        ctx.ticks++;
    }

    return 0;
}

int emu_run(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: emu <rom_file>\n");
        return -1;
    }

    if (!cart_load(argv[1])) {
        printf("Failed to load ROM file: %s\n", argv[1]);
        return -2;
    }

    printf("**Cart loaded**\n");

    ui_init();

    pthread_t t1;
    if (pthread_create(&t1, NULL, cpu_run, NULL)) {
        fprintf(stderr, "FAILED TO START MAIN CPU THREAD!\n");
        return -1;
    }

    while(!ctx.die) {
        usleep(1000); // micro seconds
        ui_handle_events();
    }

    return 0;
}

void emu_cycles(int cpu_cycles) {
    // TODO...
}