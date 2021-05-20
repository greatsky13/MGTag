#!/usr/bin/python

import os
import sys

if __name__ == "__main__":
	factor = int(1000000 / int(sys.argv[1])) + 1
	for i in range(factor):
		os.system("cat %s >> test" % (sys.argv[2]))
	sys.exit(0) 
