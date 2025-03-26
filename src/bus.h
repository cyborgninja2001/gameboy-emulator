#ifndef BUS_H
#define BUS_H

#include "common.h"

u8 bus_read(u16 address);
void bus_write(u16 address, u8 value);

#endif