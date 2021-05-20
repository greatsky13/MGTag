#!/usr/bin/python

import os
import sys

if __name__ == "__main__":
	os.system("echo 3 > /proc/sys/vm/drop_caches")
	os.system("./bin/lrelease/range_graph_builder /jpkc/01/xcf_data/Partitioner/Dataset/Adj_LUBM100M/subject_objectlist /jpkc/01/Data/PathGraph/2.5/RDF/Range_Graph/LUBM100M/ 21735128 > out_100m")
	os.system("echo 3 > /proc/sys/vm/drop_caches")
	os.system("./bin/lrelease/range_graph_builder /jpkc/01/xcf_data/Partitioner/Dataset/Adj_LUBM200M_3/subject_objectlist /jpkc/01/Data/PathGraph/2.5/RDF/Range_Graph/LUBM200M/ 43438912 > out_200m")
	os.system("echo 3 > /proc/sys/vm/drop_caches")
	os.system("./bin/lrelease/range_graph_builder /jpkc/01/xcf_data/Partitioner/Dataset/Adj_LUBM300M/subject_objectlist /jpkc/01/Data/PathGraph/2.5/RDF/Range_Graph/LUBM300M/ 65155054 > out_300m")
	os.system("echo 3 > /proc/sys/vm/drop_caches")
	os.system("./bin/lrelease/range_graph_builder /jpkc/01/xcf_data/Partitioner/Dataset/Adj_LUBM500M_3/subject_objectlist /jpkc/01/Data/PathGraph/2.5/RDF/Range_Graph/LUBM500M/ 108598614 > out_500m")
	os.system("echo 3 > /proc/sys/vm/drop_caches")
	os.system("./bin/lrelease/range_graph_builder /jpkc/01/xcf_data/Partitioner/Dataset/Adj_yago/subject_objectlist /jpkc/01/Data/PathGraph/2.5/RDF/Range_Graph/yago/ 72136686 > out_yago")
	os.system("echo 3 > /proc/sys/vm/drop_caches")
	os.system("./bin/lrelease/range_graph_builder /jpkc/01/xcf_data/Partitioner/Dataset/Adj_dbpedia_pagelinks_en/subject_objectlist /jpkc/01/Data/PathGraph/2.5/RDF/Range_Graph/dbpedia_pagelinks_en/ 10139707 > out_pahelinks_en")
	sys.exit(0)

