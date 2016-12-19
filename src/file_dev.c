#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#include "common.h"
#include "file_dev.h"
#include "inode_block.h"
#include "data_block.h"
#include "namei.h"

#define MIN(a,b) (((a) < (b)) ? (a) : (b))

bool
get_data_block_list(struct d_inode *data, unsigned long *data_block, int size)
{
	int i, j, k, z, r;
    unsigned long *buf  = (unsigned long *)malloc(BLOCK_SIZE);
    unsigned long *buf1 = (unsigned long *)malloc(BLOCK_SIZE);

    if (size > 0 && size < 9) {

        for (i = 0; i < size; i++) {
            data_block[i] = data->i_zone[i];
        }

    } else if (size < 521) {

        for (i = 0; i < 8; i++) {
            data_block[i] = data->i_zone[i];
        }
        read_data_block(data->i_zone[i], (char *)buf);
        for (j = 0; j + 8 < size; j++, i++) {
            data_block[i] = buf[j];
        }

    } else if (size < 262665) {

        for (i = 0; i < 8; i++) {
            data_block[i] = data->i_zone[i];
        }

        read_data_block(data->i_zone[i], (char *)buf);
        for (j = 0; j + 8 < 520; j++, i++) {
            data_block[i] = buf[j];
        }

        read_data_block(data->i_zone[9], (char *)buf);
        z = (size - 520) / 512;
        r = (size - 520) % 512;
        for (j = 0; j < z; j++) {
            read_data_block(buf[j], (char *)buf1);
            for (k = 0; k < 512; k++, i++) {
                data_block[i] = buf1[k];
            }
        }
        if (r != 0) {
            read_data_block(buf[j], (char *)buf1);
            for (k = 0; k < r; k++, i++) {
                data_block[i] = buf1[k];
            }
        }

    } else {
        fprintf(stderr, "The size of file exceed the limit!\n");

        free(buf);
        free(buf1);

        return false;
    }

    free(buf);
    free(buf1);
    return true;
}

bool
add_data_block_to_list(struct d_inode *data, unsigned long data_num, int size) 
{
	int z, r;
    unsigned long *buf  = (unsigned long *)malloc(BLOCK_SIZE);
    unsigned long *buf1 = (unsigned long *)malloc(BLOCK_SIZE);

    if (size < 8) {
        data->i_zone[size] = data_num;
    } else if (size < 520) {
        read_data_block(data->i_zone[8], (char *)buf);
        buf[size - 8] = data_num;
        write_data_block(data->i_zone[8], (char *)buf);
    } else if (size < 262664) {
        read_data_block(data->i_zone[9], (char *)buf);
        z = (size - 520) / 512;
        r = (size - 520) % 512;
        read_data_block(buf[z], (char *)buf1);
        buf1[r] = data_num;
        write_data_block(buf[z], (char *)buf1);
    } else {
        fprintf(stderr, "The size of file exceed the limit!\n");
        free(buf);
        free(buf1);
    }

    free(buf);
    free(buf1);
    return true;
}

unsigned long 
bmap(struct d_inode *data, int data_block_num)
{
	int block_num;
	unsigned long res;
	unsigned long *block_list;

	block_num = (data->i_size + BLOCK_SIZE - 1) / BLOCK_SIZE;
	block_list = (unsigned long *)malloc(sizeof(unsigned long) * block_num);

	if (data_block_num < block_num) {
		get_data_block_list(data, block_list, block_num);
		res = block_list[data_block_num];
	} else {
		free(block_list);
		return 0;
	}

	free(block_list);
	return res;
}

unsigned long 
create_block(struct d_inode *data, int data_block_num)
{
	int block_num;
	unsigned long res;
	unsigned long *block_list;	

	block_num = (data->i_size + BLOCK_SIZE - 1) / BLOCK_SIZE;	
	block_list = (unsigned long *)malloc(sizeof(unsigned long) * block_num);	

	if (data_block_num < block_num) {
		get_data_block_list(data, block_list, block_num);
		res = block_list[data_block_num];
	} else {
		if (!find_null_data_block_num(&res)) {
			if (block_num != 0){
				free(block_list);
			}	
			return 0;
		}
		if(!add_data_block_to_list(data, res, block_num)) {
			if (block_num != 0){
				free(block_list);
			}	
			return 0;
		}
	}
	if (block_num != 0){
		free(block_list);
	}	
	return res;	
}

int
file_read(struct d_inode *inode, struct file *filp, char *buf, int count)
{
	int left, chars, nr;
	char *r_buf = (char *)malloc(BLOCK_SIZE);
	char *flag;
	
	if ((left = count) <= 0) {
		return 0;
	}

	while (left) {
		if ((nr = bmap(inode, (filp->f_pos) / BLOCK_SIZE))) {
			if (!(read_data_block(nr, r_buf))) {
				break;
			}
			flag = r_buf;
		}else
			flag = NULL;
		nr = filp->f_pos % BLOCK_SIZE;
		chars = MIN(BLOCK_SIZE - nr, left);	
		filp->f_pos += chars;
		left -= chars;
		if (flag) {
			char *p = nr + r_buf;
			while (chars-- > 0)
				put_fs_byte(*(p++), buf++);
		} else {
			while (chars-- > 0)
				put_fs_byte(0, buf++);
		}		
	}

	free(r_buf);
	return (count - left) ? (count - left) : -99;
}

int 
file_write(struct d_inode *inode, struct file *filp, char *buf, int count)
{
	unsigned long pos;
	unsigned long block;
	char *r_buf = (char *)malloc(BLOCK_SIZE);
	char *p;
	int i = 0, c;

	if (filp->f_flags == OAPPEND) {
		pos = inode->i_size;
	} else {
		pos = filp->f_pos;
	}

	while (i < count) {
		if (!(block = create_block(inode, pos / BLOCK_SIZE)))
			break;
		if (!(read_data_block(block, r_buf))) 
			break;	
		c = pos % BLOCK_SIZE;
		p = c + r_buf;
		c = BLOCK_SIZE - c;
		if (c > count - i) c = count - i;
		pos += c;
		if (pos > inode->i_size) {
			inode->i_size = pos;
			inode->i_tsize = pos;
		}
		i += c;
		while (c-- > 0)
			*(p++) = get_fs_byte(buf++);
		write_data_block(block, r_buf);
	}
	
	time((time_t *)&inode->i_time);
	if (filp->f_flags != OAPPEND) {
		filp->f_pos = pos;
	}

	set_inode(inode->i_cinode, inode);
	free(r_buf);
	return (i ? i : -1);
}
