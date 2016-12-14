#ifndef FILE_DEV_H
#define FILE_DEV_H

struct file {
	unsigned int 	f_mode;
	unsigned int	f_flags;
	unsigned int 	f_count;
	struct d_inode *f_inode;
	unsigned long	f_pos;
};

bool get_data_block_list(struct d_inode *data, unsigned long *data_block, int size);
bool add_data_block_to_list(struct d_inode *data, unsigned long data_num, int size);
unsigned long bmap(struct d_inode *data, int data_block_num);
unsigned long create_block(struct d_inode *data, int data_block_num);
int file_read(struct d_inode *inode, struct file *filp, char *buf, int count);
int file_write(struct d_inode *inode, struct file *filp, char *buf, int count);

#endif //FILE_DEV_H
