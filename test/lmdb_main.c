#include "lemondb_wrapper.h"

int main () {
  return lmdbfs_query_execute("./dbdir/mTable0.tbl", "SWAP ( c16 c9 ) FROM mTable0 WHERE ( c0 <= -20302 ) ;", "mTable0", "./dbdir/mTable0_dump.tbl");
}
