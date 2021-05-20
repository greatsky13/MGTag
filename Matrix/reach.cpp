#include "reach.h"
#include "TempFile.h"
#include "OSFile.h"
#include "Sorter.h"
#include "TSorter.h"
#include <list>

//#define FLAT_ADJLIST 1

string reach::Dir = "";
idDegree *globalDegree;

//ID* root_layer;
//vector < vector <ID> >cross_located; //change
//vector < vector <ID> >oldID_mapto_newID; //change
//ID* oldID_mapto_newID;
unsigned oldID_mapto_newID_count;

vector < vector <ID> > excpID_O;
vector < vector <ID> > id_cross_arr_pro;

vector <ID> temp_root_arr;
ID* reach_flag;
int querycnt;

reach::reach(unsigned maxID) {
	// TODO Auto-generated constructor stub
}

reach::reach(string dir) {
	Dir = dir;

	//oldID_mapto_newID = NULL;
	//newID_mapto_oldID = NULL;
	/////
	/////
	degree = NULL;
	idcount = 0;
	maxID = 0;
	maxFromID = 0;
	maxCount = 0;
}

//reach::reach(istream &in, string graphFile, int num){
//	string line;
//	getline(in, line);
//	cout<<line<<endl;
//
//	std::istringstream iss(line, std::istringstream::in);
//
//	int s = 0;
//	int t = 0;
//	ofstream outfile(graphFile.c_str());
//	outfile<<"graph_for_greach"<<"\n";
//	outfile<<num<<"\n";
//	cout<<num<<endl;
//	//getline(in, line);
//    while (getline(in, line)) {
//      iss.clear();
//      iss.str(line);
//      iss >> s;
//      outfile<<s<<":";
//      //cout<<s<<": ";
//      while (iss >> t) {
//    	  //cout<<t;
//			outfile<<" "<<t;
//        //std::cout<<s<<" "<<t<<std::endl;
//      }
//      //cout<<endl;
//      outfile<<" #"<<"\n";
//      //getline(in, line);
//    }
//    cout<<s<<endl;
//	outfile.close();
//	cout<<"end"<<endl;
//}

reach::reach(istream &in, string dataset){
	string buf;
	getline(in, buf);

	strTrimRight(buf);
	if (buf != "graph_for_greach") {
		cout << "BAD FILE FORMAT!" << endl;
		//exit(0);
	}

	getline(in, buf);
	std::istringstream iss(buf, std::istringstream::in);
	iss >> maxID;
	maxID = maxID - 1;

	cout<<"maxID:"<<maxID<<endl;
	cout <<"reading graph..."<<endl;

	ofstream outfile(dataset.c_str());
	outfile<<maxID+1<<"\n";
	int sid = 0;
	int tid = 0;
	int begin, end;
	while (getline(in, buf)) {
		begin = buf.find(":");
		end = buf.find_last_of("#");
		sid = atoi((buf.substr(0, begin)).c_str());
		buf = buf.substr(begin+2,end-begin-2);

		iss.clear();
		iss.str(buf);

		outfile<<sid;
		while (iss >> tid) {
			outfile<<" "<<tid;
		}
		outfile<<"\n";
	}

	 outfile.close();
	 cout<<"end"<<endl;
}

reach::reach(istream &in, string dataset, string daggraph, int num){
	Dir = dataset;
	degree = NULL;
	idcount = 0;
	maxID = 0;
	maxFromID = 0;
	maxCount = 0;

	string line;
	getline(in, line);
	std::istringstream iss(line, std::istringstream::in);

	int s = 0;
	int t = 0;
	ofstream outfile(daggraph.c_str());
	outfile<<"graph_for_greach"<<"\n";
	outfile<<num<<"\n";
	//getline(in, line);
    while (getline(in, line)) {
      iss.clear();
      iss.str(line);
      iss >> s;
      outfile<<s<<":";
      //cout<<s<<": ";
      while (iss >> t) {
    	  //cout<<t;
			outfile<<" "<<t;
        //std::cout<<s<<" "<<t<<std::endl;
      }
      //cout<<endl;
      outfile<<" #"<<"\n";
      //getline(in, line);
    }
	 outfile.close();
}

reach::reach(istream &in, string dataset, string sccpath, string tfpath){
	Dir = dataset;
	degree = NULL;
	idcount = 0;
	maxID = 0;
	maxFromID = 0;
	maxCount = 0;

	readGraph(in);
	ofstream outfile(sccpath.c_str());
	ofstream outfiletf(tfpath.c_str());
	 if(!outfile || !outfiletf){
		 cout<<"unable to open file!"<<endl;
		 exit(0);
	 }
	 outfile<<"# Directed Node Graph"<<"\n";
	 outfile<<"# synthetic graph"<<"\n";
	 outfile<<"# Nodes: "<<maxID + 1<<" Edges: "<<idcount<<"\n";
	 outfile<<"# SrcNId	DstNId"<<"\n";

	 outfiletf<<maxID + 1<<" "<<idcount<<"\n";
	 for(int i = 0; i <= maxID; i ++){
			ID *addr = (ID *)DAG_getOffset(i);
			assert(addr[0] == i);
			unsigned count = addr[1];
			addr += 2;
			outfiletf<<i<<" "<<count<<" ";
			for(int j = 0; j < count; j ++){
				outfile<<i<<"	"<<addr[j]<<"\n";
				outfiletf<<addr[j]<<" ";
			}
			outfiletf<<"\n";
	 }
	 outfile.close();
	 outfiletf.close();
}

reach::reach(istream &in, string dataset, string querypath){
	Dir = dataset;
	degree = NULL;
	idcount = 0;
	maxID = 0;
	maxFromID = 0;
	maxCount = 0;

	readGraph(in);

	generate_querys(querypath);
}

reach::reach(istream& in, string dir, unsigned f) {
	Dir = dir;
	//oldID_mapto_newID = NULL;
	//newID_mapto_oldID = NULL;
	degree = NULL;
	idcount = 0;
	maxID = 0;
	maxFromID = 0;
	maxCount = 0;

	if(f == 0){
		readGraph(in);
		//get_forward_root_vertices(maxID);
	}
	else{
		string buf;
		getline(in, buf);

		strTrimRight(buf);
		if (buf != "graph_for_greach") {
			cout << "BAD FILE FORMAT!" << endl;
			//exit(0);
		}

		getline(in, buf);
		istringstream(buf) >> maxID;
		maxID = maxID - 1;
		cout<<"maxID:"<<maxID<<endl;
		cout<<"init..."<<endl;
		init_DAG();
		//init_reverse_DAG();
		cout<<"init end"<<endl;
		cout<<"init degree:"<<endl;
		init_degree();
		//init_reverse_DAG();
		cout<<"endl"<<endl;
	}
}

reach::~reach() {
	// TODO Auto-generated destructor stub
	unsigned i = 0;
	unsigned adjSize = DAGMap.size();

//	if (oldID_mapto_newID){
//		free(oldID_mapto_newID);
//		oldID_mapto_newID = NULL;
//	}

//	if (newID_mapto_oldID) {
//		free(newID_mapto_oldID);
//		newID_mapto_oldID = NULL;
//	}

	for (i = 0; i < DAGMap.size(); i++) {
		delete DAGMap[i];
		DAGMap[i] = NULL;
		DAG[i] = NULL;
	}
	DAGMap.clear();
	DAG.clear();

	if(global_DAG_indexMap){
		delete global_DAG_indexMap;
		global_DAG_indexMap = NULL;
		global_DAG_index = NULL;
	}

//	if (degree) {
//		free(degree);
//		degree = NULL;
//	}

	char fileName[256];
	for (i = 0; i < adjSize; i++) {
		sprintf(fileName, "%s/adj.%d", Dir.c_str(), i);
		OSFile::FileDelete(fileName);
	}
	sprintf(fileName, "%s/global_adj_index.0", Dir.c_str());
	OSFile::FileDelete(fileName);
}

void reach::FIXLINE(char * s) {
	int len = (int) strlen(s) - 1;
	if (s[len] == '\n')
		s[len] = 0;
}

int qcomparedegree(const void* a, const void* b) {
	ID p1 = *(const ID*) a;
	ID p2 = *(const ID*) b;
	p1 = globalDegree[p1].outdeg - globalDegree[p1].indeg;
	p2 = globalDegree[p2].outdeg - globalDegree[p2].indeg;
	return p1 - p2;
}

int qcompareoutdegree(const void* a, const void* b) {
	//sort from large to small
	ID p1 = *(const ID*) a;
	ID p2 = *(const ID*) b;
	p1 = globalDegree[p1].outdeg;
	p2 = globalDegree[p2].outdeg;
	return p2 - p1;
}

int qcompareindegree(const void* a, const void* b) {
	//sort from small to large
	ID p1 = *(const ID*) a;
	ID p2 = *(const ID*) b;
	p1 = globalDegree[p1].indeg;
	p2 = globalDegree[p2].indeg;
	return p1 - p2;
}

int qcompare(const void* a, const void* b) {
	//sort from small to large
	return *(ID *)a - *(ID *)b;
}

int qcompareindegree21(const void* a, const void* b) {
	//sort from large to small
	ID p1 = *(const ID*) a;
	ID p2 = *(const ID*) b;
	p1 = globalDegree[p1].indeg;
	p2 = globalDegree[p2].indeg;
	return p2 - p1;
}

void reach::init_degree() {
	//degree = (unsigned*) calloc(maxID + 1, sizeof(unsigned));
	//assert(degree);
	globalDegree = (idDegree*) calloc(maxID + 1, sizeof(idDegree));
	assert(globalDegree);
	for(unsigned i = 0; i <= maxID; i ++){
		ID *addr = (ID *)DAG_getOffset(i);
		assert(addr[0] == i);
		unsigned count = addr[1];
		addr += 2;
		//degree[i].outdeg = count;
		globalDegree[i].outdeg = count;
		//cout<<i<<":";
		for(unsigned j = 0; j < count; j ++){
			//degree[addr[j]] ++;
			globalDegree[addr[j]].indeg ++;
			//cout<<addr[j]<<" ";
		}
		//cout<<endl;
	}
}

void reach::get_forward_root_vertices(ID maxID) {
	EntityIDBuffer* buffer = new EntityIDBuffer();
	buffer->setIDCount(1);

	//cout<<"root:"<<endl;
	for (ID start = 0; start <= maxID; start++) {
		if (!globalDegree[start].indeg /*&& globalDegree[start].outdeg*/) {
			buffer->insertID(start);
			//cout<<"start:"<<start<<" ";
		}
	}
	//cout<<endl;
	qsort((void*) buffer->getBuffer(), buffer->getSize(), 4, qcompareoutdegree);
	if (buffer->getSize() == 0)buffer->insertID(0);
	cout << "DAG forward root number: " << buffer->getSize() << endl;
	TempFile* rootFile = new TempFile(Dir + "/DAGroot.forward", 0);
	rootFile->write((size_t) buffer->getSize() * sizeof(ID),(const char*) buffer->getBuffer());
	rootFile->flush();
	delete rootFile;
	cout << "finish writing DAGrootFile" << endl;
	delete buffer;
}

