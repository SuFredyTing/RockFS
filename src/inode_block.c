#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "bitmap.h"
#include "spdk_interface.h"
#include "inode_block.h"

//static char buf[BLOCK_SIZE];

bool 
get_inode_location(unsigned long inode_num, unsigned long *block_num, 
				   unsigned long *inter_num)
{
	*block_num = inode_num / INODE_NUM_IN_BLOCK + INODE_BLOCK_START;
	*inter_num = inode_num % INODE_NUM_IN_BLOCK;

	if (*block_num >= DATA_BLOCK_START) {
		fprintf(stderr, "The number of inode overflows!\n");
		return false;
	}

	return true;
}

bool
get_inode(unsigned long inode_num, struct d_inode *t_inode)
{
	unsigned long block_num, inter_num;
	unsigned long *buf = (unsigned long *)malloc(BLOCK_SIZE);
	int location, i;

	//char *tmp;

	if ( !get_inode_location(inode_num, &block_num, &inter_num) ) {
		free(buf);
		return false;
	}

	spdk_read_and_write((char *)buf, block_num, 1, READ);
	
	location = (inter_num * INODE_SIZE) / 8;

	t_inode->i_mode	  = buf[location];
	t_inode->i_uid	  = buf[location + 1];
	t_inode->i_size	  = buf[location + 2];
	t_inode->i_time   = buf[location + 3];
	t_inode->i_tsize  = buf[location + 4];
	t_inode->i_cinode = buf[location + 5];
	
	for (i = 0; i < 10; i++) {
		t_inode->i_zone[i] = buf[location + 6 + i];
	}
	
	/*tmp = (char *)buf;
	printf("read %lu word:\n", block_num);
    for (i = 0; i < BLOCK_SIZE; i++) {
        //buf[i] = 0x00;
        printf("%02X ", tmp[i]);
    }
    printf("\nend!\n");
	*/
	free(buf);
	return true;
}

bool
set_inode(unsigned long inode_num, struct d_inode *t_inode)
{
	unsigned long block_num, inter_num;
	unsigned long *buf = (unsigned long *)malloc(BLOCK_SIZE);
	int location, i;

	//char *tmp;

	if ( !get_inode_location(inode_num, &block_num, &inter_num) ) {
		free(buf);
		return false;
	}
	
	spdk_read_and_write((char *)buf, block_num, 1, READ);	

	location = (inter_num * INODE_SIZE) / 8;
	
	buf[location]	  = t_inode->i_mode;	 	
    buf[location + 1] = t_inode->i_uid;	 
    buf[location + 2] = t_inode->i_size;	 
    buf[location + 3] = t_inode->i_time;  
	buf[location + 4] = t_inode->i_tsize;	 
	buf[location + 5] = t_inode->i_cinode;

	for (i = 0; i < 10; i++) {
		buf[location + 6 + i] = t_inode->i_zone[i];
	}

	spdk_read_and_write((char *)buf, block_num, 1, WRITE);
	set_bitmap(inode_num, INODE_BITMAP, 1);

	/*tmp = (char *)buf;
	printf("write %lu word:\n", block_num);
    for (i = 0; i < BLOCK_SIZE; i++) {
	    //tmp[i] = 0x00;
	    printf("%02X ", tmp[i]);
	}
	printf("\nend!\n");
    */
	free(buf);
	return true;
}

bool 
del_inode(unsigned long inode_num)
{
	set_bitmap(inode_num, INODE_BITMAP, 0);
	return true;
}

bool
find_null_inode_num(unsigned long *inode_num)
{
	if (!find_bitmap_first_zero(INODE_BITMAP, inode_num)) {
		return false;
	}

	return true;
}

/*
int 
main()
{
	struct d_inode t_inode;
	char *buf = (char *)malloc(BLOCK_SIZE);
	int i, j;

	spdk_init();
	
	t_inode.i_mode   = 1;
    t_inode.i_uid    = 2;
    t_inode.i_size   = 3;
    t_inode.i_time   = 4;
    t_inode.i_gid    = 5;
    t_inode.i_cinode = 6;

    for (i = 0; i < 10; i++) {
        t_inode.i_zone[i] = i;

		clear_block(i);
    }

	for (i = INODE_BLOCK_START; i < INODE_BLOCK_START + 10; i++){
		clear_block(i);
	}

	printf("write inode:\n");
	printf("t_inode.i_mode   = %lu\n", t_inode.i_mode  );
	printf("t_inode.i_uid    = %lu\n", t_inode.i_uid   );
	printf("t_inode.i_size   = %lu\n", t_inode.i_size  );
	printf("t_inode.i_time   = %lu\n", t_inode.i_time  );
	printf("t_inode.i_gid    = %lu\n", t_inode.i_gid   );
	printf("t_inode.i_cinode = %lu\n", t_inode.i_cinode);
	for (i = 0; i < 10; i++) {
		printf("t_inode->i_zone[%d]= %lu\n", i, t_inode.i_zone[i]);
	}
	printf("\nend!\n");
	
	set_inode(0, &t_inode);
	set_inode(1, &t_inode);
	get_inode(1, &t_inode);

	printf("write inode:\n");
	printf("t_inode.i_mode   = %lu\n", t_inode.i_mode  );
	printf("t_inode.i_uid    = %lu\n", t_inode.i_uid   );
	printf("t_inode.i_size   = %lu\n", t_inode.i_size  );
	printf("t_inode.i_time   = %lu\n", t_inode.i_time  );
	printf("t_inode.i_gid    = %lu\n", t_inode.i_gid   );
	printf("t_inode.i_cinode = %lu\n", t_inode.i_cinode);
	for (i = 0; i < 10; i++) {
		printf("t_inode.i_zone[%d]= %lu\n", i, t_inode.i_zone[i]);
	}
	printf("\nend!\n");

	for ( i = INODE_BLOCK_START; i < INODE_BLOCK_START + 10; i++){
        spdk_read_and_write(buf, i, 1, READ);
        printf("read %d's block word:\n", i);
        for (j = 0; j < BLOCK_SIZE; j++) {
			printf("%02X ", buf[j]);
        }
        printf("\nend!\n------------------------------------\n");
    }

	spdk_cleanup();
	free(buf);

	return 0;
}*/
