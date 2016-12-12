#ifndef DATA_BLOCK_H
#define DATA_BLOCK_H

bool get_data_location(unsigned long data_num, unsigned long *block_num);
bool read_dir_block(unsigned long data_num, struct dir_entry *buf);
bool write_dir_block(unsigned long data_num, struct dir_entry *buf);
bool read_data_block(unsigned long data_num, char *buf);
bool write_data_block(unsigned long data_num, char *buf);
bool del_data_block(unsigned long data_num);
bool find_null_data_block_num(unsigned long *data_num);

#endif //DATA_BLOCK_H
