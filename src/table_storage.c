#define FUSE_USE_VERSION 30

#include <config.h>

#include "common.h"

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <assert.h>

#include "spdk_interface.h"
#include "file_dev.h"
#include "namei.h"
#include "t_time.h"

static struct options {
	const char *filename;
	const char *contents;
	int show_help;
} options;

#define OPTION(t, p)                           \
    { t, offsetof(struct options, p), 1 }
static const struct fuse_opt option_spec[] = {
	OPTION("--name=%s", filename),
	OPTION("--contents=%s", contents),
	OPTION("-h", show_help),
	OPTION("--help", show_help),
	FUSE_OPT_END
};

static struct d_inode inode;
static bool   file_is_exist = false;

static void *
stor_init(struct fuse_conn_info *conn,
			struct fuse_config *cfg)
{
	(void) conn;

	cfg->kernel_cache = 1;
	if (spdk_init() != 0) 
		exit(1);

	return NULL;
}

static void 
stor_destroy(void *value)
{
	spdk_cleanup();
}

static int 
stor_getattr(const char *path, struct stat *stbuf,
			 struct fuse_file_info *fi)
{
	(void) fi;
	int res = 0;
	unsigned long long a, b, c;
	
	a = get_time();
	memset(stbuf, 0, sizeof(struct stat));
	if (open_namei(path, O_RDWR, DIR_INODE, &inode)){
		file_is_exist = false;
    	b = get_time();
	    printf("[stor_getattr] time = %lf\n", (b-a)/2.2);
		return -ENOENT;
	}
	if ( inode.i_mode == DIR_INODE) {
		stbuf->st_mode = S_IFDIR | 0755; 
		stbuf->st_size = inode.i_tsize;
	} else if ( inode.i_mode == COMMON_INODE) {
		stbuf->st_mode = S_IFREG | 0666;
		stbuf->st_size = inode.i_tsize;	
	}
	stbuf->st_nlink = 1;
	b = get_time();
	printf("[stor_getattr] time = %lf\n", (b-a)/2.2);
	file_is_exist = true;
	
	return res;
}

static int 
stor_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
			 off_t offset, struct fuse_file_info *fi,
			 enum fuse_readdir_flags flags)
{
	(void) offset;
	(void) fi;
	(void) flags;
	char (*dir_list)[NAME_LEN] ;
	int size, i;
	unsigned long long a, b, c;
	
	a = get_time();
	if (open_namei(path, O_RDWR, DIR_INODE, &inode))
        return -ENOENT;
	b = get_time();
    size = (inode.i_tsize + DIR_ENTRY_SIZE - 1) / DIR_ENTRY_SIZE;
    dir_list = (char (*)[NAME_LEN])malloc(size * NAME_LEN * sizeof(char));
	c = get_time();
    if (!get_dir_entry_list(&inode, dir_list))
        return -99;
	
	filler(buf, options.filename, NULL, 0, 0);
	
	for (i = 0; i < size; i++){
		filler(buf, dir_list[i], NULL, 0, 0); 
	}
	free(dir_list);
	printf("[stor_readdir] open_namei::time = %lf\nstor_readdir::malloc time = %lf\n[stor_readdir] get_dir_entry_list::time = %lf\n", (b-a)/2.2, (c-b)/2.2, (get_time() -c)/2.2);
	return 0;
}

static int
stor_mkdir(const char *path, mode_t mode)
{
	int res;
	
	res = sys_mkdir(path);
	if (res != 0)
		return -errno;
	
	return 0;
}

static int 
stor_open(const char *path, struct fuse_file_info *fi)
{
	unsigned long long a, b;

	a = get_time();
	if (open_namei(path, O_RDWR, DIR_INODE, &inode) == -ENOENT){
		sys_mknod(path);
	} 
	b = get_time();
	printf("[stor_open] time = %lf\n", (b - a) / 2.2);
	return 0;
}

