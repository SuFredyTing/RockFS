#ifndef NAMEI_H
#define NAMEI_H

unsigned char get_fs_byte(const char *addr);
bool get_dir_block_list(struct d_inode *dir, unsigned long *dir_block, int size);
bool add_dir_block_to_list(struct d_inode *dir, unsigned long data_num, int size);
bool find_dir_entry(struct d_inode *dir, const char *name, int namelen, struct dir_entry *dir_entry);
bool add_dir_entry(struct d_inode *dir, struct dir_entry *dir_item);
bool rm_dir_entry(struct d_inode *dir, const char *name, int namelen, struct dir_entry *dir_item);
bool get_dir(struct d_inode *dir, const char *pathname, struct d_inode *inode);
bool dir_namei(const char *pathname, int *namelen, const char **name
			 , struct d_inode *base, struct d_inode *dir);
int new_inode(unsigned long *inode_num, int mode, unsigned long parent_inode_num);
int open_namei(const char *pathname, int flag, int mode, struct d_inode *res_inode);
int sys_mknod(const char *filename);
int sys_mkdir(const char *filename);
int sys_rmdir(const char *filename);

#endif //NAMEI_H