void reach::strTrimRight(string& str) {
	string whitespaces(" \t");
	int index = str.find_last_not_of(whitespaces);
	if (index != string::npos)
		str.erase(index+1);
	else
		str.clear();
}

status reach::readGraph(istream& in) {
	string buf;
	getline(in, buf);

	strTrimRight(buf);
	if (buf != "graph_for_greach") {
		cout << "BAD FILE FORMAT!" << endl;
		//exit(0);
	}

	getline(in, buf);
	std::istringstream iss(buf, std::istringstream::in);
	iss >> maxID;
	maxID = maxID - 1;

	cout<<"maxID:"<<maxID<<endl;
	cout <<"reading graph..."<<endl;

	idcount = 0;

	string sub;
	int idx;
	int sid = 0;
	int tid = 0;
	TempFile* tempFile = new TempFile("edge_format");
	int begin, end;
	while (getline(in, buf)) {
		begin = buf.find(":");
		end = buf.find_last_of("#");
		sid = atoi((buf.substr(0, begin)).c_str());
		buf = buf.substr(begin+2,end-begin-2);

		iss.clear();
		iss.str(buf);
		while (iss >> tid) {
			if(sid == tid)
				cout << "Self-edge " << sid << endl;
			if(tid < 0 || tid > maxID)
				cout << "Wrong tid " << tid << endl;

			if(sid != tid){
				tempFile->writeId(sid);
				tempFile->writeId(tid);

				idcount ++;
			}
		}
	}
	tempFile->flush();

	TempFile* sortFile = new TempFile("sortFile");
	Sorter::sort(*tempFile, *sortFile, TempFile::skipIdId, TempFile::compare12);
	TempFile* r_sortFile = new TempFile("r_sortFile");
	Sorter::sort(*tempFile, *r_sortFile, TempFile::skipIdId, TempFile::compare21);
	tempFile->discard();
	delete tempFile;
	cout<<"sort end"<<endl;

	//---------------------------------------------------------------�ڽӱ�------------------------------------------------------------------
	MemoryMappedFile mappedIn;
	assert(mappedIn.open(sortFile->getFile().c_str()));
	const char* reader = mappedIn.getBegin(), *limit = mappedIn.getEnd();

	bool firstInsert = true, firstTime = true;
	int fileindex = -1;
	unsigned fileOff = 0;
	unsigned num;

	vector<TempFile *> adjFile;
	unsigned vertex_estimate = maxID + 1;
	size_t pageSize = vertex_estimate * sizeof(ID) / (MemoryBuffer::pagesize) + (((vertex_estimate * sizeof(ID)) % (MemoryBuffer::pagesize)) ? 1 : 0);
	char adj_indexPath[150];
	sprintf(adj_indexPath, "%s/global_DAG_index.0", Dir.c_str());
	MMapBuffer *global_adj_indexFile = new MMapBuffer(adj_indexPath, 2 * pageSize * MemoryBuffer::pagesize);
	unsigned *global_adj_index = (unsigned *)global_adj_indexFile->get_address();

	//degree = (idDegree*) calloc(vertex_estimate, sizeof(idDegree));
	//assert(degree);
	//globalDegree = (idDegree*) calloc(vertex_estimate, sizeof(idDegree));
	//assert(globalDegree);
	unsigned from = *(ID*) reader, to = 0, lastfrom = *(ID*) reader;
	EntityIDBuffer* tempEnt = new EntityIDBuffer();
	for(ID i = 0; i < from; i ++) {
		tempEnt->empty();
		tempEnt->insertID(i);
		tempEnt->insertID(0);

		num = tempEnt->getSize() - 2;
		tempEnt->getBuffer()[1] = num; //update num
		unsigned size = tempEnt->getSize() * sizeof(ID);
		if (!firstTime && fileOff + size < (unsigned) (-1) / 8) {
			adjFile[fileindex]->write(size, (const char*) tempEnt->getBuffer());
			global_adj_index[2 * i] = fileOff;
			global_adj_index[2 * i + 1] = fileindex;
			fileOff += size;
		} else {
			if (fileindex >= 0) {
				adjFile[fileindex]->close();
			}

			fileindex++;
			fileOff = 0;
			adjFile.push_back(new TempFile(Dir + "/DAG", fileindex));
			DAG_start_ID[fileindex] = i;
			adjFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
			global_adj_index[2 * i] = 1;//actually is '0', but we set '1' here, for '0' has been used to mark the unexist 'from data'
			global_adj_index[2 * i + 1] = fileindex;
			fileOff += size;
			firstTime = false;
		}
	}

	tempEnt->empty();
	while (reader < limit) {
		from = *(ID*) reader;
		reader += sizeof(ID);
		to = *(ID*) reader;
		//cout<<"from:"<<from<<" to:"<<to<<endl;
		reader += sizeof(ID);

		//degree[from].outdeg ++;
		//degree[to].indeg ++;

		//globalDegree[from].outdeg ++;
		//globalDegree[to].indeg ++;

		maxID = std::max(from, maxID);
		maxID = std::max(to, maxID);
		//maxFromID = std::max(from, maxFromID);

		if (firstInsert) {
			//cout<<"insert"<<endl;
			tempEnt->insertID(from);
			tempEnt->insertID(0); // for num
			tempEnt->insertID(to);
			firstInsert = false;
		} else if (from != lastfrom) {
			num = tempEnt->getSize() - 2;
			tempEnt->getBuffer()[1] = num; //update num
			unsigned size = tempEnt->getSize() * sizeof(ID);
			if (!firstTime && fileOff + size < (unsigned) (-1) / 8) {
				adjFile[fileindex]->write(size, (const char*) tempEnt->getBuffer());
				global_adj_index[2 * lastfrom] = fileOff;
				global_adj_index[2 * lastfrom + 1] = fileindex;
				fileOff += size;
			} else {
				if (fileindex >= 0) {
					adjFile[fileindex]->close();
				}

				fileindex++;
				fileOff = 0;
				adjFile.push_back(new TempFile(Dir + "/DAG", fileindex));
				DAG_start_ID[fileindex] = lastfrom;
				adjFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
				global_adj_index[2 * lastfrom] = 1;//actually is '0', but we set '1' here, for '0' has been used to mark the unexist 'from data'
				global_adj_index[2 * lastfrom + 1] = fileindex;
				fileOff += size;
				firstTime = false;
			}
			maxCount = max(maxCount, num);
			//cout<<"from:"<<from<<"lastfrom"<<lastfrom<<endl;
			if(from != lastfrom + 1){
				for(ID i = lastfrom + 1; i < from; i ++) {
					//cout<<"i:"<<i<<endl;
					tempEnt->empty();
					tempEnt->insertID(i);
					tempEnt->insertID(0);

					num = tempEnt->getSize() - 2;
					tempEnt->getBuffer()[1] = num; //update num
					unsigned size = tempEnt->getSize() * sizeof(ID);
					if (!firstTime && fileOff + size < (unsigned) (-1) / 8) {
						adjFile[fileindex]->write(size, (const char*) tempEnt->getBuffer());
						global_adj_index[2 * i] = fileOff;
						global_adj_index[2 * i + 1] = fileindex;
						fileOff += size;
					} else {
						if (fileindex >= 0) {
							adjFile[fileindex]->close();
						}

						fileindex++;
						fileOff = 0;
						adjFile.push_back(new TempFile(Dir + "/DAG", fileindex));
						DAG_start_ID[fileindex] = i;
						adjFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
						global_adj_index[2 * i] = 1;//actually is '0', but we set '1' here, for '0' has been used to mark the unexist 'from data'
						global_adj_index[2 * i + 1] = fileindex;
						fileOff += size;
						firstTime = false;
					}
				}
			}

			tempEnt->empty();
			tempEnt->insertID(from);
			tempEnt->insertID(0);
			tempEnt->insertID(to);
			lastfrom = from;
		} else {
			//cout<<"insert to:"<<to<<endl;
			tempEnt->insertID(to);
		}
	}

	num = tempEnt->getSize() - 2;
	tempEnt->getBuffer()[1] = num; //update num
	unsigned size = tempEnt->getSize() * sizeof(ID);
	if (!firstTime && fileOff + size < (unsigned) (-1) / 8) {
		adjFile[fileindex]->write(size, (const char*) tempEnt->getBuffer());
		global_adj_index[2 * lastfrom] = fileOff;
		global_adj_index[2 * lastfrom + 1] = fileindex;
		fileOff += size;
	} else {
		if (fileindex >= 0) {
			adjFile[fileindex]->close();
		}

		fileindex++;
		fileOff = 0;
		adjFile.push_back(new TempFile(Dir + "/DAG", fileindex));
		DAG_start_ID[fileindex] = lastfrom;
		adjFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
		global_adj_index[2 * lastfrom] = 1;//actually is '0', but we set '1' here, for '0' has been used to mark the unexist 'from data'
		global_adj_index[2 * lastfrom + 1] = fileindex;
		fileOff += size;
		firstTime = false;
	}
	maxCount = max(maxCount, num);
	for(ID i = lastfrom + 1; i <= maxID; i ++) {
		//cout<<"i:"<<i<<endl;
		tempEnt->empty();
		tempEnt->insertID(i);
		tempEnt->insertID(0);

		num = tempEnt->getSize() - 2;
		tempEnt->getBuffer()[1] = num; //update num
		unsigned size = tempEnt->getSize() * sizeof(ID);
		if (!firstTime && fileOff + size < (unsigned) (-1) / 8) {
			adjFile[fileindex]->write(size, (const char*) tempEnt->getBuffer());
			global_adj_index[2 * i] = fileOff;
			global_adj_index[2 * i + 1] = fileindex;
			fileOff += size;
		} else {
			if (fileindex >= 0) {
				adjFile[fileindex]->close();
			}

			fileindex++;
			fileOff = 0;
			adjFile.push_back(new TempFile(Dir + "/DAG", fileindex));
			DAG_start_ID[fileindex] = i;
			adjFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
			global_adj_index[2 * i] = 1;//actually is '0', but we set '1' here, for '0' has been used to mark the unexist 'from data'
			global_adj_index[2 * i + 1] = fileindex;
			fileOff += size;
			firstTime = false;
		}
	}
	//maxFromID = maxID;

	cout << "fileIndex:" << fileindex << ",  maxID " << maxID << endl;
	adjFile[fileindex]->close();
	global_adj_indexFile->flush();

	adjNum = adjFile.size();
	for(int i = 0;i < adjFile.size();i++){
		delete adjFile[i];
		adjFile[i] = NULL;
	}
	adjFile.clear();
	//delete adjFile;
	delete global_adj_indexFile;

	init_DAG();

	//-----------------------------------------------------���ڽӱ�------------------------------------------------------------
	assert(mappedIn.open(r_sortFile->getFile().c_str()));
	reader = mappedIn.getBegin(), limit = mappedIn.getEnd();

	firstInsert = true, firstTime = true;
	fileindex = -1;
	fileOff = 0;
	unsigned lastto = *(ID *)(reader + sizeof(ID));
	//EntityIDBuffer* tempEnt = new EntityIDBuffer();
	vector<TempFile*> r_adjFile;
	pageSize = vertex_estimate * sizeof(ID) / (MemoryBuffer::pagesize) + (((vertex_estimate * sizeof(ID)) % (MemoryBuffer::pagesize)) ? 1 : 0);
	char radj_indexPath[150];
	sprintf(radj_indexPath, "%s/DAG_reverse_global_adj_index.0", Dir.c_str());
	MMapBuffer *rglobal_adj_indexFile = new MMapBuffer(radj_indexPath, 2 * pageSize * MemoryBuffer::pagesize);
	unsigned *rglobal_adj_index = (unsigned *)rglobal_adj_indexFile->get_address();

	//from = 0, to = *(ID*) (reader + sizeof(ID));

	//to = *(ID*) reader;
	for(ID i = 0; i < to; i ++) {
		tempEnt->empty();
		tempEnt->insertID(i);
		tempEnt->insertID(0);

		num = tempEnt->getSize() - 2;
		tempEnt->getBuffer()[1] = num; //update num
		size = tempEnt->getSize() * sizeof(ID);
		if (!firstTime && fileOff + size < (unsigned) (-1) / 8) {
			r_adjFile[fileindex]->write(size, (const char*) tempEnt->getBuffer());
			rglobal_adj_index[2 * i] = fileOff;
			rglobal_adj_index[2 * i + 1] = fileindex;
			fileOff += size;
		} else {
			if (fileindex >= 0) {
				r_adjFile[fileindex]->close();
			}

			fileindex++;
			fileOff = 0;
			r_adjFile.push_back(new TempFile(Dir + "/DAG_reverse_adj", fileindex));
			//reverse_start_ID[fileindex] = i;
			r_adjFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
			rglobal_adj_index[2 * i] = 1;//actually is '0', but we set '1' here, for '0' has been used to mark the unexist 'from data'
			rglobal_adj_index[2 * i + 1] = fileindex;
			fileOff += size;
			firstTime = false;
		}
	}

	tempEnt->empty();

	while (reader < limit) {
		from = *(ID*) reader;
		reader += sizeof(ID);
		to = *(ID*) reader;
		reader += sizeof(ID);
		//cout<<"from:"<<from<<" to:"<<to<<endl;
		//cout<<"lastto:"<<lastto<<endl;

		//maxID = std::max(from, maxID);
		//maxID = std::max(to, maxID);
		//maxFromID = std::max(to, maxFromID);

		if (firstInsert) {
			tempEnt->insertID(to);
			tempEnt->insertID(0); // for num
			tempEnt->insertID(from);
			firstInsert = false;
		} else if (to != lastto) {
			unsigned num = tempEnt->getSize() - 2;
			tempEnt->getBuffer()[1] = num; //update num
			unsigned size = tempEnt->getSize() * sizeof(ID);
			if (!firstTime && fileOff + size < (unsigned) (-1) / 8) {
				r_adjFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
				rglobal_adj_index[2 * lastto] = fileOff;
				rglobal_adj_index[2 * lastto + 1] = fileindex;
				fileOff += size;
			} else {
				if (fileindex >= 0) {
					r_adjFile[fileindex]->close();
				}

				fileindex++;
				fileOff = 0;
				r_adjFile.push_back(new TempFile(Dir + "/DAG_reverse_adj", fileindex));
				//reverse_start_ID[fileindex] = lastto;
				r_adjFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
				rglobal_adj_index[2 * lastto] = 1;//actually is '0', but we set '1' here, for '0' has been used to mark the unexist 'to data'
				rglobal_adj_index[2 * lastto + 1] = fileindex;
				fileOff += size;
				firstTime = false;

				//cout << "fileIndex: " << fileindex << ", from minID " << to<< " to maxID " << maxID << endl;
			}
			//maxCount = max(maxCount, num);

			if(to != lastto + 1){
				for(ID i = lastto + 1; i < to; i ++) {
					//cout<<"i:"<<i<<endl;
					tempEnt->empty();
					tempEnt->insertID(i);
					tempEnt->insertID(0);

					unsigned num = tempEnt->getSize() - 2;
					tempEnt->getBuffer()[1] = num; //update num
					unsigned size = tempEnt->getSize() * sizeof(ID);
					if (!firstTime && fileOff + size < (unsigned) (-1) / 8) {
						r_adjFile[fileindex]->write(size, (const char*) tempEnt->getBuffer());
						rglobal_adj_index[2 * i] = fileOff;
						rglobal_adj_index[2 * i + 1] = fileindex;
						fileOff += size;
					} else {
						if (fileindex >= 0) {
							r_adjFile[fileindex]->close();
						}

						fileindex++;
						fileOff = 0;
						r_adjFile.push_back(new TempFile(Dir + "/DAG_reverse_adj", fileindex));
						//reverse_start_ID[fileindex] = i;
						r_adjFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
						rglobal_adj_index[2 * i] = 1;//actually is '0', but we set '1' here, for '0' has been used to mark the unexist 'from data'
						rglobal_adj_index[2 * i + 1] = fileindex;
						fileOff += size;
						firstTime = false;
					}
				}
			}

			tempEnt->empty();
			tempEnt->insertID(to);
			tempEnt->insertID(0);
			tempEnt->insertID(from);
			lastto = to;
		} else {
			tempEnt->insertID(from);
		}
	}

	num = tempEnt->getSize() - 2;
	tempEnt->getBuffer()[1] = num; //update num
	size = tempEnt->getSize() * sizeof(ID);
	if (!firstTime && fileOff + size < (unsigned) (-1) / 8) {
		r_adjFile[fileindex]->write(size, (const char*) tempEnt->getBuffer());
		rglobal_adj_index[2 * lastto] = fileOff;
		rglobal_adj_index[2 * lastto + 1] = fileindex;
		fileOff += size;
	} else {
		if (fileindex >= 0) {
			r_adjFile[fileindex]->close();
		}

		fileindex++;
		fileOff = 0;
		r_adjFile.push_back(new TempFile(Dir + "/DAG_reverse_adj", fileindex));
		//reverse_start_ID[fileindex] = lastto;
		r_adjFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
		rglobal_adj_index[2 * lastto] = 1;//actually is '0', but we set '1' here, for '0' has been used to mark the unexist 'from data'
		rglobal_adj_index[2 * lastto + 1] = fileindex;
		fileOff += size;
		firstTime = false;
	}
	//maxCount = max(maxCount, num);
	for(ID i = lastto + 1; i <= maxID; i ++) {
		//cout<<"i:"<<i<<endl;
		tempEnt->empty();
		tempEnt->insertID(i);
		tempEnt->insertID(0);

		unsigned num = tempEnt->getSize() - 2;
		tempEnt->getBuffer()[1] = num; //update num
		unsigned size = tempEnt->getSize() * sizeof(ID);
		if (!firstTime && fileOff + size < (unsigned) (-1) / 8) {
			r_adjFile[fileindex]->write(size, (const char*) tempEnt->getBuffer());
			rglobal_adj_index[2 * i] = fileOff;
			rglobal_adj_index[2 * i + 1] = fileindex;
			fileOff += size;
		} else {
			if (fileindex >= 0) {
				r_adjFile[fileindex]->close();
			}

			fileindex++;
			fileOff = 0;
			r_adjFile.push_back(new TempFile(Dir + "/DAG_reverse_adj", fileindex));
			//reverse_start_ID[fileindex] = i;
			r_adjFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
			rglobal_adj_index[2 * i] = 1;//actually is '0', but we set '1' here, for '0' has been used to mark the unexist 'from data'
			rglobal_adj_index[2 * i + 1] = fileindex;
			fileOff += size;
			firstTime = false;
		}
	}

	//maxFromID = maxID;
	//cout << "fileIndex:" << fileindex << ", maxID " << maxID << endl;

	r_adjFile[fileindex]->close();
	rglobal_adj_indexFile->flush();
	cout<<"success convert into reverse adjacency list"<<endl;

	//reverse_adjNum = r_adjFile.size();
	for(int i = 0;i < r_adjFile.size();i++){
		delete r_adjFile[i];
		r_adjFile[i] = NULL;
	}
	r_adjFile.clear();
	delete rglobal_adj_indexFile;

	init_reverse_DAG();

	mappedIn.close();
	r_sortFile->discard();
	delete r_sortFile;
	sortFile->discard();
	delete sortFile;

	delete tempEnt;


	//print_DAG();
	cout<<"readGraph end"<<endl;
	return OK;
}

