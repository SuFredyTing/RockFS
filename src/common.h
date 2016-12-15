#ifndef COMMON_H
#define COMMON_H

#define BLOCK_SIZE					4096
#define INODE_SIZE					128
#define DIR_ENTRY_SIZE				32

#define INODE_NUM_IN_BLOCK			32
#define DIR_ENTRY_NUM_IN_BLOCK		128

#define NAME_LEN					24
#define ROOT_INFO                   1

#define LOGIC_BLOCK_NUM             4000000000
#define SUPER_BLOCK_NO              0
#define INODE_BLOCK_BITMAP_START    1
#define LOGIC_BLOCK_BITMAP_START    4096
#define INODE_BLOCK_START           65536
#define DATA_BLOCK_START            2097152

enum RW_MODE { READ = 1, WRITE };
enum BLOCK_TYPE { INODE_BITMAP = 10, LOGIC_BITMAP };
enum FILE_INODE_MODE { DIR_INODE = 40, COMMON_INODE };
enum FILE_OPT_FLAG { O_RDONLY = 50, O_WRONLY, O_RDWR, O_CREAT, O_EXCL, OAPPEND};

#endif  //COMMON_H
