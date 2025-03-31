#include "bus.h"
#include "cart.h"
#include "ram.h"
#include "cpu.h"

/*
Memory Map:
Start	End	Description	Notes
0000	3FFF	16 KiB ROM bank 00	From cartridge, usually a fixed bank
4000	7FFF	16 KiB ROM Bank 01-NN	From cartridge, switchable bank via mapper (if any)
8000	9FFF	8 KiB Video RAM (VRAM)	In CGB mode, switchable bank 0/1
A000	BFFF	8 KiB External RAM	From cartridge, switchable bank if any
C000	CFFF	4 KiB Work RAM (WRAM)
D000	DFFF	4 KiB Work RAM (WRAM)	In CGB mode, switchable bank 1-7
E000	FDFF	Echo RAM (mirror of C000-DDFF)	Nintendo says use of this area is prohibited.
FE00	FE9F	Object attribute memory (OAM)
FEA0	FEFF	Not Usable	Nintendo says use of this area is prohibited.
FF00	FF7F	I/O Registers
FF80	FFFE	High RAM (HRAM)
FFFF	FFFF	Interrupt Enable register (IE)

*/


u8 bus_read(u16 address) {
    // We're gonna handle the rom for now
    if (address < 0x8000) {
        // ROM Data
        return cart_read(address); // we read from the rom
    } else if (address < 0xA000) {
        //Char/Map Data
        // TODO
        printf("UNSUPPORTED bus_read(%04X)\n", address);
        NO_IMPLEMENTED
    } else if (address < 0xC000) {
        // external RAM (cartdrige RAM)
        return cart_read(address);
    } else if (address < 0xE000) {
        // WRAM (Working RAM)
        return wram_read(address);
    } else if (address < 0xFE00) {
        // reserved echo ram...
        return 0; // nintendo says it's prohibited to use this area
    } else if (address < 0xFEA0) {
        // OAM (for the ppu?)
        // TODO
        printf("UNSUPPORTED bus_read(%04X)\n", address);
        NO_IMPLEMENTED
    } else if (address < 0xFF00) {
        // reserved unusable
        return 0;
    } else if (address < 0xFF80) {
        // IO registers
        // TODO
        printf("UNSUPPORTED bus_read(%04X)\n", address);
        NO_IMPLEMENTED
    } else if (address == 0xFFFF) {
        // CPU enable register
        return cpu_get_ie_register();
    }

    // everything left over is gonna be the high RAM (HRAM) < 0xFFFF
    return hram_read(address);
}

void bus_write(u16 address, u8 value) {
    if (address < 0x8000) {
        // ROM Data
        cart_write(address, value); // we write to the cartdrige
    } else if (address < 0xA000) {
        // Char/Map Data
        // TODO
        printf("UNSUPPORTED bus_write(%04X)\n", address);
        NO_IMPLEMENTED
    } else if (address < 0xC000) {
        // external RAM (cartdrige RAM)
        cart_write(address, value);
    } else if (address < 0xE000) {
        // WRAM
        wram_write(address, value);
    } else if (address < 0xFE00) {
        // reserved ram
    } else if (address < 0xFEA0) {
        // OAM
        printf("UNSUPPORTED bus_write(%04X)\n", address);
        NO_IMPLEMENTED
    } else if (address < 0xFF00) {
        // unusable reserved
    } else if (address < 0xFF80) {
        // IO registers
        //TODO
        printf("UNSUPPORTED bus_write(%04X)\n", address);
        //NO_IMPLEMENTED
    } else if (address == 0xFFFF) {
        // CPU SET enable register
        cpu_set_ie_register(value);
    } else {
        hram_write(address, value);
    }
}

u16 bus_read16(u16 address) {
    u8 lo = bus_read(address);
    u8 hi = bus_read(address + 1);
    return lo | (hi << 8);
}

void bus_write16(u16 address, u16 value) {
    bus_write(address + 1, (value >> 8) & 0xFF);
    bus_write(address, value & 0xFF);
}