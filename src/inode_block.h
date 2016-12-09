#ifndef INODE_BLOCK_H
#define INODE_BLOCK_H

struct d_inode;
struct dir_entry;

bool get_inode_location(unsigned long inode_num, unsigned long *block_num,
						unsigned long *inter_num);
bool get_inode(unsigned long inode_num, struct d_inode *t_inode);
bool set_inode(unsigned long inode_num, struct d_inode *t_inode);
bool del_inode(unsigned long inode_num);

#endif  //INODE_BLOCK_H
