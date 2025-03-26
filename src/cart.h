#ifndef CART_H
#define CART_H

#include "common.h"

typedef struct {
    u8 entry[4];   // 0x0100 - 0x0103
    u8 logo[0x30]; // or 48 bytes

    char title[16];
    u16 new_lic_code;
    u8 sgb_flag;
    u8 type;
    u8 rom_size;
    u8 ram_size;
    u8 dest_code;
    u8 lic_code; // old licence code
    u8 version;
    u8 checksum;
    u16 global_checksum;
} rom_header;

bool cart_load(char *cart);

#endif