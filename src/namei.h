#ifndef NAMEI_H
#define NAMEI_H

static bool get_dir_block_list(struct d_inode *dir, unsigned long *dir_block, int size);
bool find_dir_entry(struct d_inode *dir, const char *name, int namelen, struct dir_entry *dir_entry);
bool add_dir_block_to_list(struct d_inode *dir, unsigned long data_num, int size);
bool add_dir_entry(struct d_inode *dir, struct dir_entry *dir_item);

#endif //NAMEI_H
