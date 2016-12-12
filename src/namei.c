#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "common.h"
#include "inode_block.h"
#include "data_block.h"
#include "namei.h"
#include "spdk_interface.h"

static bool
get_dir_block_list(struct d_inode *dir, unsigned long *dir_block, int size)
{
	int i, j, k, z, r;
	unsigned long *buf	= (unsigned long *)malloc(BLOCK_SIZE);
	unsigned long *buf1 = (unsigned long *)malloc(BLOCK_SIZE);

	if (size > 0 && size < 9) {
		
		for (i = 0; i < size; i++) {
			dir_block[i] = dir->i_zone[i];	
		}
	
	} else if (size < 521) {

		for (i = 0; i < 8; i++) {
			dir_block[i] = dir->i_zone[i];
		}
		read_data_block(dir->i_zone[i], (char *)buf);
		for (j = 0; j + 8 < size; j++, i++) {
			dir_block[i] = buf[j];	
		}

	} else if (size < 262665) {
		
		for (i = 0; i < 8; i++) {
        	dir_block[i] = dir->i_zone[i];
        }
        
		read_data_block(dir->i_zone[i], (char *)buf);
	    for (j = 0; j + 8 < 520; j++, i++) {
		    dir_block[i] = buf[j];
		}
		
		read_data_block(dir->i_zone[9], (char *)buf);
		z = (size - 520) / 512;
		r = (size - 520) % 512;
		for (j = 0; j < z; j++) {
			read_data_block(buf[j], (char *)buf1);
			for (k = 0; k < 512; k++, i++) {
				dir_block[i] = buf1[k];
			}
		}
		if (r != 0) {
			read_data_block(buf[j], (char *)buf1);
			for (k = 0; k < r; k++, i++) {
				dir_block[i] = buf1[k];
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
add_dir_block_to_list(struct d_inode *dir, unsigned long data_num, int size)
{
	int z, r;
    unsigned long *buf  = (unsigned long *)malloc(BLOCK_SIZE);
    unsigned long *buf1 = (unsigned long *)malloc(BLOCK_SIZE);	

	if (size < 8) {
		dir->i_zone[size] = data_num;
	} else if (size < 520) {
        read_data_block(dir->i_zone[8], (char *)buf);
		buf[size] = data_num;
		write_data_block(dir->i_zone[8], (char *)buf);
	} else if (size < 262664) {
		read_data_block(dir->i_zone[9], (char *)buf);
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

bool
find_dir_entry(struct d_inode *dir, const char *name, int namelen, struct dir_entry *dir_item)
{
	struct dir_entry *buf = (struct dir_entry *)malloc(BLOCK_SIZE);
	unsigned long *dir_block;
	int entry_num, block_num, entry_size_in_block;
	int i, j;

	if (namelen > NAME_LEN || namelen <= 0) {
		fprintf(stderr, "The length of filename exceeds the limit(16)!\n");
		free(buf);
		return false;
	}

	//round up to an integer
	entry_num = (dir->i_size + DIR_ENTRY_SIZE - 1) / DIR_ENTRY_SIZE;
	block_num = (entry_num + DIR_ENTRAY_NUM_IN_BLOCK - 1) / DIR_ENTRAY_NUM_IN_BLOCK;
	entry_size_in_block = entry_num % DIR_ENTRAY_NUM_IN_BLOCK;

	dir_block = (unsigned long *)malloc(sizeof(unsigned long) * block_num);	

	if ( !get_dir_block_list(dir, dir_block, block_num) ) {
		free(dir_block);
		free(buf);
		return false;
	}
	
	for (i = 0; i < block_num - 1; i++) {
		read_dir_block(dir_block[i], buf);
		for (j = 0; j < DIR_ENTRY_SIZE; j++) {
			if ( 0 == strcmp(buf[j].name, name) ) {
				memcpy(dir_item, &buf[j], DIR_ENTRY_SIZE);
				free(dir_block);
				free(buf);
				return true;
			}
		}	
	}
	
	read_dir_block(dir_block[i], buf);
	for (j = 0; j < entry_size_in_block; j++) {
		if ( 0 == strcmp(buf[j].name, name) ) {
	        memcpy(dir_item, &buf[j], DIR_ENTRY_SIZE);
    	    free(dir_block);
            free(buf);
            return true;
        }
	}
	
	fprintf(stderr," Not find %s !\n", name);
	free(dir_block);
	free(buf);

	return false;
}

bool 
add_dir_entry(struct d_inode *dir, struct dir_entry *dir_item)
{
	struct dir_entry *buf = (struct dir_entry *)malloc(BLOCK_SIZE);
	unsigned long *dir_block;
	unsigned long data_num;
	int entry_num, block_num, entry_size_in_block;

	//round up to an integer
	entry_num = (dir->i_size + DIR_ENTRY_SIZE - 1) / DIR_ENTRY_SIZE;
	block_num = (entry_num + DIR_ENTRAY_NUM_IN_BLOCK - 1) / DIR_ENTRAY_NUM_IN_BLOCK;
	entry_size_in_block = entry_num % DIR_ENTRAY_NUM_IN_BLOCK;
	
	if (entry_size_in_block == 0)
		block_num++;
	dir_block = (unsigned long *)malloc(sizeof(unsigned long) * block_num);
	//block_num--;

    if ( !get_dir_block_list(dir, dir_block, block_num) ) {
        free(dir_block);
		free(buf);
        return false;
    }

	if (entry_size_in_block != 0) {
		read_dir_block(dir_block[block_num - 1], buf);
		/*(if (!find_null_data_block_num(&data_num)) {
			free(dir_block);
			free(buf);
			return false;
		}*/
		memcpy(&buf[entry_size_in_block], dir_item, DIR_ENTRY_SIZE);
		write_dir_block(dir_block[block_num - 1], buf);
	} else {
		if (!find_null_data_block_num(&data_num)) {
           	free(dir_block);
			free(buf);
            return false;
        }
		memcpy(&buf[0], dir_item, DIR_ENTRY_SIZE);
		if (!add_dir_block_to_list(dir, data_num, block_num - 1)) {
			free(dir_block);
			free(buf);
			return false;
		}
		write_dir_block(data_num, buf);
	}
	
	dir->i_size += DIR_ENTRY_SIZE;	

	free(dir_block);
	free(buf);
	return true;
}

/*
void
get_dir()
{

}

void
dir_namei()
{

}

void
namei()
{

}

void
open_namei()
{

}

void
sys_mknod()
{

}

void
sys_mkdir()
{

}

void
empty_dir()
{

}

void
sys_rmdir()
{

}
*/

/*
int 
main(int argc, char *argv[])
{
	struct d_inode dir;
	struct dir_entry dir_item;
	
	//get_inode(ROOT_INFO, &dir);
	find_null_data_block_num(&dir_item.inode);
	strcpy(dir_item.name, "hello_world");
	spdk_init();

	get_inode(ROOT_INFO, &dir);
	printf("dir_item.inode = %lu\n", dir_item.inode);
	//printf("dir->zone[0]   = %lu\n", dir.i_zone[0]);    

	//clear_block(4096);
	//set_bitmap(0, LOGIC_BITMAP, 1);
	//clear_block(65537);	
	//find_dir_entry(&dir, ".", 1, &dir_item);
	add_dir_entry(&dir, &dir_item);
	printf("dir->zone[0]   = %lu\n", dir.i_zone[0]);
	printf("dir_item.inode = %lu\n", dir_item.inode);
	printf("dir_item.name  = %s\n", dir_item.name);
	set_inode(ROOT_INFO, &dir);	

	spdk_cleanup();
	return 0;
}*/