void reach::init_DAG() {
	unsigned fileindex = 0;
	char sccPath[150];
	char scc_indexPath[150];
	//cout<<"fst if"<<endl;
	sprintf(sccPath, "%s/DAG.%d", Dir.c_str(), fileindex);

	//cout<<"while begin"<<endl;
	while (OSFile::FileExists(sccPath) == true) {
		DAGMap.push_back(new MMapBuffer(sccPath, 0));
		DAG.push_back(DAGMap[fileindex]->get_address());
		fileindex++;
		sprintf(sccPath, "%s/DAG.%d", Dir.c_str(), fileindex);
	}

	//cout<<"while end"<<endl;
	sprintf(scc_indexPath, "%s/global_DAG_index.0", Dir.c_str());
	global_DAG_indexMap = new MMapBuffer(scc_indexPath, 0);
	global_DAG_index = (unsigned *)global_DAG_indexMap->get_address();
}

void reach::init_reverse_DAG() {
	unsigned fileindex = 0;
	char sccPath[150];
	char scc_indexPath[150];
	//cout<<"fst if"<<endl;
	sprintf(sccPath, "%s/DAG_reverse_adj.%d", Dir.c_str(), fileindex);

	//cout<<"while begin"<<endl;
	while (OSFile::FileExists(sccPath) == true) {
		DAG_reverse_adjMap.push_back(new MMapBuffer(sccPath, 0));
		DAG_reverse_adj.push_back(DAG_reverse_adjMap[fileindex]->get_address());
		fileindex++;
		sprintf(sccPath, "%s/DAG_reverse_adj.%d", Dir.c_str(), fileindex);
	}

	//cout<<"while end"<<endl;
	sprintf(scc_indexPath, "%s/DAG_reverse_global_adj_index.0", Dir.c_str());
	DAG_reverse_global_adj_indexMap = new MMapBuffer(scc_indexPath, 0);
	DAG_reverse_global_adj_index = (unsigned *)DAG_reverse_global_adj_indexMap->get_address();
}

char* reach::DAG_getOffset(ID id){
	if (id < 0)return NULL;

	unsigned fileindex = global_DAG_index[2 * id + 1];
	unsigned adjOff = global_DAG_index[2 * id];
	if(adjOff == 0)return NULL; //'0' indicate unexist 'from or to data'
	if(adjOff == 1)adjOff = 0; //we modify the '1' back to '0' here
	char *temp = (DAG[fileindex] + adjOff);
	return temp;
}

