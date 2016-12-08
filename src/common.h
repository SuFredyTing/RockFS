#ifndef COMMON_H
#define COMMON_H

#define BLOCK_SIZE					4096

#define LOGIC_BLOCK_NUM             4000000000
#define SUPER_BLOCK_NO              0
#define INODE_BLOCK_BITMAP_START    1
#define LOGIC_BLOCK_BITMAP_START    4096
#define INODE_BLOCK_START           65536
#define DATA_BLOCK_START            2097152

enum RW_MODE { READ = 1, WRITE };

#endif  //COMMON_H
