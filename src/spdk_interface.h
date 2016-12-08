#ifndef SPDK_INTERFACE_H
#define SPDK_INTERFACE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <rte_config.h>
#include <rte_eal.h>

#include "spdk/nvme.h"
#include "spdk/env.h"

void register_ns(struct spdk_nvme_ctrlr *ctrlr, struct spdk_nvme_ns *ns);
void read_complete(void *arg, const struct spdk_nvme_cpl *completion);
void write_complete(void *arg, const struct spdk_nvme_cpl *completion);
int  spdk_read_and_write(char *buf, int start, int length, int mode);
bool probe_cb(void *cb_ctx, const struct spdk_nvme_probe_info *probe_info,
       			     struct spdk_nvme_ctrlr_opts *opts);
void attach_cb(void *cb_ctx, const struct spdk_nvme_probe_info *probe_info,
          	    	 struct spdk_nvme_ctrlr *ctrlr, const struct spdk_nvme_ctrlr_opts *opts);
void spdk_cleanup(void);
int  spdk_init(void);

#endif //SPDK_INTERFACE_H