char* reach::DAG_reverse_getOffset(ID id) {
	if (id < 0)return NULL;

	unsigned fileindex = DAG_reverse_global_adj_index[2 * id + 1];
	unsigned adjOff = DAG_reverse_global_adj_index[2 * id];
	if(adjOff == 0)return NULL; //'0' indicate unexist 'from or to data'
	if(adjOff == 1)adjOff = 0; //we modify the '1' back to '0' here
	char *temp = (DAG_reverse_adj[fileindex] + adjOff);
	return temp;
}

//bool reach::zeroDegree(ID id) {
//	if (!degree[id].indeg && !degree[id].outdeg)
//		return true;
//	else
//		return false;
//}
//
//bool reach::both_in_and_out_vertex(ID id) {
//	if (degree[id].indeg && degree[id].outdeg)
//		return true;
//	else
//		return false;
//}

size_t reach::loadFileinMemory(const char* filePath, char*& buf) {
	MemoryMappedFile temp;
	temp.open(filePath);
	size_t size = temp.getEnd() - temp.getBegin();
	assert(size);
	buf = (char*) malloc(size);
	assert(buf);
	memcpy(buf, temp.getBegin(), size);
	temp.close();
	return size;
}

void reach::parallel_load_task(char *&buf, MemoryMappedFile *&temp,
		unsigned long cur_pos, size_t copy_size) {
	memcpy(buf + cur_pos, temp->getBegin() + cur_pos, copy_size);
}

void reach::parallel_load_inmemory(const char* filePath, char*& buf) {
	MemoryMappedFile *temp = new MemoryMappedFile();
	temp->open(filePath);
	size_t size = temp->getEnd() - temp->getBegin();
	assert(size);
	buf = (char*) malloc(size);
	assert(buf);
	size_t task_num = size / MEMCPY_SIZE, left = size % MEMCPY_SIZE;
	unsigned long cur_pos = 0;

	for (size_t i = 0; i < task_num; i++) {
		CThreadPool::getInstance().AddTask(
				boost::bind(&reach::parallel_load_task, buf, temp, cur_pos, MEMCPY_SIZE));
		cur_pos += MEMCPY_SIZE;
	}
	if (left)
		CThreadPool::getInstance().AddTask(
				boost::bind(&reach::parallel_load_task, buf, temp, cur_pos, left));
	CThreadPool::getInstance().Wait();

	temp->close();
	delete temp;
}

int compquick(const void *a, const void *b) {
	return *(int *) a - *(int *) b;
}

static void writeId(TempFile *adjfile, unsigned id, char *ptr, size_t &index, bool &flag, size_t &offset)
{
	if ( flag == true ) {
		while (id >= 128) {
			if(index == TMP_BUF_SIZE){
				adjfile->direct_write(TMP_BUF_SIZE, (const char*)ptr);
				index = 0;
			}

			ptr[index] = static_cast<unsigned char> (id & 127);
			index++;
			assert(index <= TMP_BUF_SIZE);
			offset++;
			id >>= 7;
		}

		if(index == TMP_BUF_SIZE){
			adjfile->direct_write(TMP_BUF_SIZE, (const char*)ptr);
			index = 0;
		}

		ptr[index] = static_cast<unsigned char> (id & 127);
		index++;
		assert(index <= TMP_BUF_SIZE);
		offset++;
	} else {
		while (id >= 128) {
			if(index == TMP_BUF_SIZE){
				adjfile->direct_write(TMP_BUF_SIZE, (const char*)ptr);
				index = 0;
			}

			ptr[index] = static_cast<unsigned char> (id | 128);
			index++;
			assert(index <= TMP_BUF_SIZE);
			offset++;
			id >>= 7;
		}

		if(index == TMP_BUF_SIZE){
			adjfile->direct_write(TMP_BUF_SIZE, (const char*)ptr);
			index = 0;
		}

		ptr[index] = static_cast<unsigned char> (id | 128);
		index++;
		assert(index <= TMP_BUF_SIZE);
		offset++;
	}
	flag = !flag;
}

static void batch_write(FILE *flat_adjlist, FILE *vertex_pair, TempFile *adjfile, vector<unsigned> range, unsigned srcid, char *ptr, size_t &index, bool &flag, size_t &offset){
	size_t len = range.size() / 2;
	int low = 0, mid, high = len-1;
	while(low <= high){
		mid = (low+high)/2;
		if(range[2*mid] > srcid)high = mid-1;
		else if(range[2*mid] < srcid)low = mid+1;
		else break;
	}
	assert(range[2*mid] == srcid);//must find srcid, or error

	writeId(adjfile, range[2*mid], ptr, index, flag, offset);
	writeId(adjfile, range[2*mid+1]-range[2*mid], ptr, index, flag, offset);
#ifdef FLAT_ADJLIST
	//fprintf(flat_adjlist, "[%u, %u]: ", range[2*mid], range[2*mid+1]);
#endif
	int previous;
	bool first = true;
	for(size_t i = 0;i < len;i++){
		if(i != mid){
			if(first){
				writeId(adjfile, range[2*i], ptr, index, flag, offset);
				writeId(adjfile, range[2*i+1]-range[2*i], ptr, index, flag, offset);
				previous = 2*i+1;
				first = false;
			}else{
				writeId(adjfile, range[2*i]-range[previous], ptr, index, flag, offset);
				writeId(adjfile, range[2*i+1]-range[2*i], ptr, index, flag, offset);
				previous = 2*i+1;
			}
#ifdef FLAT_ADJLIST
			//fprintf(flat_adjlist, "[%u, %u] ", range[2*i], range[2*i+1]);
			/* random generate vertex pairs for test */
			if(range[2*mid+1] == range[2*mid])fprintf(vertex_pair, "%u ",range[2*mid]);
			//else fprintf(vertex_pair, "%u ",range[2*mid]+(random() % (range[2*mid+1]-range[2*mid]+1)));  /*choose this line, then all intersect*/
			else fprintf(vertex_pair, "%u ",1+range[2*mid]+(random() % (range[2*mid+1]-range[2*mid]+1)));  /*choose this line, then part intersect*/
			if(range[2*i+1] == range[2*i])fprintf(vertex_pair, "%u\n",range[2*i]);
			else fprintf(vertex_pair, "%u\n",range[2*i]+(random() % (range[2*i+1]-range[2*i]+1)));
			/* end of random generate */
#endif
		}
	}
#ifdef FLAT_ADJLIST
	//fprintf(flat_adjlist, "\n");
#endif
}

static void final_flush(TempFile *adjfile, char *ptr, size_t &index){
	adjfile->direct_write(index, (const char*)ptr);
	adjfile->flush();
}

void reach::get_backward_root_vertices(ID maxID) {
	EntityIDBuffer* buffer = new EntityIDBuffer();
	buffer->setIDCount(1);

	//cout<<"root:"<<endl;
	for (ID start = 0; start <= maxID; start++) {
		if (globalDegree[start].indeg && !globalDegree[start].outdeg) {
			buffer->insertID(start);
			//cout<<"start:"<<start<<" ";
		}
	}
	//cout<<endl;
	qsort((void*) buffer->getBuffer(), buffer->getSize(), 4, qcompareindegree21);
	if (buffer->getSize() == 0)buffer->insertID(0);
	cout << "DAG backward root number: " << buffer->getSize() << endl;
	TempFile* rootFile = new TempFile(Dir + "/DAGroot.backward", 0);
	rootFile->write((size_t) buffer->getSize() * sizeof(ID),(const char*) buffer->getBuffer());
	rootFile->flush();
	delete rootFile;
	cout << "finish writing DAGrootFile" << endl;
	delete buffer;
}

ID reach::tranverse(ID x, ID y) {
	bool *visit = (bool *)calloc(maxID + 1, sizeof(bool));
	memset(visit, false, sizeof(bool) * (maxID + 1));
	deque<ID>* que = new deque<ID>();
	if(! visit[x]){
		que->push_back(x);
		visit[x] = true;
		while(!que->empty()) {
			ID curID = que->front();
			que->pop_front();

			ID *addr = (ID*) DAG_getOffset(curID);
			assert(addr[0] == curID);
			unsigned count = addr[1];
			addr += 2;
			for(unsigned i = 0; i < count; i ++) {
				if(addr[i] == y)
					return 1;
				else if(! visit[addr[i]]) {
					que->push_back(addr[i]);
					visit[addr[i]] = true;
				}
			}
		}
	}
	return 0;
}

void reach::generate_querys(string path){
	 srand((unsigned int)time(NULL));
	 ID min = 0, max = maxID;
	 unsigned num = 100000000;
	 ID *random_x = (ID *)calloc(num, sizeof(ID));
	 ID *random_y = (ID *)calloc(num, sizeof(ID));
	 ofstream outfile(path.c_str());
	 if(!outfile){
		 cout<<"unable to open file!"<<endl;
		 exit(0);
	 }
	 for (size_t i = 0; i < num; i++) {
		 random_x[i] = rand() % (max - min + 1) + min;
		 random_y[i] = rand() % (max - min + 1) + min;
		 outfile<<random_x[i]<<" "<<random_y[i]<<" "<<-1<<"\n";
//		 while (random_x[i] == random_y[i]) {
//			 random_y[i] = rand() % (max - min + 1) + min;
//		 }
	}
	 outfile.close();
}

bool reach::same_layer_tree_reach(ID id1, ID id2){
	//reach_flag[id1] = querycnt;
	if(rlt[id1] != rlt[id2]){
		return false;
	}
	else{
		if(interval[id1] >= interval[id2]){
			return true;
		}
		else {
//			int l = 0, r = excpID[id1].size() - 1;
//			if(l > r || excpID[id1][l] > id2) return false;
//			do {
//				int m = l + (r-l)/2;
//				if(excpID[id1][m] == id2)return true;
//				else if(excpID[id1][m] < id2) l = m + 1;
//				else r = m - 1;
//			}while(l <= r);

			int len = excpID[id1].size();
			for(int i = 0; i < len; i ++) {
				if(excpID[id1][i] > id2)
					return false;
				else if(interval[excpID[id1][i]] >= interval[id2]) {
					return true;
				}
			} //for

			return false;
		}
	}
}

bool reach::down_layer_to_up_layer_without_cross_located(ID id1, ID id2){
	//reach_flag[id1] = querycnt;
	int len = id_cross_arr[id1].size();
//	if(len == 0){
//		return false;
//	}

//	if(//left_to_right[id1] > left_to_right[id2] ||
//			nor_interval[id1] < nor_interval[id2])
//		return false;

//	if(rev_oldID_mapto_newID[id1] < rev_oldID_mapto_newID[id2])
//		return false;

	for(int i = 0; i < len; i ++){
		if(id_cross_arr[id1][i] > id2)
			return false;
//		if(rl[id_cross_arr[id1][i]] > rl[id2]){
//			return false;
//		}
		else if(rl[id_cross_arr[id1][i]] < rl[id2]){
			querycnt ++;
			if(//reach_flag[id_cross_arr[id1][i]] != querycnt &&
					down_layer_to_up_layer_without_cross_located(id_cross_arr[id1][i], id2))
				return true;
		}
		else {
			if(//reach_flag[id_cross_arr[id1][i]] != querycnt &&
					same_layer_tree_reach(id_cross_arr[id1][i], id2)){
				return true;
			}
		}
	}
	return false;
}

