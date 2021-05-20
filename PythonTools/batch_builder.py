#!/usr/bin/python

import sys
import os

if __name__ == '__main__':
	cmd = './bin/lrelease/builder /jpkc/03/zwy_Data/graph/dataset/ukunion.txt /jpkc/01/Data/PathGraph/2.4/ukunion/ 133632930 >> out2'
	os.system(cmd)
	cmd = 'echo 3 > /proc/sys/vm/drop_caches'
	os.system(cmd)
	cmd = './bin/lrelease/builder /jpkc/03/zwy_Data/graph/dataset/uk2007.txt /jpkc/01/Data/PathGraph/2.4/uk2007/ 105896268 >> out2'
	os.system(cmd)
	sys.exit(0)