static int
stor_truncate(const char *path, off_t size, struct fuse_file_info *fi)
{
	unsigned long long a, b;
	
	a = get_time();
	//if (sys_truncate(path, (unsigned long)size))
	//	return -ENOENT;
	inode.i_tsize = inode.i_size = (unsigned long)size;
	set_inode(inode.i_cinode, &inode);
	b = get_time();
	printf("[stor_truncate] time = %lf\n", (b - a) / 2.2);
	return 0;
}

static int 
stor_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
	unsigned long long a, b;
	a = get_time();	
	if (!file_is_exist){
		sys_mknod(path);
		//open_namei(path, O_CREAT, COMMON_INODE, &inode);
	}	
	
	/*
	if (open_namei(path, O_RDWR, DIR_INODE, &inode) == -ENOENT){
        sys_mknod(path);
    }*/
	b = get_time();
	printf("[stor_create()] time = %lf\n", (b - a) / 2.2);
	return 0;
}

static int 
stor_read(const char *path, char *buf, size_t size, off_t offset,
	      struct fuse_file_info *fi)
{
	size_t len;
	(void) fi;
	struct file filp;
	unsigned long long a, b;
	a = get_time();

	filp.f_mode = inode.i_mode;
	filp.f_flags = inode.i_mode;
	filp.f_inode = &inode;
	filp.f_pos = (unsigned long) offset;
	b = get_time();
	if (inode.i_uid != 99) {
		len = file_read(&inode, &filp, buf, size);	
	} else {
		len = strlen(options.contents);
		if (offset < (off_t)len) {
			if (offset + size > len)
				size = len - offset;
			memcpy(buf, options.contents + offset, size);
		} else
			size = 0;
	}

	printf("[stor_read()] open_namei::time = %lf\n[stor_read()] file_read::time = %lf\n", (b - a) / 2.2, (get_time() - b) / 2.2);
	return len;
}

static int
stor_write(const char *path, const char *buf, size_t size,
			off_t offset, struct fuse_file_info *fi)
{
	size_t len;
	struct file filp;
	unsigned long long a, b;	
	
	a = get_time();
	b = get_time();
	filp.f_mode = inode.i_mode;
	filp.f_flags = inode.i_mode;
	filp.f_inode = &inode;
	filp.f_pos = (unsigned long) offset;
	
	len = file_write(&inode, &filp, (char *)buf, size);
	printf("[stor_write()] open_namei::time = %lf\n[stor_write()] file_write::time = %lf\n", (b - a) / 2.2, (get_time() - b) / 2.2);
	return len;
}

static int
stor_fsync(const char *path, int isdatasync, struct fuse_file_info *fi)
{
	return 0;
}

static struct fuse_operations stor_oper = {
	.init		= stor_init,
	.destroy	= stor_destroy,
	.getattr	= stor_getattr,
	.readdir	= stor_readdir,
	.mkdir		= stor_mkdir,
	.open		= stor_open,
	.truncate	= stor_truncate,
	.create		= stor_create,
	.read		= stor_read,
	.write		= stor_write,
	.fsync		= stor_fsync,
};

static void 
show_help(const char *progname)
{
	printf("usage: %s [options] <mountpoint>\n\n", progname);
	printf("File-system specific options:\n"
	       "    --name=<s>          Name of the \"stor\" file\n"
	       "                        (default: \"stor\")\n"
	       "    --contents=<s>      Contents \"stor\" file\n"
	       "                        (default \"Hello, World!\\n\")\n"
	       "\n");
}


int 
main(int argc, char *argv[])
{
	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

	options.filename = strdup("README.txt");
	options.contents = strdup("Author:ystu\n");

	if (fuse_opt_parse(&args, &options, option_spec, NULL) == -1)
		return 1;

	if (options.show_help) {
		show_help(argv[0]);
		assert(fuse_opt_add_arg(&args, "--help") == 0);
		args.argv[0] = (char*) "";
	}

	return fuse_main(args.argc, args.argv, &stor_oper, NULL);
}
