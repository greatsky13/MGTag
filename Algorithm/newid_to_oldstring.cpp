#include "TripleBit.h"
#include "URITable.h"
#include "TempFile.h"

int main(int argc, char* argv[])
{
	if(argc != 4) {
		fprintf(stderr, "Usage: %s <Database Directory> <Dictionary Directory> <vertex_pair file>\n", argv[0]);
		return -1;
	}

	struct timeval start_time, end_time;
	gettimeofday(&start_time, NULL);
	
	MemoryMappedFile new_mapto_old;
	assert(new_mapto_old.open((string(argv[1])+"/new_mapto_old.0").c_str()));
	ID *newID_mapto_oldID = (ID *)new_mapto_old.getBegin();
	
	FILE *vertex_string_pair = fopen((string(argv[1])+"/vertex_string_pair").c_str(), "w");
	URITable *uriTable = URITable::load(argv[2]);
	cout<<"uriTable loaded"<<endl;
	
	ID vertex1, vertex2;
	string vertex1_str, vertex2_str;
	Status status1, status2;
	
	freopen(string(argv[3]).c_str(), "r", stdin);
	while(scanf("%d %d", &vertex1, &vertex2) != EOF){
		status1 = uriTable->getURIById(vertex1_str, newID_mapto_oldID[vertex1]);
		status2 = uriTable->getURIById(vertex2_str, newID_mapto_oldID[vertex2]);
		if(status1 == OK && status2 == OK)fprintf(vertex_string_pair, "%s %s\n", vertex1_str.c_str(), vertex2_str.c_str());
	}
	
	new_mapto_old.close();
	fclose(stdin);
	fclose(vertex_string_pair);
	delete uriTable;
	
	gettimeofday(&end_time, NULL);
	cout<<"time elapse: "<<((end_time.tv_sec - start_time.tv_sec) * 1000000 + (end_time.tv_usec - start_time.tv_usec)) / 1000000.0<<" s"<<endl;
	return 0;
}
