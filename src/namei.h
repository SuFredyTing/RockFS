#ifndef NAMEI_H
#define NAMEI_H

static inline unsigned char get_fs_byte(const char *addr);
static bool get_dir_block_list(struct d_inode *dir, unsigned long *dir_block, int size);
static bool add_dir_block_to_list(struct d_inode *dir, unsigned long data_num, int size);
bool find_dir_entry(struct d_inode *dir, const char *name, int namelen, struct dir_entry *dir_entry);
bool add_dir_entry(struct d_inode *dir, struct dir_entry *dir_item);
bool get_dir(struct d_inode *dir, const char *pathname, struct d_inode *inode);

#endif //NAMEI_H
