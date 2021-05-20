//#include "TripleBit.h"
//#include <signal.h>
//#include <execinfo.h>
//#include "MMapBuffer.h"
//#include "HashID.h"
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
//int cilk_main(int argc, char* argv[]){
//	if(argc != 6) {
//		fprintf(stderr, "Usage: %s <Dataset> <Database Directory> <Dataset Line Count> <first time run this dataset(1 or 0)> <edge(0) or adj(1) txt>\n", argv[0]);
//		return -1;
//	}
//    signal(SIGSEGV, handleSegmentFault);
//
//	string Dir = argv[2];
//	if(OSFile::DirectoryExists(argv[2]) == false) {
//		OSFile::MkDir(argv[2]);
//	}
//
//	struct timeval start_time, end_time, start_time1, end_time1;
//	gettimeofday(&start_time, NULL);
//	TempFile* rawFile = new TempFile(Dir+"/rawTest");
//	TempFile* rawos =  new TempFile(Dir+"/rawTest.os");
//	string path = argv[1];
//	{
//		DebugTimer timer("DFS = %.2fs\n");
//		DebugTimerObject _(timer);
//		HashID* hi = new HashID(Dir);
//		if(atoi(argv[5]) == 1){
//			hi->convert_adj_nonum(path, atoi(argv[3]));
//			hi->init(true);
//			hi->convert_adj_to_reverse_adj(path, atoi(argv[3]));
//			hi->init(false);
//		}else if(atoi(argv[5]) == 0){
//			hi->convert_edge(path, atoi(argv[3]));
//			hi->init(true);
//			hi->convert_edge_to_reverse_adj(path, atoi(argv[3]));
//			hi->init(false);
//		}
//
//		//unsigned len = hi->getAdjNum();
//		//cout <<"len:" << len << endl;
////		cilk_for( unsigned i = 0;i <= len;i++){
////			hi->sort_degree(i);
////		}
////		cout <<"end all sort"<< endl;
//
////		hi->convert_adj_to_reverse_adj(path, atoi(argv[3]));
////		cout<<"convert okay"<<endl;
////		hi->init(false);
////		cout<<"reverse init okay"<<endl;
////		hi->convert_adj_nonum(path, atoi(argv[3]));
////		hi->init(true);
////		cout<<"init okay"<<endl;
//		hi->pre_code(Dir, atoi(argv[4]), start_time1, end_time1, end_time);
//
////		 srand((unsigned int)time(NULL));
////		 ID min = 1, max;
////		 max = hi->get_idcount();
////		 unsigned num = 1000000;
////		 ID *random_x = (ID *)calloc(num, sizeof(ID));
////		 ID *random_y = (ID *)calloc(num, sizeof(ID));
////		 bool *reachable = (bool *)calloc(num, sizeof(bool));
////		 memset(reachable, false, num);
////		 for (size_t i = 0; i < num; i++) {
////			 random_x[i] = rand() % (max - min + 1) + min;
////			 random_y[i] = rand() % (max - min + 1) + min;
////			 while (random_x[i] == random_y[i]) {
////				 random_y[i] = rand() % (max - min + 1) + min;
////			 }
////		}
////
////		gettimeofday(&start_time1, NULL);
////		cilk_for(unsigned i = 0; i <= num; i ++) {
////			hi->vertices_reachable(random_x[i], random_y[i], reachable[i]);
////		}
////		cout<<"DFS_fst okay"<<endl;
////		hi->DFS(true);
//
////		hi->convertToRaw(rawFile);
//		delete hi;
////		rename(string(Dir+"/degFile.forward.0").c_str(), string(Dir+"/degFile.backward.0").c_str());
//	}
//
////    DebugTimer timer("Sorter = %f.2fs\n");
////    DebugTimerObject _(timer);
////
////	cout<<"--------------------------------"<<endl;
////	cout <<"start build Matrixos" <<endl;
////    Sorter::sort(*rawFile,*rawos,TempFile::skipIdId,TempFile::compare21);
////    BuildAdjGraph* bua = new BuildAdjGraph();
////    bua->convertToAdj(rawos->getFile().c_str(),Dir.c_str(),"Matrixos",1);
////    delete bua;
////
////	cout<<"--------------------------------"<<endl;
////    cout <<"start build Matrixso" <<endl;
////    TempFile* rawso =  new TempFile(Dir+"/rawTest.so");
////    Sorter::sort(*rawFile,*rawso,TempFile::skipIdId,TempFile::compare12);
////    BuildAdjGraph* bub = new BuildAdjGraph();
////    bub->convertToAdj(rawso->getFile().c_str(),Dir.c_str(),"Matrixso",0);
////    delete bub;
////
////    rawos->discard();
////	rawFile->discard();
////	rawso->discard();
////    delete rawos;
////    delete rawso;
////    delete rawFile;
//
//	//gettimeofday(&end_time, NULL);
//	cout<<"--------------------------------"<<endl;
//	cout<<"total build time elapse:"<<((end_time.tv_sec - start_time.tv_sec) * 1000000 + (end_time.tv_usec - start_time.tv_usec)) / 1000000.0<<" s"<<endl;
//	cout<<"generate DAG build time elapse:"<<((end_time1.tv_sec - start_time.tv_sec) * 1000000 + (end_time1.tv_usec - start_time.tv_usec)) / 1000000.0<<" s"<<endl;
//	cout<<"index construction build time elapse:"<<((start_time1.tv_sec - end_time1.tv_sec) * 1000000 + (start_time1.tv_usec - end_time1.tv_usec)) / 1000000.0<<" s"<<endl;
//	cout<<"reachability build time elapse:"<<(end_time.tv_sec - start_time1.tv_sec) * 1000.0 + (end_time.tv_usec - start_time1.tv_usec)*1.0/ 1000.0<<" ms"<<endl;
//	cout<<"reachability build time elapse:"<<((end_time.tv_sec - start_time1.tv_sec) * 1000000 + (end_time.tv_usec - start_time1.tv_usec)) / 1000000.0<<" s"<<endl;
//	return 0;
//}

#include "TripleBit.h"
#include <signal.h>
#include <execinfo.h>
#include "MMapBuffer.h"
//#include "reach.h"
#include "Graph.h"
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
	if(argc != 7) {
		fprintf(stderr, "Usage: %s <Dataset> <scc dataset> <Database Directory> <DAG Database Directory> <testfile name> <DAG gra directory>\n", argv[0]);
		return -1;
	}
    signal(SIGSEGV, handleSegmentFault);

	string Dir1 = argv[3];
	if(OSFile::DirectoryExists(argv[3]) == false) {
		OSFile::MkDir(argv[3]);
	}

	string Dir2 = argv[4];
	if(OSFile::DirectoryExists(argv[4]) == false) {
		OSFile::MkDir(argv[4]);
	}

	//string path = argv[1];
	char* filename = argv[1];
	ifstream infile(filename);
	if (!infile) {
		cout << "Error: Cannot open " << filename << endl;
		return -1;
	}

	//string path2 = argv[2];
	char* filename2 = argv[2];
	ifstream infile2(filename2);
	if (!infile2) {
		cout << "Error: Cannot open " << filename2 << endl;
		return -1;
	}

	//char *testfilename = argv[3];
	string Dir3 = argv[5];
	string Dir4 = argv[6];
	Graph g(infile, infile2, Dir1, Dir2, Dir3, Dir4);
	//g.process(Dir, testfilename);


	return 0;
}
