/*
 * Author: Mr. Frown
 * Creation date: 2017.12.23 
 *
 */	 

#define FUSE_USE_VERSION 26

#define TABLE_NUM_MAX 10
#define TABLE_NAME_MAX 32

//#define DEBUG

#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <dirent.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <limits.h>
#include <fuse.h>
#include <stdbool.h>

#include "lemondb_wrapper.h"

struct lemondb_state {
  int num_tbl;
  char* table_name[TABLE_NUM_MAX];
  char* db_dir;
  bool M_bit[TABLE_NUM_MAX];
};

#define LM_DATA ((struct lemondb_state *) fuse_get_context()->private_data)

static void db_fullpath(char fpath[PATH_MAX], const char *path)
{
  strcpy(fpath, LM_DATA->db_dir);
  strncat(fpath, path, PATH_MAX); // ridiculously long paths will
  // break here
#ifdef DEBUG
  fprintf(stderr, "    bb_fullpath:  rootdir = \"%s\", path = \"%s\", fpath = \"%s\"\n",
          LM_DATA->db_dir, path, fpath);
#endif
}

// FUSE function implementations.
static void *lemonDBfs_init(struct fuse_conn_info *conn)
{
  printf("init!\n");
  fuse_get_context();
  return LM_DATA;
}

static int lemonDBfs_getattr(const char *path, struct stat *stbuf)
{
  printf("getattr(\"%s\"\n", path);
  char fpath[PATH_MAX] = {0};
	int res = 0;

	memset(stbuf, 0, sizeof(struct stat));
	if (strcmp(path, "/") == 0) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
    return res;
	}
  char tmp_path[PATH_MAX] = {0};
  for (int i = 0;i < LM_DATA->num_tbl;++i) {
    strcpy(tmp_path, "/");
    strcat(tmp_path, LM_DATA->table_name[i]); // tmp_path := /mtable0 (directory)
    db_fullpath(fpath, tmp_path); // fpath := *db_dir*/mtable0
    strcat(fpath, ".tbl"); // fpath := *db_dir*/mtable0.tbl
    if (strcmp(path, tmp_path) == 0) {
      stbuf->st_mode = S_IFDIR | 0755;
      stbuf->st_nlink = 2;
      return res;
    }

    strcat(tmp_path, "/orig.tbl"); // tmp_path := /mtable0/orig.tbl (file)
    if (strcmp(path, tmp_path) == 0) {
      res = lstat(fpath,stbuf);
      return res;
    }

    strcpy(tmp_path, "/");
    strcat(tmp_path, LM_DATA->table_name[i]); // tmp_path := /mtable0 (directory)
    strcat(tmp_path, "/.query"); // tmp_path := /mtable0/.query (file)
    if (strcmp(path, tmp_path) == 0) {
      stbuf->st_mode = S_IFREG | 0444;
      stbuf->st_nlink = 1;
      stbuf->st_size = 0;
      return res;
    }

    if (LM_DATA->M_bit[i]) {
      strcpy(tmp_path, "/");
      strcat(tmp_path, LM_DATA->table_name[i]); // tmp_path := /mtable0 (directory)
      db_fullpath(fpath, tmp_path); // fpath := *db_dir*/mtable0
      strcat(fpath, "_dump.tbl"); // fpath := *db_dir*/mtable0_dump.tbl

      strcat(tmp_path, "/res.tbl"); // tmp_path := /mtable0/res.tbl (file)
      if (strcmp(path, tmp_path) == 0) {
        res = lstat(fpath,stbuf);
        return res;
      }
    }
  }
  fprintf(stderr, "no match path for getattr(%s)", path);
  res = -1;
	return res;
}

static int lemonDBfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
  printf("readdir(\"%s\"\n", path);
  (void) offset;
  (void) fi;

  // root
  if (strcmp(path, "/") == 0) {
    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);
    for (int i = 0;i < LM_DATA->num_tbl;++i)
      filler(buf, LM_DATA->table_name[i], NULL, 0);
    return 0;
  }

  // dir for each table
  char tmp_path[PATH_MAX] = {0};
  for (int i = 0;i < LM_DATA->num_tbl;++i) {
    strcpy(tmp_path, "/");
    strcat(tmp_path, LM_DATA->table_name[i]); // tmp_path := /mtable0 (directory)
    if (strcmp(path, tmp_path) == 0) {
      filler(buf, ".", NULL, 0);
      filler(buf, "..", NULL, 0);
      filler(buf, "orig.tbl", NULL, 0);
      filler(buf, ".query", NULL, 0);
      if (LM_DATA->M_bit[i])
        filler(buf, "res.tbl", NULL, 0);
      return 0;
    }
  }

  fprintf(stderr, "no match path for readdir(%s)", path);
  return -1;
}

