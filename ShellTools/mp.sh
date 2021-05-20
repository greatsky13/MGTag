#!/bin/sh
export PATH=/usr/local/cilk/bin:$PATH
echo 3 > /proc/sys/vm/drop_caches
echo "==============================amazon "
for ((i=1 ; i<3 ; i++)) do time ./bin/lrelease/matrixProduct ../../Data/graphdb/amazon/ Matrixso/ 2 ; done
echo 3 > /proc/sys/vm/drop_caches
echo "=================================dblp "
for ((i=1 ; i<3 ; i++)) do time ./bin/lrelease/matrixProduct ../../Data/graphdb/dblp2011/ Matrixso/ 2 ; done
echo 3 > /proc/sys/vm/drop_caches
echo "================================enwiki"
for ((i=1 ; i<3 ; i++)) do time ./bin/lrelease/matrixProduct ../../Data/graphdb/enwiki/ Matrixso/ 2 ; done
echo 3 > /proc/sys/vm/drop_caches
echo "============================================webbase"
for ((i=1 ; i<3 ; i++)) do time ./bin/lrelease/matrixProduct ../../Data/graphdb/webbase2001/ Matrixso/ 2 ; done
echo 3 > /proc/sys/vm/drop_caches
echo "=======================================twitter"
for ((i=1 ; i<3 ; i++)) do time ./bin/lrelease/matrixProduct ../../Data/graphdb/twitter/ Matrixso/ 2 ; done
echo 3 > /proc/sys/vm/drop_caches
echo "=========================================uk2007"
for ((i=1 ; i<3 ; i++)) do time ./bin/lrelease/matrixProduct ../../Data/graphdb/uk2007/ Matrixso/ 2 ; done
echo 3 > /proc/sys/vm/drop_caches
echo "============================================ukunion"
for ((i=1 ; i<3 ; i++)) do time ./bin/lrelease/matrixProduct ../../Data/graphdb/ukunion/ Matrixso/ 2 ; done
echo 3 > /proc/sys/vm/drop_caches
echo "===========================================yahoo"
for ((i=1 ; i<3 ; i++)) do time ./bin/lrelease/matrixProduct ../../Data/graphdb/yahoolink1/ Matrixso/ 2 ; done
