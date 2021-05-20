#!/bin/sh
echo "start"
#basePath="/jpkc/01/Data/io_mem_data/"
while :
do 
	#free -m >> $basePath$1
	free -m >> $1
	sleep 1
done
echo "finish"
