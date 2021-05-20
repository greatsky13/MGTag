/*
 * pagerank_workstealing.cpp
 *
 *  Created on: 2014-7-1
 *      Author: root
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

	//in itera 0, use cilk_for rather than work stealing to initialize the vertices,
	//for the partition tasks need little time, no need to split them into small chunk tasks
	cilk_for (unsigned i = 0; i < size; i++) {
		app_exam->pagerank(i, 0);
	}
	app_exam->endComputePageRank();
	cout << "(cilk_for) end itera 0" << endl;

	if(app_exam->small_graph()){
		//small graph means little tasks, so no need to split the partition tasks into chunk tasks
		for (unsigned j = 1; j < itera; j++){
			cilk_for(unsigned i = 0; i < size; i++) {
				app_exam->pagerank(i, j);
			}
			app_exam->endComputePageRank();
			cout << "(cilk_for) end itera " << j << endl;
		}
	}
	else{
		//split partition tasks into chunk tasks, then use work stealing as a scheduler
		for (unsigned j = 1; j < itera; j++){
			app_exam->pagerank(j);
			app_exam->endComputePageRank();
			cout << "(work stealing) end itera " << j << endl;
		}
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
