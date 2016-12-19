#include <stdio.h>
#include <stdlib.h>

#include "spdk_interface.h"
#include "inode_block.h"
#include "data_block.h"
#include "common.h"
#include "bitmap.h"

bool 
get_data_location(unsigned long data_num, unsigned long *block_num)
{
	*block_num = data_num + DATA_BLOCK_START;
	
	if( *block_num >= LOGIC_BLOCK_NUM ){
		fprintf(stderr, "The number of data block overflows!\n");
		return false;
	}

	return true;
}

bool
read_dir_block(unsigned long data_num, struct dir_entry *buf)
{
	unsigned long block_num;

	if (!get_data_location(data_num, &block_num)) {
		return false;
	}
	
	spdk_read_and_write((char *)buf, block_num, 1, READ);

	return true;
}

bool
write_dir_block(unsigned long data_num, struct dir_entry *buf)
{
	unsigned long block_num;

	if (!get_data_location(data_num, &block_num)) {
		return false;
	}

	spdk_read_and_write((char *)buf, block_num, 1, WRITE);

	set_bitmap(data_num, LOGIC_BITMAP, 1);

	return true;
}

bool
read_data_block(unsigned long data_num, char *buf)
{
	unsigned long block_num;

	if (!get_data_location(data_num, &block_num)) {
		return false;
	}

	spdk_read_and_write(buf, block_num, 1, READ);

	return true;
}

bool
write_data_block(unsigned long data_num, char *buf)
{
	unsigned long block_num;
	
	if (!get_data_location(data_num, &block_num)) {
		return false;
	}

	spdk_read_and_write(buf, block_num, 1, WRITE);

	set_bitmap(data_num, LOGIC_BITMAP, 1);

	return true;
}

bool
del_data_block(unsigned long data_num)
{
	set_bitmap(data_num, LOGIC_BITMAP, 0);
	return true;
}

bool
find_null_data_block_num(unsigned long *data_num)
{
	if (!find_bitmap_first_zero(LOGIC_BITMAP, data_num)) {
		return false;
	}

	return true;
}

