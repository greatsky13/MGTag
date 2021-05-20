/*
 * generate_new_root.cpp
 *
 *  Created on: 2014-4-14
 *      Author: root
 */

#include "TempFile.h"
#include <signal.h>
#include <execinfo.h>

void handleSegmentFault(int sig) {
        void* array[1024];
        size_t size;
        size = backtrace(array, sizeof(array) / sizeof(void*));
        backtrace_symbols_fd(array, size, fileno(stderr));
        abort();
        return;
}

int main(int argc, char* argv[]) {
	if(argc != 2){
		fprintf(stderr, "Usage: %s <Database Directory>\n", argv[0]);
		return -1;
	}
	signal(SIGSEGV, handleSegmentFault);
	
	string dir = argv[1];
	MemoryMappedFile *degFile = new MemoryMappedFile();
	assert(degFile->open(string(dir+"/degFile.forward.0").c_str()));
	unsigned num = (degFile->getEnd() - degFile->getBegin()) / (2 * sizeof(unsigned));
	cout<<"vertex num: "<<num<<endl;
	unsigned *degree = (unsigned *)degFile->getBegin();
	TempFile *newRoot = new TempFile(dir+"/new_root.forward", 0);

	for(size_t i = 0, id =1; id <= num; id++, i+=2){
		if(!degree[i] && degree[i+1]){
			newRoot->writeId(id);
		}
	}

	newRoot->flush();
	cout<<"finish"<<endl;
	newRoot->close();
	degFile->close();
	delete degFile;
	delete newRoot;
	return 0;
}
