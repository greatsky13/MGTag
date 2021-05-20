/*
 * PageRank.cpp
 *
 *  Created on: 2013-4-18
 *      Author: yaya
 */


#include "TripleBit.h"
#include <signal.h>
#include <execinfo.h>
#include "MMapBuffer.h"
#include "AppExample.h"
#include <vector>

void handleSegmentFault(int sig) {
	void* array[1024];
	size_t size;
	size = backtrace(array, sizeof(array) / sizeof(void*));
	backtrace_symbols_fd(array, size, fileno(stderr));
	abort();
	return;
}

int cilk_main(int argc, char* argv[]) {
	if(argc != 5) {
		fprintf(stderr, "Usage: %s <Database Directory> <Matrix> <iterate times> <printtop>\n", argv[0]);
		return -1;
	}
//	signal(SIGSEGV, handleSegmentFault);

	struct timeval start_time, end_time, end_time1;
	gettimeofday(&start_time, NULL);
	AppExample *app_exam = new AppExample(true, false, true);
	app_exam->init(argv[1], argv[2], true);
	gettimeofday(&end_time1, NULL);
	cout<<"load time cost:"<<((end_time1.tv_sec - start_time.tv_sec) * 1000000 + (end_time1.tv_usec - start_time.tv_usec)) / 1000000.0<<" s"<<endl;

	unsigned size = app_exam->getBucSize();
	unsigned maxID = app_exam->getMaxID();
	unsigned itera = atoi(argv[3]);

	for (unsigned j = 0; j < itera; j++) {
		cilk_for (unsigned i = 0; i < size; i++) {
			app_exam->pagerank(i, j);
		}
		app_exam->endComputePageRank();
		cout << "(cilk_for) end itera " << j << endl;
		/*if(j >= 1){
			double sum = app_exam->sum_of_changes();
        	cout<<"compared with iterate "<<j-1<<", total square deviation changes of iterate "<<j<<" is: "<<sum<<endl;
       	}
        app_exam->save_rank();*/
	}
	app_exam->save_rank();
	//app_exam->save(string(argv[1])+"/pagerank.0", app_exam->get_vertices_rank(), (maxID+1)*sizeof(float));

	cout << "compute over" << endl;
	app_exam->getMaxValue(atoi(argv[4]));
	delete app_exam;
	gettimeofday(&end_time, NULL);
	cout<<"run time cost:"<<((end_time.tv_sec - start_time.tv_sec) * 1000000 + (end_time.tv_usec - start_time.tv_usec)) / 1000000.0<<" s"<<endl;

	return 0;
}
