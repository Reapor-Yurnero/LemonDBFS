#include "lemondb.h"

int main() {
  return execute_query("./dbdir/t1.tbl", "UPDATE ( cF 15 ) FROM t1 WHERE ( c2 > 0 ) ( KEY = r10 );", "t1", "./dbdir/t1_dump.tbl");
}
