#!/bin/sh
export PATH=/usr/local/cilk/bin:$PATH
echo 3 > /proc/sys/vm/drop_caches
echo "==============================amazon "
for ((i=1 ; i<3 ; i++)) do time ./bin/lrelease/spmv /jpkc/01/Data/newdb/amazon/ Matrixso/; done
echo 3 > /proc/sys/vm/drop_caches
echo "=================================dblp "
for ((i=1 ; i<3 ; i++)) do time ./bin/lrelease/spmv /jpkc/01/Data/newdb/dblp/ Matrixso/; done
echo 3 > /proc/sys/vm/drop_caches
echo "================================enwiki"
for ((i=1 ; i<3 ; i++)) do time ./bin/lrelease/spmv /jpkc/01/Data/newdb/enwiki/ Matrixso/; done
echo 3 > /proc/sys/vm/drop_caches
echo "=======================================twitter"
for ((i=1 ; i<3 ; i++)) do time ./bin/lrelease/spmv /jpkc/01/Data/newdb/twitter/ Matrixso/; done
echo 3 > /proc/sys/vm/drop_caches
echo "=========================================uk2007"
for ((i=1 ; i<3 ; i++)) do time ./bin/lrelease/spmv /jpkc/01/Data/newdb/uk2007/ Matrixso/; done
echo 3 > /proc/sys/vm/drop_caches
echo "============================================ukunion"
for ((i=1 ; i<3 ; i++)) do time ./bin/lrelease/spmv /jpkc/01/Data/newdb/ukunion/ Matrixso/; done
echo 3 > /proc/sys/vm/drop_caches
echo "============================================webbase"
for ((i=1 ; i<3 ; i++)) do time ./bin/lrelease/spmv /jpkc/01/Data/newdb/webbase/ Matrixso/; done
echo 3 > /proc/sys/vm/drop_caches
echo "===========================================yahoo"
for ((i=1 ; i<3 ; i++)) do time ./bin/lrelease/spmv /jpkc/01/Data/graphdb/yahoolink1/ Matrixso/; done
echo 3 > /proc/sys/vm/drop_caches