static int lemonDBfs_open(const char *path, struct fuse_file_info *fi)
{
  printf("open(\"%s\"\n", path);
  int res = 0;
  int fd;
  char fpath[PATH_MAX];

  char tmp_path[PATH_MAX] = {0};
  for (int i = 0;i < LM_DATA->num_tbl;++i) {
    strcpy(tmp_path, "/");
    strcat(tmp_path, LM_DATA->table_name[i]); // tmp_path := /mtable0 (directory)
    db_fullpath(fpath, tmp_path); // fpath := *db_dir*/mtable0
    strcat(fpath, ".tbl"); // fpath := *db_dir*/mtable0.tbl

    strcat(tmp_path, "/orig.tbl"); // tmp_path := /mtable0/orig.tbl (file)
    if (strcmp(path, tmp_path) == 0) {
      fd = open(fpath, fi->flags);
      if (fd < 0) return -1;
      fi->fh = fd;
      return res;
    }

    if (LM_DATA->M_bit[i]) {
      strcpy(tmp_path, "/");
      strcat(tmp_path, LM_DATA->table_name[i]); // tmp_path := /mtable0 (directory)
      db_fullpath(fpath, tmp_path); // fpath := *db_dir*/mtable0
      strcat(fpath, "_dump.tbl"); // fpath := *db_dir*/mtable0_dump.tbl

      strcat(tmp_path, "/res.tbl"); // tmp_path := /mtable0/res.tbl (file)
      if (strcmp(path, tmp_path) == 0) {
        fd = open(fpath, fi->flags);
        if (fd < 0) return -1;
        fi->fh = fd;
        return res;
      }
    }

    strcpy(tmp_path, "/");
    strcat(tmp_path, LM_DATA->table_name[i]); // tmp_path := /mtable0 (directory)
    strcat(tmp_path, "/.query"); // tmp_path := /mtable0/.query (file)
    if (strcmp(path, tmp_path) == 0) {
      return res;
    }
  }

  fprintf(stderr, "no match path for open(%s)", path);
  return -1;
}

static int lemonDBfs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
  printf("read(\"%s\"\n", path);
  (void) fi;
  int res = 0;
  char fpath[PATH_MAX];

  char tmp_path[PATH_MAX] = {0};
  for (int i = 0;i < LM_DATA->num_tbl;++i) {
    strcpy(tmp_path, "/");
    strcat(tmp_path, LM_DATA->table_name[i]); // tmp_path := /mtable0 (directory)
    db_fullpath(fpath, tmp_path); // fpath := *db_dir*/mtable0
    strcat(fpath, ".tbl"); // fpath := *db_dir*/mtable0.tbl

    strcat(tmp_path, "/orig.tbl"); // tmp_path := /mtable0/orig.tbl (file)
    if (strcmp(path, tmp_path) == 0) {
      res = pread(fi->fh, buf, size, offset);
      return res;
    }

    if (LM_DATA->M_bit[i]) {
      strcpy(tmp_path, "/");
      strcat(tmp_path, LM_DATA->table_name[i]); // tmp_path := /mtable0 (directory)
      db_fullpath(fpath, tmp_path); // fpath := *db_dir*/mtable0
      strcat(fpath, "_dump.tbl"); // fpath := *db_dir*/mtable0_dump.tbl

      strcat(tmp_path, "/res.tbl"); // tmp_path := /mtable0/res.tbl (file)
      if (strcmp(path, tmp_path) == 0) {
        res = pread(fi->fh, buf, size, offset);
        return res;
      }
    }
  }
  fprintf(stderr, "no contents to be read!\n");
  return res;
}

static int lemonDBfs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
  printf("write(\"%s\"\n", path);
  (void) fi;
  //int res = 0;
  char fpath[PATH_MAX];
  char dpath[PATH_MAX];

  char tmp_path[PATH_MAX] = {0};
  for (int i = 0;i < LM_DATA->num_tbl;++i) {
    strcpy(tmp_path, "/");
    strcat(tmp_path, LM_DATA->table_name[i]); // tmp_path := /mtable0 (directory)
    db_fullpath(fpath, tmp_path); // fpath := *db_dir*/mtable0
    strcat(fpath, ".tbl"); // fpath := *db_dir*/mtable0.tbl
    db_fullpath(dpath, tmp_path); // dpath := *db_dir*/mtable0
    strcat(dpath, "_dump.tbl"); // dpath := *db_dir*/mtable0_dump.tbl

    strcat(tmp_path, "/.query"); // tmp_path := /mtable0/.query (file)
    if (strcmp(path, tmp_path) == 0) {
      char query[PATH_MAX] = {0};
      strcpy(query, buf);
      printf("write query: %s", query);
      printf("fpath: %s\ntable_name: %s\ndpath:%s\n",fpath, LM_DATA->table_name[i], dpath);
      lmdbfs_query_execute(fpath, query, LM_DATA->table_name[i], dpath);
      LM_DATA->M_bit[i] = true;
      return size;
    }
  }
  fprintf(stderr, "Not allowed to write on %s!\n", path);
  return size;
}

