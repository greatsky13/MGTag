#!/bin/sh
export PATH=/usr/local/cilk/bin/:$PATH
#echo 3 > /proc/sys/vm/drop_caches
#time ./bin/lrelease/builder /jpkc/03/zwy_Data/graph/wcc/webbase/webbase-2001.txt /jpkc/01/Data/newdb/webbase/
#echo 3 > /proc/sys/vm/drop_caches
#time ./bin/lrelease/builder /jpkc/03/zwy_Data/graph/wcc/twitter/twitter-2010.txt /jpkc/01/Data/newdb/twitter/
#echo 3 > /proc/sys/vm/drop_caches
#time ./bin/lrelease/builder /jpkc/03/zwy_Data/graph/wcc/uk2007/uk2007.txt /jpkc/01/Data/newdb/uk2007/
#echo 3 > /proc/sys/vm/drop_caches
#time ./bin/lrelease/builder /jpkc/03/zwy_Data/graph/wcc/ukunion/ukunion.txt /jpkc/01/Data/newdb/ukunion/
#echo 3 > /proc/sys/vm/drop_caches
#time ./bin/lrelease/builder /jpkc/03/zwy_Data/graph/wcc/yahoo/yahulink.txt /jpkc/01/Data/newdb/yahoo/
echo 3 > /proc/sys/vm/drop_caches
for ((i=1 ; i<3 ; i++)) do time ./bin/lrelease/par_wcc_2 /jpkc/01/Data/newdb/amazon/ Matrixso/ 20; done
echo 3 > /proc/sys/vm/drop_caches
for ((i=1 ; i<3 ; i++)) do time ./bin/lrelease/par_wcc_2 /jpkc/01/Data/newdb/dblp/ Matrixso/ 20; done
echo 3 > /proc/sys/vm/drop_caches
for ((i=1 ; i<3 ; i++)) do time ./bin/lrelease/par_wcc_2 /jpkc/01/Data/newdb/enwiki/ Matrixso/ 20; done
echo 3 > /proc/sys/vm/drop_caches
for ((i=1 ; i<3 ; i++)) do time ./bin/lrelease/par_wcc_2 /jpkc/01/Data/newdb/webbase/ Matrixso/ 20; done
echo 3 > /proc/sys/vm/drop_caches
for ((i=1 ; i<3 ; i++)) do time ./bin/lrelease/par_wcc_2 /jpkc/01/Data/newdb/twitter/ Matrixso/ 20; done
echo 3 > /proc/sys/vm/drop_caches
for ((i=1 ; i<3 ; i++)) do time ./bin/lrelease/par_wcc_2 /jpkc/01/Data/newdb/uk2007/ Matrixso/ 20; done
echo 3 > /proc/sys/vm/drop_caches
for ((i=1 ; i<3 ; i++)) do time ./bin/lrelease/par_wcc_2 /jpkc/01/Data/newdb/ukunion/ Matrixso/ 20; done
echo 3 > /proc/sys/vm/drop_caches
#for ((i=1 ; i<3 ; i++)) do time ./bin/lrelease/par_wcc_2 /jpkc/01/Data/newdb/yahoo/ Matrixso/ 20; done
#echo 3 > /proc/sys/vm/drop_caches

