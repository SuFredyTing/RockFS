#include <rte_config.h>
#include <rte_eal.h>

#include "spdk/nvme.h"
#include "spdk/env.h"

#include "common.h"
#include "spdk_interface.h"

struct ctrlr_entry {
	struct spdk_nvme_ctrlr	*ctrlr;
	struct ctrlr_entry	*next;
	char			name[1024];
};

struct ns_entry {
	struct spdk_nvme_ctrlr	*ctrlr;
	struct spdk_nvme_ns	*ns;
	struct ns_entry		*next;
	struct spdk_nvme_qpair	*qpair;
};

static char *ealargs[] = {
	"table_storage",
	"-c 0x1",
	"-n 4",
	"--proc-type=auto",
};

struct storage_sequence {
	struct ns_entry	*ns_entry;
	char		*buf;
	int		is_completed;
};

static struct ctrlr_entry *g_controllers = NULL;
static struct ns_entry *g_namespaces = NULL;

void
register_ns(struct spdk_nvme_ctrlr *ctrlr, struct spdk_nvme_ns *ns)
{
	struct ns_entry *entry;
	const struct spdk_nvme_ctrlr_data *cdata;

	cdata = spdk_nvme_ctrlr_get_data(ctrlr);

	if (!spdk_nvme_ns_is_active(ns)) {
		printf("Controller %-20.20s (%-20.20s): Skipping inactive NS %u\n",
		       cdata->mn, cdata->sn,
		       spdk_nvme_ns_get_id(ns));
		return;
	}

	entry = malloc(sizeof(struct ns_entry));
	if (entry == NULL) {
		perror("ns_entry malloc");
		exit(1);
	}

	entry->ctrlr = ctrlr;
	entry->ns = ns;
	entry->next = g_namespaces;
	g_namespaces = entry;

	printf("  Namespace ID: %d size: %juGB\n", spdk_nvme_ns_get_id(ns),
	       spdk_nvme_ns_get_size(ns) / 1000000000);
}

void
read_complete(void *arg, const struct spdk_nvme_cpl *completion)
{
	struct storage_sequence *sequence = arg;

//	printf("%s", sequence->buf);
	//spdk_free(sequence->buf);
	sequence->is_completed = 1;
}

void
write_complete(void *arg, const struct spdk_nvme_cpl *completion)
{
	struct storage_sequence	*sequence = arg;
/*	struct ns_entry			*ns_entry = sequence->ns_entry;
	int				rc;

	spdk_free(sequence->buf);
	sequence->buf = spdk_zmalloc(0x1000, 0x1000, NULL);
*/
 //   spdk_free(sequence->buf);
	sequence->is_completed = 1;
}

int 
spdk_rw(char *buf, unsigned long start, unsigned long length, int mode)
{
	struct ns_entry         *ns_entry;
	struct storage_sequence sequence;
	int             rc;

	//init();

	ns_entry = g_namespaces;
	while (ns_entry != NULL) {
		
		ns_entry->qpair = spdk_nvme_ctrlr_alloc_io_qpair(ns_entry->ctrlr, 0);
		if (ns_entry->qpair == NULL) {
			printf("ERROR: spdk_nvme_ctrlr_alloc_io_qpair() failed\n");
			return -1;
		}
		
		sequence.is_completed = 0;
		sequence.ns_entry = ns_entry;

		sequence.buf = spdk_zmalloc(length * 512, length * 512, NULL);

		if (mode == WRITE) {
			memcpy(sequence.buf, buf, sizeof(char) * length * 512);
			rc = spdk_nvme_ns_cmd_write(ns_entry->ns, ns_entry->qpair, sequence.buf,
							start, 
							length, 
							write_complete, &sequence, 0);
			if (rc != 0) {
				fprintf(stderr, "starting write I/O failed\n");
				exit(1);
			}
		} else if (mode == READ ) {
			//sequence.buf = spdk_zmalloc(length * 4096, length * 4096, NULL);
			printf("start = %lu\n", start);
			rc = spdk_nvme_ns_cmd_read(ns_entry->ns, ns_entry->qpair, sequence.buf,
						   start,
						   length,
						   read_complete, &sequence, 0);
			if (rc != 0) {
				fprintf(stderr, "starting read I/O failed\n");
				exit(1);
			}
			printf("testing!\n");
//			printf("sequence.buf=%s\n", sequence.buf);
//			memcpy(*buf, sequence.buf, sizeof(char) * 4096);
//			printf("*buf=%s\n",*buf);
		}
        
	    while (!sequence.is_completed) {
        	spdk_nvme_qpair_process_completions(ns_entry->qpair, 0);
		}
		printf("tystesting\n");
		if (mode == READ) {
			//printf("sequence.buf=%s\n", sequence.buf);
			memcpy(buf, sequence.buf, sizeof(char) * length * 512);
			//printf("*buf=%s\n",buf);
		}
    	
		spdk_free(sequence.buf);
		
		spdk_nvme_ctrlr_free_io_qpair(ns_entry->qpair);
		ns_entry = ns_entry->next;
		printf("ffffffffffffffffffffffff\n");
	}

	//cleanup();
	return 0;
}