bool reach::query(ID id1, ID id2){
//	if(rl[id1] > rl[id2])
//		return false;
//	else
	if(rl[id1] < rl[id2]){
//		if(rev_rl[id1] < rev_rl[id2])
//			return false;
//		else if(rev_rl[id1] == rev_rl[id2] && rev_rlt[id1] != rev_rlt[id2])
//			return false;
//		else
		return down_layer_to_up_layer_without_cross_located(id1, id2);
	}
	else {
		return same_layer_tree_reach(id1, id2);
	}
	return false;
}

void reach::process(char *testfilename) {
	struct timeval start_time, end_time;

	cout<<"process index construction:"<<endl;
	gettimeofday(&start_time, NULL);
	construct_index();
	gettimeofday(&end_time, NULL);
//	cout<<"index construction build time elapse:"<<(end_time.tv_sec - start_time.tv_sec) * 1000 +
//			(end_time.tv_usec - start_time.tv_usec) * 1.0/ 1000.0<<" ms"<<endl;

	cout << "preparing queries..." << endl;
	int label;
	ID s , t;
	vector<ID> src;
	vector<ID> dest;
	vector<ID> labels;

	ifstream in(testfilename);
	string buf;
	//getline(in, buf);
	std::istringstream iss(buf, std::istringstream::in);

	while (getline(in, buf)) {
		iss.clear();
		iss.str(buf);
		iss >> s;
		iss >> t;
		iss >> label;
        src.push_back(s);
        dest.push_back(t);
        labels.push_back(label);
	}

//	int size = 0;
//	if(testfilename!=NULL){
//		std::ifstream fstr(testfilename);
//		while(!fstr.eof() && size < 1000000-1) {
//			size ++;
//        	fstr >> s >> t >> label;
//        	src.push_back(s);
//        	dest.push_back(t);
//        	labels.push_back(label);
//		}
//	}
	cout << "queries are ready" << endl;

	// process queries
	cout << "process queries..." << endl;
	bool r;

	//memset(reach_flag, 0, sizeof(ID) * (maxID+1));
	//querycnt = 0;

	cout<<"query num:"<<src.size()<<endl;
	querycnt = 0;
	gettimeofday(&start_time, NULL);
	for(vector<ID>::iterator x=src.begin(), y=dest.begin(); x!=src.end(); x++, y++) {
		//ID id1 = *x;
		//ID id2 = *y;
		if(*x == *y)
			r = true;
		else if(Ldown[*x] >= Ldown[*y] || Lup[*x] <= Lup[*y])
			r = false;
		else{
			//querycnt ++;
			if(oldID_mapto_newID[*x] > oldID_mapto_newID[*y])
				r = false;
			else r = query(oldID_mapto_newID[*x], oldID_mapto_newID[*y]);
		}
	}
	gettimeofday(&end_time, NULL);
	cout<<"reachability build time elapse:"<<(end_time.tv_sec - start_time.tv_sec) * 1000.0 +
			(end_time.tv_usec - start_time.tv_usec)*1.0/ 1000.0<<" ms"<<endl;
//	cout<<"traverse num:"<<querycnt<<" average: "<<(double)querycnt/(maxID+1)<<endl;

//	gettimeofday(&start_time, NULL);
//	for(vector<ID>::iterator x=src.begin(), y=dest.begin(); x!=src.end(); x++, y++) {
//		ID id1 = *x;
//		ID id2 = *y;
//		if(id1 == id2)
//			r = true;
//		else if(Ldown[id1] >= Ldown[id2] || Lup[id1] <= Lup[id2])
//			r = false;
//		else {
//			//querycnt ++;
//			if(oldID_mapto_newID[id1] > oldID_mapto_newID[id2])
//				r = false;
//			else r = query(oldID_mapto_newID[id1], oldID_mapto_newID[id2]);
//		}
//	}
//	gettimeofday(&end_time, NULL);
//	cout<<"reachability build time elapse:"<<(end_time.tv_sec - start_time.tv_sec) * 1000.0 +
//			(end_time.tv_usec - start_time.tv_usec)*1.0/ 1000.0<<" ms"<<endl;
//
//
//	int tf = 0, tt = 0;
//	int reachable = 0, nonreachable =0;
//	int success = 0, fail = 0;
//	//vector<int> fail_ans;
//	gettimeofday(&start_time, NULL);
//	for(vector<ID>::iterator x=src.begin(), y=dest.begin(), l = labels.begin(); x!=src.end(); x++, y++, l++) {
//		ID id1 = *x;
//		ID id2 = *y;
//		if(id1 == id2)
//			r = true;
//		else if(Ldown[id1] >= Ldown[id2] || Lup[id1] <= Lup[id2])
//			r = false;
//		//else if(oldID_mapto_newID[id1][0] > oldID_mapto_newID[id2][0])
//			//r = false;
//		else {
//			//querycnt ++;
//			if(oldID_mapto_newID[id1] > oldID_mapto_newID[id2])
//				r = false;
//			else r = query(oldID_mapto_newID[id1], oldID_mapto_newID[id2]);
//		}
//
//		if(r == true){
//			reachable ++;
//			if(*l == 0) {
//				fail ++;
//				tf ++;
//				//cout<<id1<<" "<<id2<<":"<<*l<<endl;
//				//fail_ans.push_back(id1);
//				//fail_ans.push_back(id2);
//				//fail_ans.push_back(*l);
//			}
//			else {
//				tt ++;
//				success ++;
//			}
//		}
//		else {
//			nonreachable ++;
//			if(*l == 1) {
//				fail ++;
//				//cout<<id1<<" "<<id2<<":"<<*l<<endl;
//				//fail_ans.push_back(id1);
//				//fail_ans.push_back(id2);
//				//fail_ans.push_back(*l);
//			}
//			else success ++;
//		}
//	}
//	gettimeofday(&end_time, NULL);
//	cout<<"reachability build time elapse:"<<(end_time.tv_sec - start_time.tv_sec) * 1000.0 +
//			(end_time.tv_usec - start_time.tv_usec)*1.0/ 1000.0<<" ms"<<endl;
//
//	cout << "Success Rate " << success << "/" << success+fail << endl;
//	cout<<"0:"<<"true:"<<success - tt<<"  false:"<<fail - tf<<endl;
//	cout<<"1:"<<" true:"<<tt<<"  false:"<<tf<<endl;
//	cout<<"reach:"<<reachable<<"  nonreach:"<<nonreachable<<endl;

//	for(int i = 0; i < fail_ans.size(); i ++){
//		ID id1 = fail_ans[i];
//		i ++;
//		ID id2 = fail_ans[i];
//		i ++;
//		ID r = fail_ans[i];
//		cout<<"id:"<<id1<<" "<<id2<<":"<<r<<endl;
//		query(id1, id2);
//	}

	int totalsize = 4 * (maxID + 1);
	for(int i = 1; i <= maxID+1; i ++){
		totalsize += excpID[i].size() + id_cross_arr[i].size();// + oldID_mapto_newID[i].size();// + cross_located[i].size();
	}
	totalsize += oldID_mapto_newID_count + temp_root_arr.size();
//	cout<<"totalsize:"<<totalsize<<" every vertex size:"<<(float)totalsize/(maxID+1)<<endl;
	cout<<"index size (MB): "<<(double)totalsize*4/(1024*1024)<<endl;

//	totalsize = 0;
//	int totalrootexcpsize = 0;
//	for(int i = 1; i <= maxID+1; i ++){
//		totalsize += excpID[i].size();
//		//if(rlt[i] == i + 1)
//		//	totalrootexcpsize += excpID[i].size();
//	}
//	cout<<"excp total size:"<<totalsize<<" totalrootexcpsize:"<<totalrootexcpsize<<endl;
//	cout<<"every vertex excp size:"<<(float)totalsize/(maxID+1)<<endl;
//	totalsize = 0;
//	totalrootexcpsize = 0;
//	for(int i = 0; i <= maxID; i ++){
//		totalsize += id_cross_arr[i].size();
//		//if(root_located[i] == i + 1)
//		//	totalrootexcpsize += id_cross_arr[i].size();
//	}
//	cout<<"cross total size:"<<totalsize<<" totalrootcrosssize:"<<totalrootexcpsize<<endl;
//	cout<<"every vertex cross size:"<<(float)totalsize/(maxID+1)<<endl;

	cout<<"total root layer:"<<oldID_mapto_newID_count<<endl;

	free(root_located);
	root_located = NULL;

	free(Lup);
	Lup = NULL;

	free(Ldown);
	Ldown = NULL;

	free(interval);
	interval = NULL;

	free(oldID_mapto_newID);
	oldID_mapto_newID = NULL;

	free(root_layer);
	root_layer = NULL;

	free(rl);
	rl = NULL;
	free(rlt);
	rlt = NULL;

	vector <ID>().swap(temp_root_arr);
	vector< vector <ID> >().swap(id_cross_arr);
	vector< vector <ID> >().swap(excpID);
}

void reach::pre_process(bool *&cross_vertices_arr, unsigned *&nextInDegree){
	//TopoOrder.resize(maxID + 1);

	int i = 0, k = TopoOrder.size();
	unsigned layer = 1;
	//ID component;

	bool flag=true;
	int vectorsize = k;

	while(flag){
		flag=false;
		layer++;
		//cout<<"vectorsize:"<<vectorsize<<endl;
		for(; i<vectorsize; i++){
			ID u = TopoOrder[i];
			//cout<<"u:"<<u<<endl;
			ID *addr = (ID*) DAG_getOffset(u);
			unsigned count = addr[1];
			addr += 2; //skip the id and count

			for(int j=0; j<count; j++){
				globalDegree[addr[j]].indeg --;

				if(cross_vertices_arr[u]){
					cross_vertices_arr[addr[j]] = true;
					root_located[addr[j]] = 0; // add

					nextInDegree[addr[j]] ++;
					//degree[addr[j]] = 0;
				}

				else if(root_located[addr[j]] != 0){// && root_located[addr[j]] != root_located[u]){
					if(root_located[addr[j]] != root_located[u] && !cross_vertices_arr[addr[j]]){
						cross_vertices_arr[addr[j]] = true;
						cross_id.push_back(addr[j]);

						root_located[addr[j]] = 0; // add
						//degree[addr[j]] = 0;
					}
				}
				else if(!cross_vertices_arr[addr[j]]){ //add
					root_located[addr[j]] = root_located[u];
					//root_layer[addr[j]] = oldID_mapto_newID_count;
				}

				if(globalDegree[addr[j]].indeg== 0){
					TopoOrder.push_back(addr[j]);
					Ldown[addr[j]]=layer;
					k++;
				}
			}
		}

		if(vectorsize!=k){
			vectorsize=k;
			flag=true;
		}
	}

	//cout<<"TopoOrder:"<<TopoOrder.size()<<" "<<"cross size:"<<cross_id.size()<<endl;

}

