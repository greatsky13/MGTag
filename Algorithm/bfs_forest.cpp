#include "TripleBit.h"
#include "OSFile.h"
#include "MMapBuffer.h"
#include "EntityIDBuffer.h"
#include "OnePredicateOp.h"
#include "EntityIDBuffer.h"
#include "MatrixMapOp.h"

int cilk_main(int argc, char* argv[])
{
	if(argc != 3) {
		fprintf(stderr, "Usage: %s <Database Directory> <Matrix>\n", argv[0]);
		return -1;
	}

	struct timeval start_time, end_time;
	gettimeofday(&start_time, NULL);
	OnePredicateOp* repo = new OnePredicateOp();
	if(repo == NULL) {
		cout << "create failed!" << endl;
		return -1;
	}
	repo->init(argv[1],argv[2],false,65536);

	unsigned bucsize = repo->getBucSize();
	unsigned maxID = repo->getMaxID();
	char* ispush = (char*)calloc(maxID+1,sizeof(char));
	unsigned treeNum = 0;

   	for(int i = 0; i< bucsize; i++){
   		repo->bfs_forest(i,ispush,treeNum);
	}
	cout << "treeNum:" << treeNum << endl;

	free(ispush);
	delete repo;
	gettimeofday(&end_time, NULL);
	cout<<"bfs_forest time elapse: "<<((end_time.tv_sec - start_time.tv_sec) * 1000000 + (end_time.tv_usec - start_time.tv_usec)) / 1000000.0<<" s"<<endl;
	return 0;
}
