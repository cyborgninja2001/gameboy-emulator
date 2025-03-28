#ifndef EMU_H
#define EMU_H

#include "common.h"

typedef struct {
    bool paused;
    bool running;
    u64 ticks;    // related to emu cycles?

} emu_context;

int emu_run(int argc, char *argv[]);

// in case it needs to be used eventually for example ui
emu_context *emu_get_context();

// we use this to syncronize to our ppu and timer, etc...
void emu_cycles(int cpu_cycles);

#endif