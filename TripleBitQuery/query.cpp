
#include "TripleBitRepository.h"


int main(int argc, char* argv[])
{
	if(argc != 3) {
		fprintf(stderr, "Usage: %s <TripleBit Directory> <Query files Directory>\n", argv[0]);
		return -1;
	}


	TripleBitRepository* repo = TripleBitRepository::create(argv[1]);
	if(repo == NULL) {
		return -1;
	}

	repo->cmd_line(stdin, stdout, argv[2]);


/*	unsigned offset = 0;
	repo->getBitmapBuffer()->predicate_managers[0][4]->getChunkIndex(1)->getFirstOffsetByID(
			135, offset, 2);
	cout << "offset:" << offset << endl;*/


	delete repo;

	return 0;
}
