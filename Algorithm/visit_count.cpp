#include "TripleBit.h"
#include "OSFile.h"
#include "MMapBuffer.h"
#include "EntityIDBuffer.h"
#include "AppExample.h"
#include "EntityIDBuffer.h"
#include "MatrixMapOp.h"
#include "TempFile.h"

int cilk_main(int argc, char* argv[])
{
	if(argc != 3) {
		fprintf(stderr, "Usage: %s <Database Directory> <Matrix>\n", argv[0]);
		return -1;
	}

	struct timeval start_time, end_time;
	gettimeofday(&start_time, NULL);
	AppExample* app_exam = new AppExample(false, true, false);
	if(app_exam == NULL) {
		cout << "create failed!" << endl;
		return -1;
	}
	app_exam->init(argv[1],argv[2],true);

	unsigned maxID = app_exam->getMaxID();
	MemoryMappedFile *rootFile = new MemoryMappedFile();
	assert(rootFile->open((string(argv[1])+"/new_root.forward.0").c_str()));
	unsigned rootNum = (rootFile->getEnd() - rootFile->getBegin()) / sizeof(ID);
	cout<<"tree number: "<<rootNum<<endl;
	ID *root_arr = (ID*) rootFile->getBegin();
	
	MemoryMappedFile *degFile = new MemoryMappedFile();
	assert(degFile->open((string(argv[1])+"/degFile.forward.0").c_str()));
	unsigned *deg = (unsigned *)degFile->getBegin();
	
	MMapBuffer *resBuf = new MMapBuffer((string(argv[1])+"/visitedCount").c_str(), maxID * sizeof(ID));
	unsigned *res = (unsigned *)(resBuf->get_address());
	memset(res, 0, maxID * sizeof(ID));
	
	unsigned loop_vertices = 0;
	unsigned long total_visit_count = 0;
	
	app_exam->visit_count(string(argv[1]), rootNum, root_arr, res, deg, &loop_vertices);
	
	for(int i = 0;i < rootNum;i++){
		total_visit_count += res[root_arr[i]];
	}
	cout<<"loop_vertices: "<<loop_vertices<<", total_visit_count: "<<total_visit_count<<endl;
	
	rootFile->close();
	degFile->close();
	delete rootFile;
	delete degFile;
	delete resBuf;
	delete app_exam;
	gettimeofday(&end_time, NULL);
	cout<<"parallel visit calculate time elapse: "<<((end_time.tv_sec - start_time.tv_sec) * 1000000 + (end_time.tv_usec - start_time.tv_usec)) / 1000000.0<<" s"<<endl;
	return 0;
}
