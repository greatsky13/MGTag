/*
 * graph_builder.cpp
 *
 *  Created on: 2014-10-9
 *      Author: root
 */

#include "TripleBit.h"
#include <signal.h>
#include <execinfo.h>
#include "MMapBuffer.h"
#include "HashID.h"
#include "BuildAdjGraph.h"
#include "Sorter.h"
#include "OSFile.h"

void handleSegmentFault(int sig) {
    void*  array[1024];
    size_t size;
    size = backtrace(array, sizeof(array) / sizeof(void*));
    backtrace_symbols_fd(array, size, fileno(stderr));
    abort();
    return;
}

int main(int argc, char* argv[]){
    if(argc != 4) {
        fprintf(stderr, "Usage: %s <Dataset> <Database Directory> <Dataset Line Count>\n", argv[0]);
        return -1;
    }
    signal(SIGSEGV, handleSegmentFault);

    string Dir = argv[2];
    if(OSFile::DirectoryExists(argv[2]) == false) {
        OSFile::MkDir(argv[2]);
    }

    struct timeval start_time, end_time, tmp_start, tmp_end;
    gettimeofday(&start_time, NULL);

    //TempFile* rawFile = new TempFile(Dir+"/rawTest");
    string path = argv[1];
    {
        HashID* hi = new HashID(Dir);
        hi->convert_adj_nonum(path, atoi(argv[3]));
        hi->init(true);
		hi->topology_encode(Dir);
        //hi->encode(true);
        //hi->convertToRaw(rawFile, true);
        delete hi;
    }
    /*gettimeofday(&tmp_end, NULL);
    cout<<"encode_so time elapse:"<<((tmp_end.tv_sec - start_time.tv_sec) * 1000000 + (tmp_end.tv_usec - start_time.tv_usec)) / 1000000.0<<" s"<<endl;

    cout<<"--------------------------------"<<endl;
    gettimeofday(&tmp_start, NULL);
    cout <<"start build Matrixso" <<endl;
    TempFile* rawso =  new TempFile(Dir+"/rawTest.so");
    Sorter::sort(*rawFile,*rawso,TempFile::skipIdId,TempFile::compare12);
    BuildAdjGraph* bua = new BuildAdjGraph();
    bua->convertToAdj(rawso->getFile().c_str(),Dir.c_str(),"Matrixso",0);
    rawFile->discard();
    rawso->discard();
    delete rawFile;
    delete rawso;
    delete bua;
    cout <<"finish building Matrixso" <<endl;
    gettimeofday(&tmp_end, NULL);
    cout<<"build_so time elapse:"<<((tmp_end.tv_sec - tmp_start.tv_sec) * 1000000 + (tmp_end.tv_usec - tmp_start.tv_usec)) / 1000000.0<<" s"<<endl;*/

    cout<<"--------------------------------"<<endl;
    gettimeofday(&end_time, NULL);
    cout<<"build time elapse:"<<((end_time.tv_sec - start_time.tv_sec) * 1000000 + (end_time.tv_usec - start_time.tv_usec)) / 1000000.0<<" s"<<endl;
    return 0;
}
