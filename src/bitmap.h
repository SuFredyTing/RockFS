#ifndef BITMAP_H
#define BITMAP_H

#include "common.h"
#include "spdk_interface.h"

void clear_block(int addr);
bool set_bit(int nr, int addr, int flag);
void get_bit(int nr, int addr, int *res);
void find_first_zero(int addr, int *nr);

#endif //BITMAP_H