void reach::pre_process_cross(bool *&cross_vertices_arr, unsigned *&curInDegree, unsigned *&nextInDegree){
	int i = 0, k = TopoOrder_cross.size();

	bool flag=true;
	int vectorsize = k;
	cross_id.clear();
	while(flag){
		flag=false;
		//cout<<"vectorsize:"<<vectorsize<<endl;
		for(; i<vectorsize; i++){
			ID u = TopoOrder_cross[i];
			//cout<<"u:"<<u<<endl;
			ID *addr = (ID*) DAG_getOffset(u);
			unsigned count = addr[1];
			addr += 2; //skip the id and count

			for(int j=0; j<count; j++){
				//if(globalDegree[addr[i]].indeg)
				curInDegree[addr[j]] --;
				//degree[addr[j]].indeg ++;
				//cout<<addr[j]<<" root_lcated:"<<root_located[addr[j]]<<endl;

				if(cross_vertices_arr[u]){
				    cross_vertices_arr[addr[j]] = true;
					root_located[addr[j]] = 0; // add

					nextInDegree[addr[j]] ++;
					//degree[addr[j]].indeg = 0;
				}

				else if(root_located[addr[j]] != 0){// && root_located[addr[j]] != root_located[u]){
					if(root_located[addr[j]] != root_located[u] && !cross_vertices_arr[addr[j]]){
						cross_vertices_arr[addr[j]] = true;
						cross_id.push_back(addr[j]);

						root_located[addr[j]] = 0; // add
						//degree[addr[j]].indeg = 0;
					}
				}
				else if(!cross_vertices_arr[addr[j]]){ //add
					root_located[addr[j]] = root_located[u];
					//root_layer[addr[j]] = oldID_mapto_newID_count;
				}

				if(curInDegree[addr[j]] == 0){
					TopoOrder_cross.push_back(addr[j]);
					k++;
				}
			}
		}

		if(vectorsize!=k){
			vectorsize=k;
			flag=true;
		}
	}

	//cout<<"TopoOrder:"<<TopoOrder_cross.size()<<" "<<"cross size:"<<cross_id.size()<<endl;
}

void reach::cal_cross_degree(bool *&visited_arr, unsigned *&curInDegree){//, unsigned *&nextInDegree){
	int len = temp_root_arr.size();
	deque<ID>* que = new deque<ID>();
	for(int i = 0; i < len; i ++){
		que->push_front(temp_root_arr[i]);
		while(!que->empty()){
			ID curID = que->front();
			que->pop_front();
			ID *addr = (ID*) DAG_getOffset(curID);
			unsigned count = addr[1];
			addr += 2;
			//nextInDegree[curID] = count;
			for(int j = 0; j < count; j ++){
				if(root_located[addr[j]] == root_located[curID]){
					curInDegree[addr[j]]++;
					if(!visited_arr[addr[j]]){
						visited_arr[addr[j]] = true;
						que->push_back(addr[j]);
					}
				}
			}
		}
	}
	delete que;
	que = NULL;
}

void reach::decode(unsigned *&curInDegree){
	deque<ID>* que = new deque<ID>();

	int rootNum = temp_root_arr.size();
	idcount = 0;
	for(int index = 0;index < rootNum; index ++){
		que->push_front(temp_root_arr[index]);

		while(!que->empty()){
			ID curID = que->front();
			que->pop_front();

			idcount++;
			oldID_mapto_newID[curID] = idcount;
			rl[idcount] = root_layer[temp_root_arr[index]];
			rlt[idcount] = root_located[curID];

			ID *addr = (ID*) DAG_getOffset(curID);
			unsigned count = addr[1];
			addr += 2; //skip the id and count

			for (size_t i = 0; i < count; i++) {
				if(root_located[addr[i]] == root_located[curID]){
					if(curInDegree[addr[i]]){
						curInDegree[addr[i]] --;
						//globalDegree[addr[i]].indeg ++;
					}
					if(curInDegree[addr[i]]){
						//if(root_located[addr[i]] == temp_root_arr[index] + 1)
							excpID_O[curID].push_back(addr[i]);
						continue;
					}//degree if
					que->push_front(addr[i]);
				}
			}//for
		} //while
	} //for

	delete que;
	que = NULL;
}

void reach::construct_index(){
	get_forward_root_vertices(maxID);

	MemoryMappedFile *rootMap = new MemoryMappedFile();
	assert(rootMap->open((Dir+"/DAGroot.forward.0").c_str()));
	unsigned rootNum = (rootMap->getEnd() - rootMap->getBegin()) / sizeof(ID);
	ID *root_arr = (ID*) rootMap->getBegin();
	if (rootNum > 0)cout << "root:" << *(root_arr) << "  rootNum:" << rootNum << endl;

//	cout<<"processing..."<<endl;
	struct timeval start_time, end_time;
	gettimeofday(&start_time, NULL);

	unsigned* curInDegree =  (unsigned *)calloc((maxID + 1), sizeof(unsigned));
	unsigned* nextInDegree = (unsigned *)calloc((maxID + 1), sizeof(unsigned));

	Ldown = (unsigned *)calloc((maxID + 1), sizeof(unsigned));
	root_located = (ID *)calloc(maxID + 1, sizeof(ID));
	root_layer = (ID *)calloc(maxID + 1, sizeof(ID));
	rl = (ID *)calloc(maxID + 2, sizeof(ID));
	rlt = (ID *)calloc(maxID + 2, sizeof(ID));
	oldID_mapto_newID_count = 1;

//	string fileName2 = string(path+"/cross_vertices");
//	MMapBuffer *cross_vertices = new MMapBuffer(fileName2.c_str(), (maxID+1) * sizeof(bool));
//	bool *cross_vertices_arr = (bool *)(cross_vertices->get_address());
	bool *cross_vertices_arr = (bool *)calloc(maxID + 1, sizeof(bool));

//	vector<int> r;
//	r.push_back(rootNum);
	for(int i = 0; i < rootNum; i ++){
		temp_root_arr.push_back(root_arr[i]);

		Ldown[root_arr[i]] = 1;
		TopoOrder.push_back(root_arr[i]);
		root_located[root_arr[i]] = root_arr[i] + 1;

		root_layer[root_arr[i]] = oldID_mapto_newID_count;
	}

	memset(cross_vertices_arr, false, sizeof(bool) * (maxID+1));
	pre_process(cross_vertices_arr, curInDegree);

	// deal with cross
//	int cur = maxID + 1;
//	int next;
//	vector<int>num;
	while(cross_id.size() != 0){
		TopoOrder_cross.clear();
		oldID_mapto_newID_count++;

		for(unsigned i = 0; i < cross_id.size(); i ++){
			if(curInDegree[cross_id[i]] == 0/* && degree[cross_id[i]].outdeg != 0*/){
				temp_root_arr.push_back(cross_id[i]);
				TopoOrder_cross.push_back(cross_id[i]);
				root_located[cross_id[i]] = cross_id[i] + 1;

				root_layer[cross_id[i]] = oldID_mapto_newID_count;
			}
		}
//		r.push_back(TopoOrder_cross.size());
		memset(cross_vertices_arr, false, sizeof(bool) * (maxID+1));
		pre_process_cross(cross_vertices_arr, curInDegree, nextInDegree);
		unsigned* temp = curInDegree;
		curInDegree = nextInDegree;
		nextInDegree = temp;

//		next = TopoOrder_cross.size();
//		num.push_back(cur-next);
//		cur = next;
	}

	gettimeofday(&end_time, NULL);
	double t1 = (end_time.tv_sec - start_time.tv_sec) * 1000 +
			(end_time.tv_usec - start_time.tv_usec) * 1.0/ 1000.0;
//	cout<<"pre process time:"<<t1<<" ms"<<endl;

//	int cnt = 0;
//	for(int i = 0; i < num.size(); i ++){
//		cout<<"the "<<i + 1<<" layer number: "<<num[i]<<endl;
//		cout<<"the non-shared graph number: "<<r[i]<<endl;
//		cnt = cnt + num[i];
//	}
//	cout<<"the "<<num.size() + 1<<" layer number: "<<maxID+1-cnt<<endl;
//	cout<<"the non-shared graph number: "<<r[r.size()-1]<<endl;

	gettimeofday(&start_time, NULL);

	//left_to_right = (ID *)calloc(maxID + 1, sizeof(ID));
	//right_to_left = (ID *)calloc((maxID + 1), sizeof(ID));
	//code(root_arr, rootNum, curInDegree);
	//degree = curInDegree;

	memset(cross_vertices_arr, false, sizeof(bool) * (maxID + 1));
	cal_cross_degree(cross_vertices_arr, curInDegree);//, nextInDegree);

	gettimeofday(&end_time, NULL);
	double t2 = (end_time.tv_sec - start_time.tv_sec) * 1000 +
			(end_time.tv_usec - start_time.tv_usec) * 1.0/ 1000.0;
//	cout<<"cal_cross_degree time:"<<t2<<"ms"<<endl;

	gettimeofday(&start_time, NULL);

	oldID_mapto_newID = (ID *)calloc(maxID + 1, sizeof(ID));
	excpID_O = vector < vector <ID> >(maxID + 1);
	decode(curInDegree);

	gettimeofday(&end_time, NULL);
	double t3 = (end_time.tv_sec - start_time.tv_sec) * 1000 +
			(end_time.tv_usec - start_time.tv_usec) * 1.0/ 1000.0;
//	cout<<"decode time:"<<t3<<"ms"<<endl;

//	cout<<"root size:"<<temp_root_arr.size()<<" rate:"<<(float)temp_root_arr.size()/(maxID+1)<<endl;

	gettimeofday(&start_time, NULL);
	Lup = (unsigned *)calloc((maxID + 1), sizeof(unsigned));
	interval = (ID *)calloc(maxID + 2, sizeof(ID));
	//nor_interval = (ID *)calloc(maxID + 2, sizeof(ID));

	cal_index();

	gettimeofday(&end_time, NULL);
	double t4 = (end_time.tv_sec - start_time.tv_sec) * 1000 +
			(end_time.tv_usec - start_time.tv_usec) * 1.0/ 1000.0;
//	cout<<"cal_interval time:"<<t4<<" ms"<<endl;

	gettimeofday(&start_time, NULL);
	excpID = vector < vector <ID> >(maxID + 2);
	id_cross_arr = vector < vector <ID> >(maxID + 2);
	id_cross_arr_pro = vector < vector <ID> >(maxID + 2);

	//temp_root_arr.clear();
	cal_excp_and_cross_arr();
	gettimeofday(&end_time, NULL);
	double t5 = (end_time.tv_sec - start_time.tv_sec) * 1000 +
			(end_time.tv_usec - start_time.tv_usec) * 1.0/ 1000.0;
//	cout<<"cal_excp_and_cross_arr time:"<<t5<<" ms"<<endl;

	cout<<"construction total time :"<<t1 + t2 + t4 + t5 + t3<<" ms"<<endl;

//	for(size_t index = 0;index <= maxID;index++){
//		//cout<<index<<" cross:"<<cross_located[index][cross_located_count]
//		cout<<index<<"  root:"<<root_located[index]<<" "<<rlt[oldID_mapto_newID[index]];
//		cout<<" layer:"<<rl[oldID_mapto_newID[index]]<<endl;
//	}
//	for(int i = 0; i <= maxID; i ++)
//		cout<<i<<"newID:"<<oldID_mapto_newID[i]<<":"<<interval[oldID_mapto_newID[i]]<<endl;
//	for(size_t index = 0;index <= maxID;index++){
//		cout<<index<<":"<<endl;
//
//		cout<<" excp:";
//		for(unsigned i = 0; i < excpID[oldID_mapto_newID[index]].size(); i ++) {
//			cout<<excpID[oldID_mapto_newID[index]][i]<<" ";
//		}
//		cout<<endl;
//
//		cout<<"id_cross_arr:";
//		for(unsigned i = 0; i < id_cross_arr[oldID_mapto_newID[index]].size(); i ++) {
//			cout<<id_cross_arr[oldID_mapto_newID[index]][i]<<" ";
//		}
//		cout<<endl;
//	}

//	gettimeofday(&start_time, NULL);
//	memset(root_located, 0, sizeof(ID) * (maxID+1));
//	rev_oldID_mapto_newID = (ID*) calloc(maxID+2, sizeof(ID));
//	rev_rl = (ID*) calloc(maxID+2, sizeof(ID));
//	rev_rlt = (ID*) calloc(maxID+2, sizeof(ID));
//	oldID_mapto_newID_count = 1;
//	TopoOrder_cross.clear();
//	for(int i = 0; i < temp_root_arr.size(); i ++){
//		TopoOrder_cross.push_back(temp_root_arr[i]);
//		root_located[temp_root_arr[i]] = temp_root_arr[i] + 1;
//
//		rev_rl[oldID_mapto_newID[temp_root_arr[i]]] = oldID_mapto_newID_count;
//	}
//
//	memset(cross_vertices_arr, false, sizeof(bool) * (maxID+1));
//	rev_pre_process(cross_vertices_arr, nextInDegree, curInDegree);
//	while(cross_id.size() != 0){
//		TopoOrder_cross.clear();
//		oldID_mapto_newID_count++;
//
//		for(unsigned i = 0; i < cross_id.size(); i ++){
//			if(curInDegree[cross_id[i]] == 0/* && degree[cross_id[i]].outdeg != 0*/){
//				temp_root_arr.push_back(cross_id[i]);
//				TopoOrder_cross.push_back(cross_id[i]);
//				root_located[cross_id[i]] = cross_id[i] + 1;
//
//				rev_rl[oldID_mapto_newID[cross_id[i]]] = oldID_mapto_newID_count;
//			}
//		}
//		memset(cross_vertices_arr, false, sizeof(bool) * (maxID+1));
//		rev_pre_process(cross_vertices_arr, curInDegree, nextInDegree);
//		unsigned* temp = curInDegree;
//		curInDegree = nextInDegree;
//		nextInDegree = temp;
//	}
//
//	memset(cross_vertices_arr, false, sizeof(bool) * (maxID + 1));
//	cal_rev_cross_degree(cross_vertices_arr, curInDegree);
//	rev_decode(curInDegree);
//
//	gettimeofday(&end_time, NULL);
//	double t6 = (end_time.tv_sec - start_time.tv_sec) * 1000 +
//			(end_time.tv_usec - start_time.tv_usec) * 1.0/ 1000.0;
//	cout<<"reverse time:"<<t6<<" ms"<<endl;
//
////	for(size_t index = 0;index <= maxID;index++){
////		//cout<<index<<" cross:"<<cross_located[index][cross_located_count]
////		cout<<index<<"rev newID:"<<rev_oldID_mapto_newID[index];
////		cout<<" layer:"<<rev_rl[oldID_mapto_newID[index]]<<endl;
////	}
//
//	cout<<"total construction total time:"<<t1 + t2 + t4 + t5 + t3 + t6<<endl;
//
//	cout<<"root size:"<<temp_root_arr.size()<<" rate:"<<(float)temp_root_arr.size()/(maxID+1)<<endl;

	//vector <ID>().swap(temp_root_arr);
	vector< vector <ID> >().swap(id_cross_arr_pro);
	vector< vector <ID> >().swap(excpID_O);
	rootMap->close();
	delete rootMap;
	//free(root_arr);
	free(curInDegree);
	free(nextInDegree);
	//remove(fileName2.c_str());
	//delete cross_vertices;
	free(reach_flag);
	reach_flag = NULL;

	free(cross_vertices_arr);
	cross_vertices_arr = NULL;

	free(globalDegree);
	globalDegree = NULL;
}

