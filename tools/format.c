#include <stdio.h>
#include <time.h>

#include "spdk_interface.h"
#include "inode_block.h"
#include "namei.h"
#include "bitmap.h"

int 
main(int argc, char *argv[1])
{
	int choice, i;
	struct d_inode root;
	struct dir_entry de;	

	choice = atoi(argv[1]);

	spdk_init();
	switch (choice) {
		case 1:
			for ( i =  0; i < INODE_BLOCK_START; i++) {
				clear_block(i);
			}
			break;
		case 2:
			for ( i =  0; i < INODE_BLOCK_BITMAP_START; i++) {
				clear_block(i);
			}
			break;
		case 3:
			for ( i =  INODE_BLOCK_BITMAP_START; i < LOGIC_BLOCK_BITMAP_START; i++) {
				clear_block(i);
			}
			break;
		case 4:
			for ( i =  LOGIC_BLOCK_BITMAP_START; i < INODE_BLOCK_START; i++) {
				clear_block(i);
			}
			break;
		default:
			fprintf(stderr, "parameter error!\n");
			break;
	}	
	set_bitmap(0, INODE_BITMAP, 1);
	set_bitmap(0, LOGIC_BITMAP, 1);
	
	root.i_mode= DIR_INODE;
	root.i_uid= 1;
	root.i_size= 0;
	time((time_t *)&root.i_time);
	root.i_tsize = 0;
	root.i_cinode= 1;
	
	for ( i = 0; i < 10; i++) {
		root.i_zone[i] = 0;
	}

	de.inode = 1;
	strcpy((char *)&de.name, ".");
	add_dir_entry(&root, &de);
	
	set_inode(root.i_cinode, &root);	

	spdk_cleanup();
	return 0;
}
