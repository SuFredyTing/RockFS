#include <stdio.h>
#include <stdlib.h>

#include "bitmap.h"
#include "common.h"
#include "spdk_interface.h"

void 
clear_block(unsigned long addr)
{
	char *buf = (char *)malloc(BLOCK_SIZE);
	int i;
	
	spdk_read_and_write(buf, addr, 1, READ);

	for (i = 0; i < BLOCK_SIZE; i++) {
		buf[i] &= 0x00;
	}

	spdk_read_and_write(buf, addr, 1, WRITE);
	
	free(buf);
} 

bool
set_bit(unsigned long nr, unsigned long addr, int flag)
{
	char *buf = (char *)malloc(BLOCK_SIZE);
    int tmp = 1;
	int z, r;

	z = nr / 8;
	r = nr % 8;

	spdk_read_and_write(buf, addr, 1, READ);
	
	tmp = tmp << r;
	if (flag == 1) {
		buf[z] |= (char)tmp;
	} else if (flag == 0) {
		buf[z] &= ~(char)tmp;
	} else {
		fprintf(stderr,"The flag in the function of set_ bit() is error!\n");
		free(buf);
		return false;
	}

	spdk_read_and_write(buf, addr, 1, WRITE);
	
	free(buf);
	
	return true;
}

void 
get_bit(unsigned long nr, unsigned long addr, int *res)
{
	char *buf = (char *)malloc(BLOCK_SIZE);
    int tmp = 1;
	int z, r;

	z = nr / 8;
	r = nr % 8;

	spdk_read_and_write(buf, addr, 1, READ);
	
	tmp = tmp << r;
	tmp = (int)(buf[z] & (char)tmp);
	*res = tmp >> r;

	free(buf);
}

void
find_first_zero(unsigned long addr, unsigned long *nr)
{
	char *buf = (char *)malloc(BLOCK_SIZE);
	int tmp;
	int res, flag = 0;
	unsigned long i, j;
	
	spdk_read_and_write(buf, addr, 1, READ);
   
	for (i = 0; i < BLOCK_SIZE; i++) {
		for (j = 0; j < 8; j++) {
			tmp = 0x01 << j;
			tmp = (char)tmp & buf[i];
			res = (tmp >> j);
			if (res == 0) {
				flag = 1;
				break;
			}
		}
		if (flag == 1)
			break;
	}
	
	(*nr) = i * 8 + j;

	free(buf);
}

bool
set_bitmap(unsigned long num, int block_type, int flag)
{
	unsigned long block_num, inter_num;
	
	block_num = num / (BLOCK_SIZE * 8);
	inter_num = num % (BLOCK_SIZE * 8);	
	
	if (block_type == INODE_BITMAP) {
		
		block_num += INODE_BLOCK_BITMAP_START;
		if (block_num >= LOGIC_BLOCK_BITMAP_START) {
			fprintf(stderr, "The number of inode block bitmap overflows!");
			return false;
		}
			
	} else if (block_type == LOGIC_BITMAP) {
		
		block_num += LOGIC_BLOCK_BITMAP_START;
		if (block_num >= INODE_BLOCK_START) {
			fprintf(stderr, "The number of logic block bitmap overflows!");
			return false;
		}

	}

	set_bit(inter_num, block_num, flag);
	
	return true;
}

bool
get_bitmap(unsigned long num, int block_type, int *res)
{
	unsigned long block_num, inter_num;
	
	block_num = num / (BLOCK_SIZE * 8);
	inter_num = num % (BLOCK_SIZE * 8);	
	
	if (block_type == INODE_BITMAP) {
		
		block_num += INODE_BLOCK_BITMAP_START;
		if (block_num >= LOGIC_BLOCK_BITMAP_START) {
			fprintf(stderr, "The number of inode block bitmap overflows!");
			return false;
		}
			
	} else if (block_type == LOGIC_BITMAP) {
		
		block_num += LOGIC_BLOCK_BITMAP_START;
		if (block_num >= INODE_BLOCK_START) {
			fprintf(stderr, "The number of logic block bitmap overflows!");
			return false;
		}

	}

	get_bit(inter_num, block_num, res);
	
	return true;
}

/*
int 
main ()
{
	char *buf = (char *)malloc(BLOCK_SIZE);
	int i, j;
	int tmp;

	spdk_init();
	
	//printf("init word:\n");
	//for(i = 0; i < BLOCK_SIZE; i++) {
	//	buf[i] = 0x00;
	//	printf("%02X ", buf[i]);
	//}
	printf("\nend!\n");
	
	clear_block(2);
	//set_bit(2, 2, 1);
	//get_bit(3, 2, &tmp);
	//find_first_zero(2, &tmp);
	//printf("res = %d\n", tmp);
	//spdk_read_and_write(buf, 2, 1, 2);
	spdk_read_and_write(buf, 2, 1, 1);

	printf("read word:\n");
	for(i = 0; i < BLOCK_SIZE; i++) {
		printf("%02X ", buf[i]);
	}
	printf("\nend!\n");
    
	find_first_zero(2, &tmp);
	printf("res = %d\n", tmp);

	spdk_cleanup();
	free(buf);

	return 0;
}
*/
