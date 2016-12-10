#include <stdlib.h>

#include "spdk_interface.h"
#include "super_block.h"
#include "common.h"

struct nvme_super_block {
	unsigned long s_ninodes;		 // 节点数。
	unsigned long s_nzones; 		 // 逻辑块数。
	unsigned long s_imap_blocks;	 // i 节点位图所占用的数据块数。
	unsigned long s_zmap_blocks;	 // 逻辑块位图所占用的数据块数。
	unsigned long s_firstdatazone;   // 第一个数据逻辑块。
	unsigned long s_log_zone_size;   // log(数据块数/逻辑块)。（以2 为底）。
	unsigned long s_max_size;		 // 文件最大长度。
	unsigned long s_magic;			 // 文件系统魔数。
};

//struct nvme_super_block super_block;

void
read_super_block(struct nvme_super_block *s_block)
{
	char *buf = (char *)malloc(BLOCK_SIZE);
	unsigned long *tmp;

	//int i;
	
	spdk_read_and_write(buf, SUPER_BLOCK_NO, 1,	1);

	tmp = (unsigned long *)buf;
	
	s_block->s_ninodes		= tmp[0];
	s_block->s_nzones		= tmp[1];
	s_block->s_imap_blocks	= tmp[2];
	s_block->s_zmap_blocks	= tmp[3];
	s_block->s_firstdatazone = tmp[4];
	s_block->s_log_zone_size = tmp[5];
	s_block->s_max_size		= tmp[6];
	s_block->s_magic			= tmp[7];

	/*printf("read word:\n");
    for (i = 0; i < BLOCK_SIZE; i++) {
        printf("%02X ", buf[i]);
    }
    printf("\nend!\n");*/

	free(buf);
}	

void
write_super_block(struct nvme_super_block *s_block)
{
	unsigned long *tmp = (unsigned long *)malloc(BLOCK_SIZE);
	char *buf;

	//int i;

	tmp[0] = s_block->s_ninodes;		
	tmp[1] = s_block->s_nzones;
	tmp[2] = s_block->s_imap_blocks;
	tmp[3] = s_block->s_zmap_blocks;
	tmp[4] = s_block->s_firstdatazone;
	tmp[5] = s_block->s_log_zone_size;
	tmp[6] = s_block->s_max_size;
	tmp[7] = s_block->s_magic;
	
	buf = (char *)tmp;

	/*printf("write word:\n");
    for (i = 0; i < BLOCK_SIZE; i++) {
        printf("%02X ", buf[i]);
    }
    printf("\nend!\n");*/

	spdk_read_and_write(buf, SUPER_BLOCK_NO, 1, 2);

	free(tmp);
}

/*int
main()
{
	struct nvme_super_block super_block;

	super_block.s_ninodes		= 1;
	super_block.s_nzones		= 2;
	super_block.s_imap_blocks	= 3;
	super_block.s_zmap_blocks	= 4;
	super_block.s_firstdatazone = 5;
	super_block.s_log_zone_size = 6;
	super_block.s_max_size		= 7;
	super_block.s_magic			= 8;

	spdk_init();

	write_super_block(&super_block);
	
	read_super_block(&super_block);

	spdk_cleanup();
	
	return 0;
}*/
