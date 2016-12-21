#include <stdio.h>
#include <string.h>

#include "namei.h"
#include "inode_block.h"
#include "file_dev.h"
#include "t_time.h"
#include "spdk_interface.h"

/*
int 
main(void)
{
	struct d_inode node;
	struct file filp;	
	char *buf = (char *)malloc(5);
	unsigned long long a, b;
	
	memcpy(buf,"1234",5);
	
	spdk_init();
	
	a = get_time();
	open_namei("/121", O_RDWR, DIR_INODE, &node);
	filp.f_mode = node.i_mode;
	filp.f_flags = node.i_mode;
	filp.f_inode = &node;
	filp.f_pos = 0;
	
	file_write(&node, &filp, buf, 5);
	b = get_time();
	printf("write time = %lf\n", (b-a)/2.2);
	
	memset(buf, 0, 5);
	a = get_time();
	file_read(&node, &filp, buf, 5);
	b = get_time();	
	printf("read time = %lf\n", (b-a)/2.2);
		
	printf("buf = %s\n", buf);
	spdk_cleanup();
	free(buf);
	return 0;
}
*/
