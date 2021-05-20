//#include "TripleBit.h"
//#include <signal.h>
//#include <execinfo.h>
//#include "MMapBuffer.h"
//#include "Graph.h"
//#include "BuildAdjGraph.h"
//#include "DebugTimer.h"
//#include "Sorter.h"
//#include "OSFile.h"
//
//void handleSegmentFault(int sig) {
//    void*  array[1024];
//    size_t size;
//    size = backtrace(array, sizeof(array) / sizeof(void*));
//    backtrace_symbols_fd(array, size, fileno(stderr));
//    abort();
//    return;
//}
//
int cilk_main(int argc, char* argv[]){
//	if(argc != 6) {
//		fprintf(stderr, "Usage: %s <Dataset> <Database Directory> <testfilename>\n", argv[0]);
//		return -1;
//	}
//    signal(SIGSEGV, handleSegmentFault);
//
//	string Dir = argv[2];
//	if(OSFile::DirectoryExists(argv[2]) == false) {
//		OSFile::MkDir(argv[2]);
//	}
//
//	char* filename = &argv[1];
//	ifstream infile(filename);
//	if (!infile) {
//		cout << "Error: Cannot open " << filename << endl;
//		return -1;
//	}
//
//	Graph g(infile);
//
//	// prepare queries
//	srand48(time(NULL));
//	cout << "preparing queries..." << endl;
//	vector<ID> src;
//	vector<ID> dest;
//	vector<ID> labels;
//
//	int success=0,fail=0;
//	int label;
//	ID s , t;
//
//	char *testfilename = &argv[3];
//	if(testfilename!=NULL){
//      std::ifstream fstr(testfilename);
//		while(!fstr.eof()) {
//         fstr >> s >> t >> label;
//			src.push_back(s);
//			dest.push_back(t);
//			labels.push_back(label);
//		}
//	}
//
//	cout << "queries are ready" << endl;
//
//	struct timeval start_time, end_time;
//	gettimeofday(&start_time, NULL);
//	//start indexing
//	g.construct_index(argv[1]);
//	gettimeofday(&end_time, NULL);
//	cout<<"index construction build time elapse:"<<(end_time.tv_sec - start_time.tv_sec) * 1000 + (end_time.tv_usec - start_time.tv_usec) * 1.0/ 1000.0<<" ms"<<endl;
//
//	// process queries
//	cout << "process queries..." << endl;
//	bool r;
//	gettimeofday(&start_time, NULL);
//	for(vector<ID>::iterator x=src.begin(), y=dest.begin() ,l = labels.begin(); x!=src.end(); x++, y++, l++) {
//		r = g.topo_reach(*x, *y);
//
//		if(r==true) {
//                	if(*l == 0) {
//                	    fail++;
//                	}
//                	else success++;
//        }
//       	else {
//                if(*l == 1) {
//                	fail++;
//                }
//                else   success++;
//         }
//	}
//	gettimeofday(&end_time, NULL);
//
//	cout<<"reachability build time elapse:"<<(end_time.tv_sec - start_time.tv_sec) * 1000.0 + (end_time.tv_usec - start_time.tv_usec)*1.0/ 1000.0<<" ms"<<endl;
//	cout << "Success Rate " << success << "/" << success+fail << endl;
	return 0;
	}
