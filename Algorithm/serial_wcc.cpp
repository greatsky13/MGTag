/*
 * serial_wcc.cpp
 *
 *  Created on: Mar 18, 2014
 *      Author: root
 */

#include "TripleBit.h"
#include "OSFile.h"
#include "WccDriver.h"

int count_greater(pair<ID, unsigned> left, pair<ID, unsigned> right){
	return left.second > right.second;
}

int print_top_n(map<ID, pair<ID, unsigned> > cc_info, string dir, int printtop){
	vector<pair<ID, unsigned> > cc_count;
	for(map<ID, pair<ID, unsigned> >::iterator cc_iter = cc_info.begin(), cc_limit = cc_info.end();cc_iter != cc_limit;cc_iter++){
		cc_count.push_back(make_pair<ID, unsigned>((*cc_iter).second.first,(*cc_iter).second.second));
	}
	sort(cc_count.begin(),cc_count.end(),count_greater);

	string outName = dir + "/components.txt";
	FILE *resf = fopen(outName.c_str(), "w");
	if(resf == NULL){
		cerr<<"create file components.txt error"<<endl;
		return -1;
	}

	fprintf(resf, "component: count\n");
	for(size_t i = 0;i < cc_count.size();i++){
		fprintf(resf,"%u: %u\n", cc_count[i].first, cc_count[i].second);
	}
	fclose(resf);

	cout<<"total number of connected-components: "<<cc_count.size()<<endl;
	cout<<"List of labels was written to file: "<<outName<<endl;
	cout<<"top "<<printtop<<" follows:"<<endl;
	for(int i = 0;i < (int)std::min((size_t)printtop, cc_count.size());i++){
		cout<<(i+1)<<". label: "<<cc_count[i].first<<" , size: "<<cc_count[i].second<<endl;
	}

	return 1;
}

int main(int argc, char* argv[])
{
	if(argc != 4) {
		fprintf(stderr, "Usage: %s <Database Directory> <Matrix> <printtop>\n", argv[0]);
		return -1;
	}

	struct timeval start_time, end_time;
	gettimeofday(&start_time, NULL);
	WccDriver* driver = new WccDriver();
	driver->init(argv[1],argv[2],true,false);
	ID maxID = driver->getMaxID();

	int printtop = atoi(argv[3]);
	map<ID, pair<ID, unsigned> > cc_info;
	driver->wcc_in_serial(cc_info);

	print_top_n(cc_info, argv[1], printtop);

	delete driver;
	gettimeofday(&end_time, NULL);
	cout<<"wcc time elapse: "<<((end_time.tv_sec - start_time.tv_sec) * 1000000 + (end_time.tv_usec - start_time.tv_usec)) / 1000000.0<<" s"<<endl;
	return 0;
}
