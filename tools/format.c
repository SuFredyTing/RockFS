#include "spdk_interface.h"
#include "bitmap.h"

int 
main(int argc, char *argv[1])
{
	int choice, i;
	
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
	spdk_cleanup();
	return 0;
}
