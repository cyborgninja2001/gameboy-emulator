#include "emu.h"
#include "cart.h"
#include "cpu.h"
#include <SDL2/SDL.h>      // graphics library
#include <SDL2/SDL_ttf.h>  // true type fonts ?
#include <stdio.h>

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

void delay(u32 ms) {
    SDL_Delay(ms);
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

    SDL_Init(SDL_INIT_VIDEO); // initialize SDL
    printf("SDL init\n");

    TTF_Init();
    printf("TTF init\n");

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
            return -3;
        }

        ctx.ticks++;
    }

    return 0;
}

void emu_cycles(int cpu_cycles) {
    // TODO...
}