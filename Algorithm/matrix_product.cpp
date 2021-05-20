#include "TripleBit.h"
#include "OSFile.h"
#include "MMapBuffer.h"
#include "EntityIDBuffer.h"
#include "OnePredicateOp.h"
#include "EntityIDBuffer.h"
#include "MatrixMapOp.h"

void getRunWay(int time,vector<int>& way){
	way.clear();
	while (time > 1) {
		if (time % 2 == 0) {
			time >>=1;
			way.push_back(time);
			way.push_back(time);
		} else {
			time--;
			way.push_back(1);
			way.push_back(time);
		}
	}
}

int cilk_main(int argc, char* argv[])
{
	if(argc != 4) {
		fprintf(stderr, "Usage: %s <Database Directory> <Matrix> <Multiplication times>\n", argv[0]);
		return -1;
	}
	OnePredicateOp* repo = new OnePredicateOp();
	if(repo == NULL) {
		cout << "create failed!" << endl;
		return -1;
	}
	repo->init(argv[1],argv[2],true,65536);

	vector<int> runway;
	getRunWay(atoi(argv[3]),runway);

	unsigned bucsize = repo->getBucSize();
	int big,small;
	while(runway.empty() == false){
		big = runway.back();
		runway.pop_back();
		small = runway.back();
		runway.pop_back();
		cout <<"run "<< argv[1] << argv[2] <<" " << big <<" " << small << endl;
		repo->loadMatrix(small,big,true);
		cilk_for (int i = 0; i < bucsize; i++) {
			repo->matrixMultiply(small,big,i);
		}
		repo->clearMatrix(small,big);
	}

	delete repo;
	return 0;
}
