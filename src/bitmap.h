#ifndef BITMAP_H
#define BITMAP_H

#include "common.h"
#include "spdk_interface.h"

void clear_block(unsigned long addr);
bool set_bit(unsigned long nr, unsigned long addr, int flag);
void get_bit(unsigned long nr, unsigned long addr, int *res);
void find_first_zero(unsigned long addr, unsigned long *nr);
bool set_bitmap(unsigned long num, int block_type, int flag);
bool get_bitmap(unsigned long num, int block_type, int *res);

#endif //BITMAP_H
