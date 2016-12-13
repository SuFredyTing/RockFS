#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#include "common.h"
#include "inode_block.h"
#include "data_block.h"
#include "namei.h"
#include "spdk_interface.h"

static inline unsigned char 
get_fs_byte(const char *addr)
{
	return *addr;
}

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

static bool
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

bool
rm_dir_entry(struct d_inode *dir, const char *name, int namelen, struct dir_entry *dir_item)
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
                buf[j].inode = 0;
                write_dir_block(dir_block[i], buf);
				free(dir_block);
                free(buf);
                return true;
            }
        }
    }

    read_dir_block(dir_block[i], buf);
    for (j = 0; j < entry_size_in_block; j++) {
        if ( 0 == strcmp(buf[j].name, name) ) {
            buf[j].inode = 0;
			write_dir_block(dir_block[i], buf);
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
get_dir(struct d_inode *dir, const char *pathname, struct d_inode *inode)
{	
	char c;
	const char *thisname;
	int namelen;
	struct dir_entry de;
	
	if ((c = get_fs_byte(pathname) != '/')) {
		fprintf(stderr, "The pathname error!\n");
		return false;
	}
	
	pathname++;
	memcpy(inode, dir, INODE_SIZE);
	while (1) {
		thisname = pathname;
		for (namelen = 0; (c = get_fs_byte(pathname++)) && (c != '/'); namelen++)
			/* nothing, not error! */;
		if (c == '\0') {
			return true;
		}
		//if ( namelen !=0 ) {
		if (!find_dir_entry(inode, thisname, namelen, &de)) {
			fprintf(stderr, "Not find %s!", thisname);
			return false;
		}	
			//printf("inode = %lu\n", de.inode);
			//printf("name  = %s\n", de.name);
		get_inode(de.inode, inode);
		//}	
	}	
}

bool
dir_namei(const char *pathname, int *namelen, const char *name, struct d_inode *base
		, struct d_inode *dir)
{
	char c;
	//const char *basename;
	
	if (!(get_dir(base, pathname, dir))) {
		return false;
	}

	name = pathname;
	while ((c = get_fs_byte(pathname++)))
		if (c == '/')
			name = pathname;
	*namelen = pathname - name - 1;

	return true;
}
/*
void
namei()
{

}*/

int
new_inode(unsigned long *inode_num, int mode, unsigned long parent_inode_num)
{
	struct d_inode t_inode;
	struct dir_entry de;
	int i;
	
	if (!find_null_inode_num(inode_num)) {
		return -ENOSPC;
	}
	
	t_inode.i_mode	 = mode;
	t_inode.i_uid	 = 1;
	t_inode.i_size	 = 0;
	time((time_t *)&t_inode.i_time);
	t_inode.i_gid	 = 1;
	t_inode.i_cinode = *inode_num;
	
	for (i = 0; i < 10; i++) {
        t_inode.i_zone[i] = 0;
    }
	
	if (mode == DIR_INODE) {
		de.inode = *inode_num;
		strcpy((char *)&de.name, ".");
		add_dir_entry(&t_inode,&de);
		de.inode = parent_inode_num;
		strcpy((char *)&de.name, "..");
		add_dir_entry(&t_inode,&de);
	}	

	set_inode(t_inode.i_cinode, &t_inode);
	
	return 0;
}

int
open_namei(const char *pathname, int flag, int mode, struct d_inode *res_inode)
{
	struct d_inode dir;
	struct dir_entry de;	
	const char *basename = NULL;
	int namelen, error;

	get_inode(ROOT_INFO, &dir);
	if (!dir_namei(pathname, &namelen, basename, &dir, res_inode)) {
		return -ENOENT;
	}
	
	if (flag == O_CREAT ) {
		if ((error = new_inode(&de.inode, mode, res_inode->i_cinode))) {
			return error;
		}		
		memcpy(&de.name, basename, NAME_LEN);
		add_dir_entry(res_inode, &de);
		set_inode(res_inode->i_cinode, res_inode);	
	} else {
		if (!find_dir_entry(res_inode, basename, namelen, &de))
			return -ENOENT;
		get_inode(de.inode, res_inode);
	}
	
	return 0;
}

int
sys_mknod(const char *filename)
{
	struct d_inode dir;

	return open_namei(filename, O_CREAT, COMMON_INODE, &dir);
}

int
sys_mkdir(const char *filename)
{
	struct d_inode dir;
	
	return open_namei(filename, O_CREAT, DIR_INODE, &dir);
}

/*int
empty_dir()
{
	return 0;
}*/

int
sys_rmdir(const char *filename)
{
	struct d_inode dir, res_inode;
    struct dir_entry de;
    const char *basename = NULL;
    int namelen;
	
	get_inode(ROOT_INFO, &dir);
    
	if (!dir_namei(filename, &namelen, basename, &dir, &res_inode)) {
        return -ENOENT;
    }

	memcpy(&de.name, basename, namelen);
	if (!rm_dir_entry(&res_inode, basename, namelen, &de))
		return -ENOENT;
	
	set_inode(res_inode.i_cinode, &res_inode);	

	return 0;	
}


/*
int 
main(int argc, char *argv[])
{
	struct d_inode dir;
	struct d_inode inode;
	//struct dir_entry dir_item;
	
	spdk_init();
	//get_inode(ROOT_INFO, &dir);
	//find_null_inode_num(&dir_item.inode);
	//strcpy(dir_item.name, "hello1");
	dir.i_size = 100;
	set_inode(2, &dir);

	get_inode(ROOT_INFO, &dir);
	//printf("dir_item.inode = %lu\n", dir_item.inode);
	//printf("dir->zone[0]   = %lu\n", dir.i_zone[0]);    

	//clear_block(4096);
	//set_bitmap(0, LOGIC_BITMAP, 1);
	//clear_block(65537);	
	//find_dir_entry(&dir, ".", 1, &dir_item);
	//add_dir_entry(&dir, &dir_item);
	//printf("dir->zone[0]   = %lu\n", dir.i_zone[0]);
	get_dir(&dir, "/hello", &inode);
	//printf("dir_item.inode = %lu\n", dir_item.inode);
	//printf("dir_item.name  = %s\n", dir_item.name);
	printf("inode.i_size = %lu\n", inode.i_size);
	//set_inode(ROOT_INFO, &dir);	

	spdk_cleanup();
	return 0;
}
*/
