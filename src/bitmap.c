#include <stdio.h>
#include <stdlib.h>

#include "spdk_interface.h"

#define BLOCK_SIZE 4096

void 
clear_block(int addr)
{
	char *buf = (char *)malloc(BLOCK_SIZE);
	int i;
	
	spdk_read_and_write(buf, addr, 1, 1);

	for (i = 0; i < BLOCK_SIZE; i++) {
		buf[i] &= 0x00;
	}

	spdk_read_and_write(buf, addr, 1, 2);
	
	free(buf);
} 

bool
set_bit(int nr, int addr, int flag)
{
	char *buf = (char *)malloc(BLOCK_SIZE);
    int tmp = 1;
	int z, r;

	z = nr / 8;
	r = nr % 8;

	spdk_read_and_write(buf, addr, 1, 1);
	
	tmp = tmp << r;
	if (flag == 1) {
		buf[z] |= (char)tmp;
	} else if (flag == 0) {
		buf[z] &= ~(char)tmp;
	} else {
		return false;
	}

	spdk_read_and_write(buf, addr, 1, 2);
	
	free(buf);
	
	return true;
}

void 
get_bit(int nr, int addr, int *res)
{
	char *buf = (char *)malloc(BLOCK_SIZE);
    int tmp = 1;
	int z, r;

	z = nr / 8;
	r = nr % 8;

	spdk_read_and_write(buf, addr, 1, 1);
	
	tmp = tmp << r;
	tmp = (int)(buf[z] & (char)tmp);
	*res = tmp >> r;

	free(buf);
}

void
find_first_zero(int addr, int *nr)
{
	char *buf = (char *)malloc(BLOCK_SIZE);
	int tmp;
	int i, j, res, flag = 0;
	
	spdk_read_and_write(buf, addr, 1, 1);
   
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

int 
main ()
{
	char *buf = (char *)malloc(BLOCK_SIZE);
	int i, j;
	int tmp;

	spdk_init();
	
	/*printf("init word:\n");
	for(i = 0; i < BLOCK_SIZE; i++) {
		buf[i] = 0x00;
		printf("%02X ", buf[i]);
	}
	printf("\nend!\n");
	*/
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

