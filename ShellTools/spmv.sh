#!/bin/sh
export PATH=/usr/local/cilk/bin:$PATH
echo 3 > /proc/sys/vm/drop_caches
echo "==============================amazon "
for ((i=1 ; i<3 ; i++)) do time ./bin/lrelease/spmv ../../Data/graphdb/amazon/ Matrixso/; done
echo 3 > /proc/sys/vm/drop_caches
echo "=================================dblp "
for ((i=1 ; i<3 ; i++)) do time ./bin/lrelease/spmv ../../Data/graphdb/dblp2011/ Matrixso/; done
echo 3 > /proc/sys/vm/drop_caches
echo "================================enwiki"
for ((i=1 ; i<3 ; i++)) do time ./bin/lrelease/spmv ../../Data/graphdb/enwiki/ Matrixso/; done
echo 3 > /proc/sys/vm/drop_caches
echo "=======================================twitter"
for ((i=1 ; i<3 ; i++)) do time ./bin/lrelease/spmv ../../Data/graphdb/twitter/ Matrixso/; done
echo 3 > /proc/sys/vm/drop_caches
echo "=========================================uk2007"
for ((i=1 ; i<3 ; i++)) do time ./bin/lrelease/spmv ../../Data/graphdb/uk2007/ Matrixso/; done
echo 3 > /proc/sys/vm/drop_caches
echo "============================================ukunion"
for ((i=1 ; i<3 ; i++)) do time ./bin/lrelease/spmv ../../Data/graphdb/ukunion/ Matrixso/; done
echo 3 > /proc/sys/vm/drop_caches
echo "============================================webbase"
for ((i=1 ; i<3 ; i++)) do time ./bin/lrelease/spmv ../../Data/graphdb/webbase2001/ Matrixso/; done
echo 3 > /proc/sys/vm/drop_caches
echo "===========================================yahoo"
for ((i=1 ; i<3 ; i++)) do time ./bin/lrelease/spmv ../../Data/graphdb/yahoolink/ Matrixso/; done
