#ifndef SUPER_BLOCK_H
#define SUPER_BLOCK_H

struct nvme_super_block;

void  read_super_block(struct nvme_super_block *s_block);
void write_super_block(struct nvme_super_block *s_block);

#endif //SUPER_BLOCK_H
