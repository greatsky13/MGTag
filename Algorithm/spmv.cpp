#include "TripleBit.h"
#include "OSFile.h"
#include "MMapBuffer.h"
#include "EntityIDBuffer.h"
#include "AppExample.h"

int cilk_main(int argc, char* argv[])
{
	if(argc != 3) {
		fprintf(stderr, "Usage: %s <Database Directory> <Matrix>\n", argv[0]);
		return -1;
	}

	struct timeval start_time, end_time;
	gettimeofday(&start_time, NULL);
	AppExample* app_exam = new AppExample(false, false, true);
	if(app_exam == NULL) {
		cout << "create failed!" << endl;
		return -1;
	}
	app_exam->init(argv[1],argv[2],false);

	unsigned bucsize = app_exam->getBucSize();
	unsigned maxID = app_exam->getMaxID();
	double* res = (double *)calloc(maxID+1,sizeof(double));
	assert(res);

	cilk_for(int i = 0; i< bucsize; i++){
		app_exam->spmv(i,res);
	}
	cout<<"spmv finished"<<endl;
	gettimeofday(&end_time, NULL);
	cout<<"spmv compute time elapse: "<<((end_time.tv_sec - start_time.tv_sec) * 1000000 + (end_time.tv_usec - start_time.tv_usec)) / 1000000.0<<" s"<<endl;

	cout<<"start to write the results of spmv"<<endl;
	TempFile* tempFile = new TempFile(string(argv[1])+"/spmv");
	tempFile->write((maxID+1)*sizeof(double),(char*)res);
	tempFile->close();
	delete tempFile;
	//app_exam->save(string(argv[1])+"/spmv.0", (char *)res, (maxID+1)*sizeof(double));
	cout<<"finish writing the results"<<endl;

	free(res);
	delete app_exam;
	gettimeofday(&end_time, NULL);
	cout<<"spmv time elapse: "<<((end_time.tv_sec - start_time.tv_sec) * 1000000 + (end_time.tv_usec - start_time.tv_usec)) / 1000000.0<<" s"<<endl;

	return 0;
}