int
spdk_read_and_write(char *buf, unsigned long start, unsigned long length, int mode)
{
	unsigned long s = start * 8;
	unsigned long l = length * 8;

	printf("s = %lu\nl = %lu\n", s, l);		

	return spdk_rw(buf, s, l, mode);
}

/*
static void
hello_world(void)
{
	struct ns_entry			*ns_entry;
	struct storage_sequence	sequence;
	int				rc;

	ns_entry = g_namespaces;
	while (ns_entry != NULL) {
		
		ns_entry->qpair = spdk_nvme_ctrlr_alloc_io_qpair(ns_entry->ctrlr, 0);
		if (ns_entry->qpair == NULL) {
			printf("ERROR: spdk_nvme_ctrlr_alloc_io_qpair() failed\n");
			return;
		}

		sequence.buf = spdk_zmalloc(0x1000, 0x1000, NULL);
		sequence.is_completed = 0;
		sequence.ns_entry = ns_entry;

		sprintf(sequence.buf, "Hello world!\n");
		
		rc = spdk_nvme_ns_cmd_write(ns_entry->ns, ns_entry->qpair, sequence.buf,
					    0, 
					    1, 
					    write_complete, &sequence, 0);
		if (rc != 0) {
			fprintf(stderr, "starting write I/O failed\n");
			exit(1);
		}

		while (!sequence.is_completed) {
			spdk_nvme_qpair_process_completions(ns_entry->qpair, 0);
		}

		spdk_nvme_ctrlr_free_io_qpair(ns_entry->qpair);
		ns_entry = ns_entry->next;
	}
}
*/

bool
probe_cb(void *cb_ctx, const struct spdk_nvme_probe_info *probe_info,
	 struct spdk_nvme_ctrlr_opts *opts)
{
	printf("Attaching to %04x:%02x:%02x.%02x\n",
	       probe_info->pci_addr.domain,
	       probe_info->pci_addr.bus,
	       probe_info->pci_addr.dev,
	       probe_info->pci_addr.func);

	return true;
}

void
attach_cb(void *cb_ctx, const struct spdk_nvme_probe_info *probe_info,
	  struct spdk_nvme_ctrlr *ctrlr, const struct spdk_nvme_ctrlr_opts *opts)
{
	int nsid, num_ns;
	struct ctrlr_entry *entry;
	const struct spdk_nvme_ctrlr_data *cdata = spdk_nvme_ctrlr_get_data(ctrlr);

	entry = malloc(sizeof(struct ctrlr_entry));
	if (entry == NULL) {
		perror("ctrlr_entry malloc");
		exit(1);
	}

	printf("Attached to %04x:%02x:%02x.%02x\n",
	       probe_info->pci_addr.domain,
	       probe_info->pci_addr.bus,
	       probe_info->pci_addr.dev,
	       probe_info->pci_addr.func);

	snprintf(entry->name, sizeof(entry->name), "%-20.20s (%-20.20s)", cdata->mn, cdata->sn);

	entry->ctrlr = ctrlr;
	entry->next = g_controllers;
	g_controllers = entry;

	num_ns = spdk_nvme_ctrlr_get_num_ns(ctrlr);
	printf("Using controller %s with %d namespaces.\n", entry->name, num_ns);
	for (nsid = 1; nsid <= num_ns; nsid++) {
		register_ns(ctrlr, spdk_nvme_ctrlr_get_ns(ctrlr, nsid));
	}
}

void
spdk_cleanup(void)
{
	struct ns_entry *ns_entry = g_namespaces;
	struct ctrlr_entry *ctrlr_entry = g_controllers;

	while (ns_entry) {
		struct ns_entry *next = ns_entry->next;
		free(ns_entry);
		ns_entry = next;
	}

	while (ctrlr_entry) {
		struct ctrlr_entry *next = ctrlr_entry->next;

		spdk_nvme_detach(ctrlr_entry->ctrlr);
		free(ctrlr_entry);
		ctrlr_entry = next;
	}
}

int 
spdk_init(void)
{
	int rc;

	rc = rte_eal_init(sizeof(ealargs) / sizeof(ealargs[0]), ealargs);
	if (rc < 0) {
		fprintf(stderr, "could not initialize dpdk\n");
		return 1;
	}

	printf("Initializing NVMe Controllers\n");

	rc = spdk_nvme_probe(NULL, probe_cb, attach_cb, NULL);
	if (rc != 0) {
		fprintf(stderr, "spdk_nvme_probe() failed\n");
		spdk_cleanup();
		return 1;
	}

	printf("Initialization complete.\n");
	return 0;
}
