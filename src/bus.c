#include "bus.h"
#include "cart.h"

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
    }

    printf("UNSUPPORTED bus_read(%04X)\n", address); // log
    //NO_IMPLEMENTED
}

void bus_write(u16 address, u8 value) {
    if (address < 0x8000) {
        // ROM Data
        cart_write(address, value); // we write to the cartdrige
        return;
    }

    printf("UNSUPPORTED bus_write(%04X)\n", address); // log
    //NO_IMPLEMENTED
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