#ifndef INODE_BLOCK_H
#define INODE_BLOCK_H

#include "common.h"

// 磁盘上的索引节点(i 节点)数据结构。
struct d_inode
{
	unsigned long i_mode;       // 文件类型和属性(rwx 位)。
	unsigned long i_uid;        // 用户id（文件拥有者标识符）。
	unsigned long i_size;       // 文件大小（字节数）。
	unsigned long i_time;       // 修改时间（自1970.1.1:0 算起，秒）。
	unsigned long i_gid;        // 组id(文件拥有者所在的组)。
	unsigned long i_cinode;     // 链接数（多少个文件目录项指向该i 节点）。
	unsigned long i_zone[10];   // 直接(0-7)、间接(8)或双重间接(9)逻辑块号。
								// zone 是区的意思，可译成区段，或逻辑块。
};
// 文件目录项结构。
struct dir_entry
{
	unsigned long inode;        // i 节点。
	char name[NAME_LEN];        // 文件名。
};

bool get_inode_location(unsigned long inode_num, unsigned long *block_num,
						unsigned long *inter_num);
bool get_inode(unsigned long inode_num, struct d_inode *t_inode);
bool set_inode(unsigned long inode_num, struct d_inode *t_inode);
bool del_inode(unsigned long inode_num);
bool find_null_inode_num(unsigned long * inode_num);

#endif  //INODE_BLOCK_H
