/*
 * generate_tol_graph.cpp
 *
 *  Created on: 2017Äê4ÔÂ13ÈÕ
 *      Author: zhoushuang
 */


#include "TripleBit.h"
#include <signal.h>
#include <execinfo.h>
#include "MMapBuffer.h"
#include "reach.h"
//#include "Graph.h"
#include "BuildAdjGraph.h"
#include "DebugTimer.h"
#include "Sorter.h"
#include "OSFile.h"
#include "TempFile.h"

void handleSegmentFault(int sig) {
    void*  array[1024];
    size_t size;
    size = backtrace(array, sizeof(array) / sizeof(void*));
    backtrace_symbols_fd(array, size, fileno(stderr));
    abort();
    return;
}

int cilk_main(int argc, char* argv[]){
	if(argc != 3) {
		fprintf(stderr, "Usage: %s<dataset> <graph file>\n", argv[0]);
		return -1;
	}
    signal(SIGSEGV, handleSegmentFault);

//	string Dir = argv[2];
//	if(OSFile::DirectoryExists(argv[2]) == false) {
//		OSFile::MkDir(argv[2]);
//	}

	string path = argv[1];
	char* filename = argv[1];
	ifstream infile(filename);
	if (!infile) {
		cout << "Error: Cannot open " << filename << endl;
		return -1;
	}

	//char *testfilename = argv[3];
	reach g(infile, argv[2]);

	return 0;
}

