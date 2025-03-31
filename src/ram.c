#include "ram.h"

typedef struct {
    u8 wram[0x2000]; // 8KB = 8192 bytes
    u8 hram[0x80];   // 127 bytes
} ram_context;

static ram_context ctx;

u8 wram_read(u16 address) {
    // sino haces esto te pueden dar una direccion que caiga afuera del array
    address -= 0xC000; // because wram starts there

    if (address >= 0x2000) {
        printf("INVALID WRAM ADDRESS: %08X\n", address + 0xC000);
        exit(-1);
    }

    return ctx.wram[address];
}

void wram_write(u16 address, u8 value) {
    address -= 0xC000;
    ctx.wram[address] = value;
}

u8 hram_read(u16 address) {
    address -= 0xFF80;
    return ctx.hram[address];
}

void hram_write(u16 address, u8 value) {
    address -= 0xFF80;
    ctx.hram[address] = value;
}