bool reach::is_excp(ID id1, ID id2, int f , int l) {
	//int f = 0, l = excpID_O[id1].size() - 1;
	while(f <= l){
		int m = (f + l)/2;
		if(excpID_O[id1][m] == id2)return true;
		else if(excpID_O[id1][m] < id2)f = m + 1;
		else l = m - 1;
	}
	return false;
}

void reach::cal_index(){
	//cout<<"topoorder size:"<<TopoOrder.size()<<endl;
	for(int i = TopoOrder.size() - 1; i >= 0; i--){
		ID u = TopoOrder[i];
		//cout<<"u:"<<u<<endl;
		ID *addr = (ID*) DAG_getOffset(u);
		unsigned count = addr[1];
		addr += 2; //skip the id and count

		unsigned tmpLup=0, tmpInterval = oldID_mapto_newID[u];
		int right = excpID_O[u].size() - 1;
		//ID temp_nor = oldID_mapto_newID[u];
		for(int j=0; j<count; j++){
			//Lup
			if( Lup[addr[j]] > tmpLup){
				tmpLup = Lup[addr[j]];
			}

			//if(temp_nor < nor_interval[oldID_mapto_newID[addr[j]]])
			//	temp_nor = nor_interval[oldID_mapto_newID[addr[j]]];

			if(root_located[addr[j]] == root_located[u]) {
				//interval
				if((right < 0 || !is_excp(u, addr[j], 0, right)) && tmpInterval < interval[oldID_mapto_newID[addr[j]]])
					tmpInterval = interval[oldID_mapto_newID[addr[j]]];
			}
			//else //if(cross_located[u][cross_located_count] == 0)
				//id_cross_arr_pro[u].push_back(addr[j]);
		}
		Lup[u] = tmpLup+1;
		interval[oldID_mapto_newID[u]] = tmpInterval;
		//nor_interval[oldID_mapto_newID[u]] = temp_nor;
	}
}

//bool cmpNewID( ID a, ID b){//from small to large
//	if(oldID_mapto_newID[a] < oldID_mapto_newID[b])
//		return 1;
//	else return 0;
//}
//
//
//bool cmp_cross_id( ID a, ID b){//from small to large
////	if(root_layer[a] < root_layer[b]){
////		return 1;
////	}
////	else if(root_layer[a] == root_layer[b]){
//		if(oldID_mapto_newID[a] < oldID_mapto_newID[b]) return 1;
//		else return 0;
////	}
////	else {
////		return 0;
////	}
//}

void reach::cal_excp_and_cross_arr(){
	reach_flag = (ID *)calloc((maxID + 2), sizeof(ID));
	ID* exit_flag = (ID *)calloc((maxID + 2), sizeof(ID));
	querycnt = -1;
	//bool flag;
	for(int i = TopoOrder.size() - 1; i >= 0; i--){
		ID u = TopoOrder[i];
		ID u_newID = oldID_mapto_newID[u];
		ID *addr = (ID*) DAG_getOffset(u);
		unsigned count = addr[1];
		addr += 2; //skip the id and count

		//if(!count)temp_root_arr.push_back(u);

		querycnt += 2;
		int size = excpID_O[u].size();
		//int cnt = 0;
		for(int n = 0; n < size; n ++){
			ID newID = oldID_mapto_newID[excpID_O[u][n]];
			if(interval[u_newID] < interval[newID] /*&& exit_flag[newID] != querycnt*/){
				excpID[u_newID].push_back(newID);
				exit_flag[newID] = querycnt;
			}
		}

		for(int j=0; j<count; j++){
			int len = id_cross_arr_pro[u_newID].size();
			ID adj_newID = oldID_mapto_newID[addr[j]];
			if(rlt[adj_newID] == rlt[u_newID]) {
				//excpID
				size = excpID[adj_newID].size();
				for(unsigned n = 0; n < size; n ++){
					if(interval[u_newID] < interval[excpID[adj_newID][n]] &&
							exit_flag[excpID[adj_newID][n]] != querycnt){
						excpID[u_newID].push_back(excpID[adj_newID][n]);
						exit_flag[excpID[adj_newID][n]] = querycnt;
					}
				}
				size = id_cross_arr[adj_newID].size();
				//int len = id_cross_arr_pro[u].size();
				for(unsigned n = 0; n < size; n ++){
					if(exit_flag[id_cross_arr[adj_newID][n]] == querycnt + 1 ||
							reach_flag[id_cross_arr[adj_newID][n]] == querycnt)
						continue;

					if(!is_reachable(u_newID, id_cross_arr[adj_newID][n], len)){

						id_cross_arr_pro[u_newID].push_back(id_cross_arr[adj_newID][n]);
						exit_flag[id_cross_arr[adj_newID][n]] = querycnt + 1;
					}
				}
			}
			else {
				if(exit_flag[adj_newID] != querycnt + 1 && reach_flag[adj_newID] != querycnt &&
						!is_reachable(u_newID, adj_newID, len)){
					id_cross_arr_pro[u_newID].push_back(adj_newID);
					exit_flag[adj_newID] = querycnt + 1;
				}
			}
		}

		size = id_cross_arr_pro[u_newID].size();
		for(int j = 0; j < size; j ++){
			if(reach_flag[id_cross_arr_pro[u_newID][j]] != querycnt)
				id_cross_arr[u_newID].push_back(id_cross_arr_pro[u_newID][j]);
		}
		if(id_cross_arr[u_newID].size() > 0)
			sort(id_cross_arr[u_newID].begin(), id_cross_arr[u_newID].end());//, cmpNewID);
		if(excpID[u_newID].size() > 0)
			sort(excpID[u_newID].begin(), excpID[u_newID].end());//, cmpNewID);
	}

	free(exit_flag);
	exit_flag = NULL;
}

bool reach::is_reachable(ID id1, ID id2, int len){
	for(int i = 0; i < len; i ++){
		ID id = id_cross_arr_pro[id1][i];
		if(reach_flag[id] == querycnt)continue;
		//if(rl[id] == rl[id2]){
			if(rlt[id] == rlt[id2]){
				//if(oldID_mapto_newID[id] > oldID_mapto_newID[id2]){
				if(id > id2){
					if(same_tree_reach(id2, id)){
						reach_flag[id] = querycnt;
					}
				}
				else{
					if(same_tree_reach(id, id2)){
						reach_flag[id2] = querycnt;
						return true;
					}
				}
			}
		//}
//		else{
//			 if(root_layer[id] > root_layer[id2]){
//				if(down_layer_to_up_layer_without_cross_located_mark(id2, id)){
//					reach_flag[id] = querycnt;
//						//return true;
//				}
//			 }
//			 else{
//				 if(down_layer_to_up_layer_without_cross_located_mark(id, id2)){
//					 reach_flag[id2] = querycnt;
//					 return true;
//				 }
//			 }
//		}
	}
	return false;
}

