#!/bin/sh
export PATH=/usr/local/cilk/bin:$PATH
echo 3 > /proc/sys/vm/drop_caches
echo "==============================amazon "
for ((i=1 ; i<3 ; i++)) do time ./bin/lrelease/pageRank /jpkc/01/Data/graphdb/amazon/ Matrixos/ 8; done
echo 3 > /proc/sys/vm/drop_caches
echo "=================================dblp "
for ((i=1 ; i<3 ; i++)) do time ./bin/lrelease/pageRank /jpkc/01/Data/graphdb/dblp2011/ Matrixos/ 8; done
echo 3 > /proc/sys/vm/drop_caches
echo "================================enwiki"
for ((i=1 ; i<3 ; i++)) do time ./bin/lrelease/pageRank /jpkc/01/Data/graphdb/enwiki/ Matrixos/ 8; done
echo 3 > /proc/sys/vm/drop_caches
echo "============================================webbase"
for ((i=1 ; i<3 ; i++)) do time  ./bin/lrelease/pageRank /jpkc/01/Data/graphdb/webbase2001_2/ Matrixos/ 8; done
echo 3 > /proc/sys/vm/drop_caches
echo "=======================================twitter"
for ((i=1 ; i<3 ; i++)) do time ./bin/lrelease/pageRank /jpkc/01/Data/graphdb/twitter/ Matrixos/ 8; done
echo 3 > /proc/sys/vm/drop_caches
echo "=========================================uk2007"
for ((i=1 ; i<3 ; i++)) do time ./bin/lrelease/pageRank /jpkc/01/Data/graphdb/uk2007/ Matrixos/ 8; done
echo 3 > /proc/sys/vm/drop_caches
echo "============================================ukunion"
for ((i=1 ; i<3 ; i++)) do time ./bin/lrelease/pageRank /jpkc/01/Data/graphdb/ukunion/ Matrixos/ 8; done
echo 3 > /proc/sys/vm/drop_caches
echo "===========================================yahoo"
for ((i=1 ; i<3 ; i++)) do time ./bin/lrelease/pageRank /jpkc/01/Data/graphdb/yahoolink1/ Matrixos/ 8; done