static int lemonDBfs_flush(const char *path, struct fuse_file_info *fi)
{
  printf("flush(\"%s\"\n", path);
  return 0;
}

static int lemonDBfs_truncate(const char *path, off_t newsize)
{
  printf("truncate(\"%s\"\n", path);
  return 0;
}

static void lemonDBfs_destroy(void *userdata)
{
  printf("lemonDBfs destroy!\n");
  char dpath[PATH_MAX];
  for (int i = 0;i < LM_DATA->num_tbl;++i) {
    if (LM_DATA->M_bit[i]) {
      char tmp_path[PATH_MAX] = {0};
      strcpy(tmp_path, "/");
      strcat(tmp_path, LM_DATA->table_name[i]); // tmp_path := /mtable0 (directory)
      db_fullpath(dpath, tmp_path); // dpath := *db_dir*/mtable0
      strcat(dpath, "_dump.tbl"); // dpath := *db_dir*/mtable0_dump.tbl

      if (access(dpath, F_OK) != -1) remove(dpath);
    }
  }
}

static struct fuse_operations lemonDBfs_oper = {
                                                .init = lemonDBfs_init,
                                                .getattr = lemonDBfs_getattr,
                                                .readdir = lemonDBfs_readdir,
                                                .open = lemonDBfs_open,
                                                .read = lemonDBfs_read,
                                                .write = lemonDBfs_write,
                                                .flush = lemonDBfs_flush,
                                                .truncate = lemonDBfs_truncate,
                                                .destroy = lemonDBfs_destroy,
};

void lm_usage()
{
  fprintf(stderr, "usage:  lemondbfs [FUSE and mount options] dbDir mountPoint\n");
  abort();
}

int main(int argc, char **argv)
{
  int fuse_stat;
  struct lemondb_state * lm_data;

  // prohibit root user from using lemondbfs
  if ((getuid() == 0) || (geteuid() == 0)) {
    fprintf(stderr, "Forbidden: Running lemondbfs as root opens unnacceptable security holes!\n");
    return 1;
  }

  // See which version of fuse we're running
  fprintf(stderr, "Fuse library version %d.%d\n", FUSE_MAJOR_VERSION, FUSE_MINOR_VERSION);

  // Perform some sanity checking on the command line:  make sure
  // there are enough arguments, and that neither of the last two
  // start with a hyphen (this will break if you actually have a
  // rootpoint or mountpoint whose name starts with a hyphen, but so
  // will a zillion other programs)
  if ((argc < 3) || (argv[argc-2][0] == '-') || (argv[argc-1][0] == '-'))
    lm_usage();

  lm_data = malloc(sizeof(struct lemondb_state));
  if (lm_data == NULL) {
    perror("malloc for lm_data");
    abort();
  }

  // Pull the dbdir out of the argument list and save it in my
  // private data
  lm_data->db_dir = realpath(argv[argc-2], NULL);
  argv[argc-2] = argv[argc-1];
  argv[argc-1] = NULL;
  argc--;

  DIR * dp;
  dp = opendir(lm_data->db_dir);
#ifdef DEBUG
  fprintf(stderr, "opendir returned 0x%p\n", dp);
#endif
  if (dp == NULL) return -1;
  struct dirent * de;
  de = readdir(dp);
#ifdef DEBUG
  fprintf(stderr, "readdir returned 0x%p\n", de);
#endif
  if (de == 0) return -1;
  int i = 0;
  do {
#ifdef DEBUG
    fprintf(stderr, "found directory entry: %s\n", de->d_name);
#endif
    if (strcmp(de->d_name,".") == 0 || strcmp(de->d_name, "..") == 0) continue;
    lm_data->table_name[i] = (char *) malloc((strlen(de->d_name)-4+1)*sizeof(char));
    if (memcpy(lm_data->table_name[i],de->d_name,strlen(de->d_name)-4) == NULL) {
      perror("strcpy in main");
      abort();
    }
    lm_data->table_name[i][strlen(de->d_name)-4] = '\0';
    lm_data->M_bit[i] = false;
    ++i;
  } while ((de = readdir(dp)) != NULL);
  lm_data->num_tbl = i;

  for (i = 0;i < lm_data->num_tbl;++i) {
    fprintf(stderr, "file entry %d: %s\n", i, lm_data->table_name[i]);
  }

  fprintf(stderr, "about to call fuse_main\n");
  //getchar();
  fuse_stat = fuse_main(argc, argv, &lemonDBfs_oper, lm_data);
  fprintf(stderr, "fuse_main returned %d\n", fuse_stat);

  return fuse_stat;
}

