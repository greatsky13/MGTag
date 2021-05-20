#include "TripleBitBuilder.h"
#include "OSFile.h"


int main(int argc, char* argv[])
{
	if(argc != 3) {
		fprintf(stderr, "Usage: %s <N3 file name> <Database Directory>\n", argv[0]);
		return -1;
	}

	if(OSFile::DirectoryExists(argv[2]) == false) {
		OSFile::MkDir(argv[2]);
	}

	DATABASE_PATH = argv[2];
	TripleBitBuilder* builder = new TripleBitBuilder(argv[2]);
	builder->startBuildN3(argv[1]);
	builder->endBuild();
	delete builder;

	return 0;
}
