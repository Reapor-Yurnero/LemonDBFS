#include "lemondb.h"
#include "lemondb_wrapper.h"

extern "C" {
  int lmdbfs_query_execute(char const * table_src, char const * query, char const* table_name, char const* dump_path) {
    return execute_query(table_src, query, table_name, dump_path);
  }
}