bool reach::same_tree_reach(ID id1, ID id2){
	if(interval[id1] >= interval[id2]){
		return true;
	}
	else {
		int len = excpID[id1].size();
		for(int i = 0; i < len; i ++) {
			reach_flag[excpID[id1][i]]=querycnt;
			//if(oldID_mapto_newID[excpID[id1][i]] > oldID_mapto_newID[id2])
			if(excpID[id1][i] > id2)
				return false;
			else if(interval[excpID[id1][i]] >= interval[id2]) {
				return true;
			}
		} //for
	}

	return false;
}

//void reach::code(ID *root_arr, unsigned rootNum, unsigned *&curInDegree){
//	deque<ID>* que = new deque<ID>();
//	idcount = 0;
//	for(int index = 0;index < rootNum; index ++){
//		que->push_front(root_arr[index]);
//
//		while(!que->empty()){
//			ID curID = que->front();
//			que->pop_front();
//
//			idcount++;
//			left_to_right[curID] = idcount;
//
//
//			ID *addr = (ID*) DAG_getOffset(curID);
//			unsigned count = addr[1];
//			addr += 2; //skip the id and count
//
//			for (size_t i = 0; i < count; i++) {
//				if(degree[addr[i]]){
//					degree[addr[i]] --;
//					if(root_located[addr[i]] == root_located[curID])
//						curInDegree[addr[i]]++;
//				}
//				if(degree[addr[i]]){
//					continue;
//				}//degree if
//				que->push_front(addr[i]);
//			}//for
//		} //while
//	} //for
//
////	idcount = 0;
////	for(int index = rootNum - 1; index >= 0; index --){
////		que->push_front(root_arr[index]);
////
////		while(!que->empty()){
////			ID curID = que->front();
////			que->pop_front();
////
////			idcount++;
////			right_to_left[curID] = idcount;
////
////
////			ID *addr = (ID*) DAG_getOffset(curID);
////			unsigned count = addr[1];
////			addr += 2; //skip the id and count
////
////			for (size_t i = 0; i < count; i++) {
////				if(curInDegree[addr[i]]){
////					curInDegree[addr[i]] --;
////					if(root_located[addr[i]] == root_located[curID])
////						degree[addr[i]]++;
////				}
////				if(curInDegree[addr[i]]){
////					continue;
////				}//degree if
////				que->push_front(addr[i]);
////			}//for
////			//nor_interval[curID] = temp;
////		} //while
////	} //for
//
//	delete que;
//	que = NULL;
//}

//void reach::rev_pre_process(bool *&cross_vertices_arr, unsigned *&curInDegree, unsigned *&nextInDegree){
//	int i = 0, k = TopoOrder_cross.size();
//
//	bool flag=true;
//	int vectorsize = k;
//	cross_id.clear();
//	while(flag){
//		flag=false;
//		//cout<<"vectorsize:"<<vectorsize<<endl;
//		for(; i<vectorsize; i++){
//			ID u = TopoOrder_cross[i];
//			//cout<<"u:"<<u<<endl;
//			ID *addr = (ID*) DAG_reverse_getOffset(u);
//			unsigned count = addr[1];
//			addr += 2; //skip the id and count
//
//			for(int j=0; j<count; j++){
//				//if(globalDegree[addr[i]].indeg)
//				curInDegree[addr[j]] --;
//				//degree[addr[j]].indeg ++;
//				//cout<<addr[j]<<" root_lcated:"<<root_located[addr[j]]<<endl;
//
//				if(cross_vertices_arr[u]){
//				    cross_vertices_arr[addr[j]] = true;
//					root_located[addr[j]] = 0; // add
//
//					nextInDegree[addr[j]] ++;
//					//degree[addr[j]].indeg = 0;
//				}
//
//				else if(root_located[addr[j]] != 0){// && root_located[addr[j]] != root_located[u]){
//					if(root_located[addr[j]] != root_located[u] && !cross_vertices_arr[addr[j]]){
//						cross_vertices_arr[addr[j]] = true;
//						cross_id.push_back(addr[j]);
//
//						root_located[addr[j]] = 0; // add
//						//degree[addr[j]].indeg = 0;
//					}
//				}
//				else if(!cross_vertices_arr[addr[j]]){ //add
//					root_located[addr[j]] = root_located[u];
//					//root_layer[addr[j]] = oldID_mapto_newID_count;
//				}
//
//				if(curInDegree[addr[j]] == 0){
//					TopoOrder_cross.push_back(addr[j]);
//					k++;
//				}
//			}
//		}
//
//		if(vectorsize!=k){
//			vectorsize=k;
//			flag=true;
//		}
//	}
//}
//
//void reach::cal_rev_cross_degree(bool *&visited_arr, unsigned *&curInDegree){
//	int len = temp_root_arr.size();
//	deque<ID>* que = new deque<ID>();
//	for(int i = 0; i < len; i ++){
//		que->push_front(temp_root_arr[i]);
//		while(!que->empty()){
//			ID curID = que->front();
//			que->pop_front();
//			ID *addr = (ID*) DAG_reverse_getOffset(curID);
//			unsigned count = addr[1];
//			addr += 2;
//
//			for(int j = 0; j < count; j ++){
//				if(root_located[addr[j]] == root_located[curID]){
//					curInDegree[addr[j]]++;
//					if(!visited_arr[addr[j]]){
//						visited_arr[addr[j]] = true;
//						que->push_back(addr[j]);
//					}
//				}
//			}
//		}
//	}
//	delete que;
//	que = NULL;
//}
//void reach::rev_decode(unsigned *&curInDegree){
//	deque<ID>* que = new deque<ID>();
//
//	int rootNum = temp_root_arr.size();
//	idcount = 0;
//	for(int index = 0;index < rootNum; index ++){
//		que->push_front(temp_root_arr[index]);
//
//		while(!que->empty()){
//			ID curID = que->front();
//			que->pop_front();
//
//			idcount++;
//			rev_oldID_mapto_newID[oldID_mapto_newID[curID]] = idcount;
//			rev_rl[oldID_mapto_newID[curID]] = rev_rl[oldID_mapto_newID[temp_root_arr[index]]];
//			rev_rlt[oldID_mapto_newID[curID]] = root_located[curID];
//
//			ID *addr = (ID*) DAG_reverse_getOffset(curID);
//			unsigned count = addr[1];
//			addr += 2; //skip the id and count
//
//			for (size_t i = 0; i < count; i++) {
//				if(root_located[addr[i]] == root_located[curID]){
//					if(curInDegree[addr[i]]){
//						curInDegree[addr[i]] --;
//						//globalDegree[addr[i]].indeg ++;
//					}
//					if(curInDegree[addr[i]]){
//						//if(root_located[addr[i]] == temp_root_arr[index] + 1)
//							excpID_O[curID].push_back(addr[i]);
//						continue;
//					}//degree if
//					que->push_front(addr[i]);
//				}
//			}//for
//		} //while
//	} //for
//
//	delete que;
//	que = NULL;
//}

//void reach::cal_cross_located(bool *&visited_arr ,bool *&cross_vertices_arr){
//	deque<ID>* que = new deque<ID>();
//
//	//cross located
//	int cross_count = 0;
//	for(unsigned i = 0; i < cross_id.size(); i ++){
//		if(visited_arr[cross_id[i]])continue;
//		que->push_back(cross_id[i]);
//		visited_arr[cross_id[i]] = true;
//
//		cross_count ++;
//		while(!que->empty()){
//			ID curID = que->front();
//			que->pop_front();
//
//			cross_located[curID].push_back(cross_count);
//			ID *addr = (ID*) DAG_getOffset(curID);
//			unsigned count = addr[1];
//			addr += 2;
//			//degree[curID].outdeg = count;
//			//globalDegree[curID].outdeg = count;
//			for (unsigned j = 0; j < count; j++) {
//				degree[addr[j]].indeg ++;
//				globalDegree[addr[j]].indeg ++;
//				if(!visited_arr[addr[j]]){
//					que->push_back(addr[j]);
//					visited_arr[addr[j]] = true;
//				}
//			}
//			addr = (ID *) DAG_reverse_getOffset(curID);
//			count = addr[1];
//			addr += 2;
//			for(unsigned j = 0; j < count; j++){
//				if(cross_vertices_arr[addr[j]] && !visited_arr[addr[j]]){
//					que->push_back(addr[j]);
//					visited_arr[addr[j]] = true;
//				}
//			}
//		}
//	}
//	//cout<<"cross id size:"<<cross_id.size()<<endl;
//	//cout<<"1 cross_count:"<<cross_count<<endl;
//
//	delete que;
//	que = NULL;
//}

//void reach::cal_degree(bool *&visited_arr){
//	deque<ID>* que = new deque<ID>();
//
//	for(unsigned i = 0; i < cross_id.size(); i ++){
//		if(visited_arr[cross_id[i]])continue;
//		que->push_back(cross_id[i]);
//		visited_arr[cross_id[i]] = true;
//
//		while(!que->empty()){
//			ID curID = que->front();
//			que->pop_front();
//
//			ID *addr = (ID*) DAG_getOffset(curID);
//			unsigned count = addr[1];
//			addr += 2;
//			//degree[curID].outdeg = count;
//			//globalDegree[curID].outdeg = count;
//			for (unsigned j = 0; j < count; j++) {
//				degree[addr[j]].indeg ++;
//				globalDegree[addr[j]].indeg ++;
//				if(!visited_arr[addr[j]]){
//					que->push_back(addr[j]);
//					visited_arr[addr[j]] = true;
//				}
//			}
//		}
//	}
//
//	delete que;
//	que = NULL;
//}

//bool reach::down_layer_to_up_layer_without_cross_located_mark(ID id1, ID id2){
//	int len = id_cross_arr[id1].size();
//	if(len == 0){
//		return false;
//	}
//
//	//ID rlayer2 = root_layer[id2] - 1;
//	for(int i = 0; i < len; i ++){
//		ID id = id_cross_arr[id1][i];
//		reach_flag[id] = querycnt;
//		if(root_layer[id] > root_layer[id2]){
//			return false;
//		}
//		else if(root_layer[id] == root_layer[id2]){
//			if(root_located[id] == root_located[id2])
//				if(oldID_mapto_newID[id] <= oldID_mapto_newID[id2]){
//					if(same_tree_reach(id, id2)){
//						return true;
//					}
//				}
//		}
//		else{
//			if(down_layer_to_up_layer_without_cross_located_mark(id, id2))
//				return true;
//		}
//	}
//	return false;
//}
