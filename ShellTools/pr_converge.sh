#!/bin/sh
export PATH=/usr/local/cilk/bin:$PATH
echo 3 > /proc/sys/vm/drop_caches
echo "==============================amazon "
for ((i=1 ; i<2 ; i++)) do time ./bin/lrelease/pageRank /jpkc/01/Data/newdb/amazon/ Matrixos/ 8; done
echo 3 > /proc/sys/vm/drop_caches
echo "=================================dblp "
for ((i=1 ; i<2 ; i++)) do time ./bin/lrelease/pageRank /jpkc/01/Data/newdb/dblp/ Matrixos/ 8; done
echo 3 > /proc/sys/vm/drop_caches
echo "================================enwiki"
for ((i=1 ; i<2 ; i++)) do time ./bin/lrelease/pageRank /jpkc/01/Data/newdb/enwiki/ Matrixos/ 8; done
echo 3 > /proc/sys/vm/drop_caches
echo "============================================webbase"
for ((i=1 ; i<2 ; i++)) do time  ./bin/lrelease/pageRank /jpkc/01/Data/newdb/webbase/ Matrixos/ 8; done
echo 3 > /proc/sys/vm/drop_caches
