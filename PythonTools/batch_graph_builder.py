#!/usr/bin/python

import sys
import os

if __name__ == '__main__':
	clear_cmd = 'echo 3 > /proc/sys/vm/drop_caches'
	builder_cmd = './bin/lrelease/graph_builder /jpkc/03/zwy_Data/graph/dataset/twitter-2010.txt /jpkc/01/Data/PathGraph/2.5/twitter/ 41651940 > build_out2'
	os.system(clear_cmd)
	os.system(builder_cmd)
	builder_cmd = './bin/lrelease/graph_builder /jpkc/03/zwy_Data/graph/dataset/uk2007.txt /jpkc/01/Data/PathGraph/2.5/uk2007/ 105896268 >> build_out2'
	os.system(clear_cmd)
	os.system(builder_cmd)
	#builder_cmd = './bin/lrelease/graph_builder /jpkc/03/zwy_Data/graph/dataset/ukunion.txt /jpkc/01/Data/PathGraph/2.5/ukunion/ 133632930 >> build_out2'
	#os.system(clear_cmd)
	#os.system(builder_cmd)
	#builder_cmd = './bin/lrelease/graph_builder /jpkc/03/zwy_Data/graph/dataset/yahulink.txt /jpkc/01/Data/PathGraph/2.5/yahoo/ 1413511390 >> build_out2'
	#os.system(clear_cmd)
	#os.system(builder_cmd)
	sys.exit(0)

