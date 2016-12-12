#include <stdio.h>

#include "common.h"
#include "bitmap.h"
#include "spdk_interface.h"

int
main(int argc, char *argv[])
{
    char *buf = (char *)malloc(BLOCK_SIZE);
	int i, j, start;
	
	start = atoi(argv[1]);	

	spdk_init();
	
	spdk_read_and_write(buf, start, 1, READ);
	printf("read %d block:\n---------------------------------------------------------------------------------\n", start);
	for (i = 0; i < 409; i++) {
		printf("%03d| ", i+1);
		for (j = 0; j < 10; j++) {
			if(j % 5 == 0)
				printf(" ");
			printf("%02X ", buf[i*10+j]);
		}
		printf("|");
		for (j = 0; j < 10; j++) {
			if (j % 5 == 0)
				printf(" ");
			printf("%c", buf[i*10+j]);
		}
		printf("\n");
	}
	
	printf("410| ");
	for (j = 0; j < 6; j++) {
		if(j % 5 == 0)
			printf(" ");
		printf("%02X ", buf[i*10+j]);
	}
	printf("            |");
	for (j = 0; j< 6; j++) {
		if(j % 5 == 0)
			printf(" ");
		printf("%c", buf[i*10+j]);
	}
	printf("\nend!\n---------------------------------------------------------------------------------\n");
	//}

	spdk_cleanup();
	free(buf);
	
	return 0;
}

