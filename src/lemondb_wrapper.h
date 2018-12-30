#ifndef LEMONDB_WRAPPER_H
#define LEMONDB_WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif
  int lmdbfs_query_execute(char const * table_src, char const * query, char const* table_name, char const* dump_path);

#ifdef __cplusplus
}
#endif
#endif
