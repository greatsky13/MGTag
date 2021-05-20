#include "Graph.h"
#include "TempFile.h"
#include "OSFile.h"
#include "Sorter.h"
#include "TSorter.h"
#include <list>
#include <string>

//#define FLAT_ADJLIST 1
string Graph::Dir = "";
Degree *globalDegree;
Degree *globalDegree_ass;
Degree* newGraph_degree;
//vector < vector <unsigned> > tree_degree;
unsigned maxDegree = 100;
vector < vector <ID> > excpID_O;
vector < vector <ID> > crossid_r_adj;

ID* visit_flag;
int querycnt;

Graph::Graph(unsigned maxID) {
	// TODO Auto-generated constructor stub
}

Graph::Graph(string dir) {
	Dir = dir;
	oldID_mapto_newID = NULL;
	newID_mapto_oldID = NULL;
	/////
	idtag = NULL;
	/////
	degree = NULL;
	idcount = 0;
	maxID = 0;
	maxFromID = 0;
}

Graph::Graph(istream& in, string dir, unsigned f) {
	Dir = dir;
	oldID_mapto_newID = NULL;
	newID_mapto_oldID = NULL;
	/////
	idtag = NULL;
	/////
	degree = NULL;
	idcount = 0;
	maxID = 0;
	maxFromID = 0;
	maxCount = 0;

	if(f == 0){
		readGraph(in);
		get_forward_root_vertices(maxID);
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

		cout<<"init..."<<endl;
		init_DAG();
		init_reverse_DAG();
		cout<<"init end"<<endl;
		cout<<"init degree:"<<endl;
		init_degree();
	}
}

Graph::Graph(istream &in, istream &inScc, string dir1, string dir2, string dir3, string dir4){
	Dir = dir1;
	oldID_mapto_newID = NULL;
	newID_mapto_oldID = NULL;
	/////
	idtag = NULL;
	/////
	degree = NULL;
	idcount = 0;
	maxID = 0;
	maxFromID = 0;
	maxCount = 0;

	readGraph2(in, inScc, dir1, dir2, dir3, dir4);
	//get_forward_root_vertices(maxID);
}

Graph::~Graph() {
	// TODO Auto-generated destructor stub
	unsigned i = 0;
	unsigned adjSize = adjMap.size();

	if (oldID_mapto_newID){
		free(oldID_mapto_newID);
		oldID_mapto_newID = NULL;
	}

	if (newID_mapto_oldID) {
		free(newID_mapto_oldID);
		newID_mapto_oldID = NULL;
	}

	for (i = 0; i < adjMap.size(); i++) {
		delete adjMap[i];
		adjMap[i] = NULL;
		adj[i] = NULL;
	}
	adjMap.clear();
	adj.clear();

	if(global_adj_indexMap){
		delete global_adj_indexMap;
		global_adj_indexMap = NULL;
		global_adj_index = NULL;
	}

	if (degree) {
		free(degree);
		degree = NULL;
	}

	char fileName[256];
	for (i = 0; i < adjSize; i++) {
		sprintf(fileName, "%s/adj.%d", Dir.c_str(), i);
		OSFile::FileDelete(fileName);
	}
	sprintf(fileName, "%s/global_adj_index.0", Dir.c_str());
	OSFile::FileDelete(fileName);
}

void Graph::init_degree() {
	degree = (Degree*) calloc(maxID + 1, sizeof(Degree));
	assert(degree);
	for(unsigned i = 0; i <= maxID; i ++){
		ID *addr = (ID *)DAG_getOffset(i);
		assert(addr[0] == i);
		unsigned count = addr[1];
		addr += 2;
		degree[i].outdeg = count;
		for(unsigned j = 0; j < count; j ++){
			degree[addr[j]].indeg ++;
		}
	}
}

void Graph::FIXLINE(char * s) {
	int len = (int) strlen(s) - 1;
	if (s[len] == '\n')
		s[len] = 0;
}

bool Graph::isNewIDExist(ID id, ID& newoff) {
	ID temp = *(oldID_mapto_newID + id);
	if (temp) {
		newoff = temp;
		return true;
	} else {
		return false;
	}
}

bool Graph::setNew(ID pos, ID oldID) {
	*(newID_mapto_oldID +pos) = oldID;
	 return true;
}

bool Graph::setOld(ID pos, ID newID) {
	*(oldID_mapto_newID + pos) = newID;
	return true;
}

void Graph::init(bool forward_or_backward) {
	unsigned fileindex = 0;
	char adjPath[150];
	char adj_indexPath[150];
	//cout<<"fst if"<<endl;
	if(forward_or_backward)
		sprintf(adjPath, "%s/adj.%d", Dir.c_str(), fileindex);
	else
		sprintf(adjPath, "%s/reverse_adj.%d", Dir.c_str(), fileindex);

	//cout<<"while begin"<<endl;
	while (OSFile::FileExists(adjPath) == true) {
		if(forward_or_backward){
			adjMap.push_back(new MMapBuffer(adjPath, 0));
			adj.push_back(adjMap[fileindex]->get_address());
			fileindex++;
			sprintf(adjPath, "%s/adj.%d", Dir.c_str(), fileindex);
		}
		else{
			reverse_adjMap.push_back(new MMapBuffer(adjPath, 0));
			reverse_adj.push_back(reverse_adjMap[fileindex]->get_address());
			fileindex++;
			sprintf(adjPath, "%s/reverse_adj.%d", Dir.c_str(), fileindex);
		}
	}

	//cout<<"while end"<<endl;
	if(forward_or_backward){
		sprintf(adj_indexPath, "%s/global_adj_index.0", Dir.c_str());
		global_adj_indexMap = new MMapBuffer(adj_indexPath, 0);
		global_adj_index = (unsigned *)global_adj_indexMap->get_address();
	}
	else {
		sprintf(adj_indexPath, "%s/reverse_global_adj_index.0", Dir.c_str());
		reverse_global_adj_indexMap = new MMapBuffer(adj_indexPath, 0);
		reverse_global_adj_index = (unsigned *)reverse_global_adj_indexMap->get_address();
	}
	//cout<<"if end"<<endl;
}

void Graph::init_scc() {
	unsigned fileindex = 0;
	char sccPath[150];
	char scc_indexPath[150];
	//cout<<"fst if"<<endl;
	sprintf(sccPath, "%s/scc.%d", Dir.c_str(), fileindex);

	//cout<<"while begin"<<endl;
	while (OSFile::FileExists(sccPath) == true) {
		sccMap.push_back(new MMapBuffer(sccPath, 0));
		scc.push_back(sccMap[fileindex]->get_address());
		fileindex++;
		sprintf(sccPath, "%s/scc.%d", Dir.c_str(), fileindex);
	}

	//cout<<"while end"<<endl;
	sprintf(scc_indexPath, "%s/global_scc_index.0", Dir.c_str());
	global_scc_indexMap = new MMapBuffer(scc_indexPath, 0);
	global_scc_index = (unsigned *)global_scc_indexMap->get_address();
}

void Graph::init_DAG() {
	unsigned fileindex = 0;
	char sccPath[150];
	char scc_indexPath[150];
	//cout<<"fst if"<<endl;
	sprintf(sccPath, "%s/DAG1.%d", Dir.c_str(), fileindex);

	//cout<<"while begin"<<endl;
	while (OSFile::FileExists(sccPath) == true) {
		DAGMap.push_back(new MMapBuffer(sccPath, 0));
		DAG.push_back(DAGMap[fileindex]->get_address());
		fileindex++;
		sprintf(sccPath, "%s/DAG1.%d", Dir.c_str(), fileindex);
	}

	//cout<<"while end"<<endl;
	sprintf(scc_indexPath, "%s/global_DAG_index1.0", Dir.c_str());
	global_DAG_indexMap = new MMapBuffer(scc_indexPath, 0);
	global_DAG_index = (unsigned *)global_DAG_indexMap->get_address();
}

void Graph::init_reverse_DAG() {
	unsigned fileindex = 0;
	char sccPath[150];
	char scc_indexPath[150];
	//cout<<"fst if"<<endl;
	sprintf(sccPath, "%s/DAG_reverse_adj1.%d", Dir.c_str(), fileindex);

	//cout<<"while begin"<<endl;
	while (OSFile::FileExists(sccPath) == true) {
		DAG_reverse_adjMap.push_back(new MMapBuffer(sccPath, 0));
		DAG_reverse_adj.push_back(DAG_reverse_adjMap[fileindex]->get_address());
		fileindex++;
		sprintf(sccPath, "%s/DAG_reverse_adj1.%d", Dir.c_str(), fileindex);
	}

	//cout<<"while end"<<endl;
	sprintf(scc_indexPath, "%s/DAG_reverse_global_adj_index1.0", Dir.c_str());
	DAG_reverse_global_adj_indexMap = new MMapBuffer(scc_indexPath, 0);
	DAG_reverse_global_adj_index = (unsigned *)DAG_reverse_global_adj_indexMap->get_address();
}

void Graph::init_excp() {
	unsigned fileindex = 0;
	char sccPath[150];
	char scc_indexPath[150];
	//cout<<"fst if"<<endl;
	sprintf(sccPath, "%s/exceptions.%d", Dir.c_str(), fileindex);

	//cout<<"while begin"<<endl;
	while (OSFile::FileExists(sccPath) == true) {
		exceptionsMap.push_back(new MMapBuffer(sccPath, 0));
		exceptions.push_back(exceptionsMap[fileindex]->get_address());
		fileindex++;
		sprintf(sccPath, "%s/exceptions.%d", Dir.c_str(), fileindex);
	}

	//cout<<"while end"<<endl;
	sprintf(scc_indexPath, "%s/global_excp_index.0", Dir.c_str());
	global_exceptions_indexMap = new MMapBuffer(scc_indexPath, 0);
	global_exceptions_index = (unsigned *)global_exceptions_indexMap->get_address();
}

void Graph::init_newGraph() {
	unsigned fileindex = 0;
	char sccPath[150];
	char scc_indexPath[150];
	//cout<<"fst if"<<endl;
	sprintf(sccPath, "%s/newGraph.%d", Dir.c_str(), fileindex);

	//cout<<"while begin"<<endl;
	while (OSFile::FileExists(sccPath) == true) {
		newGrpahMap.push_back(new MMapBuffer(sccPath, 0));
		newGrpah.push_back(newGrpahMap[fileindex]->get_address());
		fileindex++;
		sprintf(sccPath, "%s/newGraph.%d", Dir.c_str(), fileindex);
	}

	//cout<<"while end"<<endl;
	sprintf(scc_indexPath, "%s/global_newGraph_index.0", Dir.c_str());
	global_newGrpah_indexMap = new MMapBuffer(scc_indexPath, 0);
	global_newGrpah_index = (unsigned *)global_newGrpah_indexMap->get_address();
}

void Graph::init_reverse_newGraph() {
	unsigned fileindex = 0;
	char sccPath[150];
	char scc_indexPath[150];
	//cout<<"fst if"<<endl;
	sprintf(sccPath, "%s/reverse_newGraph.%d", Dir.c_str(), fileindex);

	//cout<<"while begin"<<endl;
	while (OSFile::FileExists(sccPath) == true) {
		reverse_newGrpahMap.push_back(new MMapBuffer(sccPath, 0));
		reverse_newGrpah.push_back(reverse_newGrpahMap[fileindex]->get_address());
		fileindex++;
		sprintf(sccPath, "%s/reverse_newGraph.%d", Dir.c_str(), fileindex);
	}

	//cout<<"while end"<<endl;
	sprintf(scc_indexPath, "%s/global_reverse_newGraph_index.0", Dir.c_str());
	global_reverse_newGrpah_indexMap = new MMapBuffer(scc_indexPath, 0);
	global_reverse_newGrpah_index = (unsigned *)global_reverse_newGrpah_indexMap->get_address();
}

void Graph::init_hugeVertices() {
	unsigned fileindex = 0;
	char sccPath[150];
	char scc_indexPath[150];
	//cout<<"fst if"<<endl;
	sprintf(sccPath, "%s/hugeVertices.%d", Dir.c_str(), fileindex);

	//cout<<"while begin"<<endl;
	while (OSFile::FileExists(sccPath) == true) {
		hugeVerticesMap.push_back(new MMapBuffer(sccPath, 0));
		hugeVertices.push_back(hugeVerticesMap[fileindex]->get_address());
		fileindex++;
		sprintf(sccPath, "%s/hugeVertices.%d", Dir.c_str(), fileindex);
	}

	//cout<<"while end"<<endl;
	sprintf(scc_indexPath, "%s/global_hugeVertices_index.0", Dir.c_str());
	global_hugeVertices_indexMap = new MMapBuffer(scc_indexPath, 0);
	global_hugeVertices_index = (unsigned *)global_hugeVertices_indexMap->get_address();
}

void Graph::strTrimRight(string& str) {
	string whitespaces(" \t");
	int index = str.find_last_not_of(whitespaces);
	if (index != string::npos)
		str.erase(index+1);
	else
		str.clear();
}

status Graph::readGraph(istream& in) {
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
	cout <<"reading graph..."<<endl;

	string sub;
	int idx;
	int sid = 0;
	int tid = 0;
	TempFile* tempFile = new TempFile("edge_format");
	while (getline(in, buf)) {
		strTrimRight(buf);
		idx = buf.find(":");
			sub = buf.substr(0, buf.find(":"));
			istringstream(sub) >> sid;
		buf.erase(0, idx+2);
		while (buf.find(" ") != string::npos) {
			sub = buf.substr(0, buf.find(" "));
			istringstream(sub) >> tid;
			buf.erase(0, buf.find(" ")+1);
			if(sid == tid)
				cout << "Self-edge " << sid << endl;
			if(tid < 0 || tid > maxID)
				cout << "Wrong tid " << tid << endl;

			if(sid != tid){
				tempFile->writeId(sid);
				tempFile->writeId(tid);
			}
		}
	}
	tempFile->flush();

	TempFile* sortFile = new TempFile("sortFile");
	Sorter::sort(*tempFile, *sortFile, TempFile::skipIdId, TempFile::compare12);
	TempFile* r_sortFile = new TempFile("sortFile");
	Sorter::sort(*tempFile, *r_sortFile, TempFile::skipIdId, TempFile::compare21);
	tempFile->discard();
	delete tempFile;
	cout<<"sort end"<<endl;

	//---------------------------------------------------------------邻接表------------------------------------------------------------------
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

	degree = (Degree*) calloc(vertex_estimate, sizeof(Degree));
	assert(degree);
	globalDegree = (Degree*) calloc(vertex_estimate, sizeof(Degree));
	assert(globalDegree);
	globalDegree_ass = (Degree*) calloc(vertex_estimate, sizeof(Degree));
	assert(globalDegree_ass);
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

		degree[from].outdeg ++;
		degree[to].indeg ++;

		globalDegree[from].outdeg ++;
		globalDegree[to].indeg ++;

		globalDegree_ass[from].outdeg ++;
		globalDegree_ass[to].indeg ++;

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

	//-----------------------------------------------------逆邻接表------------------------------------------------------------
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
			reverse_start_ID[fileindex] = i;
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
				reverse_start_ID[fileindex] = lastto;
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
						reverse_start_ID[fileindex] = i;
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
		reverse_start_ID[fileindex] = lastto;
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
			reverse_start_ID[fileindex] = i;
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

	reverse_adjNum = r_adjFile.size();
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

void Graph::generate_querys(string path){
	 srand((unsigned int)time(NULL));
	 ID min = 0, max = idcount;
	 unsigned num = 1000000;
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
		 outfile<<random_x[i]<<" "<<random_y[i]<<" "<<0<<"\n";
//		 while (random_x[i] == random_y[i]) {
//			 random_y[i] = rand() % (max - min + 1) + min;
//		 }
	}
	 outfile.close();
}

status Graph::readScc(istream& in){
	string buf;
	getline(in, buf);
	int index = buf.find("# Strongly connected components");
	if(index == string::npos){
		cout<<"BAD FILE FORMAT!"<<endl;
		exit(0);
	}
	getline(in, buf);
	getline(in, buf);

	bool firstTime = true;
	int fileindex = -1;
	unsigned fileOff = 0;
	EntityIDBuffer* tempEnt = new EntityIDBuffer();
	vector<TempFile*> sccFile;
	size_t pageSize = (maxID + 1) * sizeof(ID) / (MemoryBuffer::pagesize) +
			((((maxID + 1) * sizeof(ID)) % (MemoryBuffer::pagesize)) ? 1 : 0);
	char scc_indexPath[150];
	sprintf(scc_indexPath, "%s/global_scc_index.0", Dir.c_str());
	MMapBuffer *global_scc_indexFile = new MMapBuffer(scc_indexPath, 2 * pageSize * MemoryBuffer::pagesize);
	unsigned *global_scc_index = (unsigned *)global_scc_indexFile->get_address();

	scc_num = 0;
	scc_start_ID = (ID *)calloc(maxID + 1, sizeof(ID));
	scc_located = (ID *)calloc(maxID + 1, sizeof(ID));
	scc_num_located = (ID *)calloc(maxID + 1, sizeof(ID));

	string sub;
	int idx;
	int sid = 0;
	int tid = 0;
	int first_scc_id = 0;
	while (getline(in, buf)) {
		//cout<<"getline:"<<scc_num<<endl;
		idx = buf.find("\t");
		sub = buf.substr(0, idx);
		istringstream(sub) >> sid;
		if(sid == 1) break;
		buf.erase(0, idx+1);
		idx = buf.find("\t");
		sub = buf.substr(0, idx);
		istringstream(sub) >> first_scc_id;
		//buf.erase(0, buf.find("\t")+1);
		//first scc id
		if(first_scc_id == 0){
			sub = buf.substr(idx + 1, buf.find("\t", idx + 1) - idx - 1);
			istringstream(sub) >> first_scc_id;
		}
		scc_start_ID[scc_num] = first_scc_id;
		tempEnt->empty();
		tempEnt->insertID(first_scc_id);
		tempEnt->insertID(0); // for num

		while (idx != string::npos) {
			sub = buf.substr(0, idx);
			istringstream(sub) >> tid;
			//cout<<tid<<endl;
			buf.erase(0, idx+1);
			idx = buf.find("\t");
			//写入scc
			tempEnt->insertID(tid);
			scc_located[tid] = first_scc_id;
			scc_num_located[tid] = scc_num + 1;
			degree[tid].indeg = 0;
			globalDegree[tid].indeg = 0;
		}
		sub = buf.substr(0);
		istringstream(sub) >> tid;
		tempEnt->insertID(tid);
		scc_located[tid] = first_scc_id;
		scc_num_located[tid] = scc_num + 1;
		degree[tid].indeg = 0;
		globalDegree[tid].indeg = 0;

		unsigned num = tempEnt->getSize() - 2;
		tempEnt->getBuffer()[1] = num; //update num
		unsigned size = tempEnt->getSize() * sizeof(ID);
		if (!firstTime && fileOff + size < (unsigned) (-1) / 8) {
			sccFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
			global_scc_index[2 * scc_num] = fileOff;
			global_scc_index[2 * scc_num + 1] = fileindex;
			fileOff += size;
		} else {
			if (fileindex >= 0) {
				sccFile[fileindex]->close();
			}

			fileindex++;
			fileOff = 0;
			sccFile.push_back(new TempFile(Dir + "/scc", fileindex));
			//scc_start_ID[fileindex] = first_id;
			sccFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
			global_scc_index[2 * scc_num] = 1;//actually is '0', but we set '1' here, for '0' has been used to mark the unexist 'to data'
			global_scc_index[2 * scc_num + 1] = fileindex;
			fileOff += size;
			firstTime = false;
		}
		scc_num ++;
	}
	if(scc_num) {
		//cout<<"write over"<<endl;
		sccFile[fileindex]->close();
		global_scc_indexFile->flush();
		//cout<<"init_scc begin:"<<endl;
		init_scc();
		print_scc();
		//cout<<"init_scc end:"<<endl;
	}

	for(int i = 0;i < sccFile.size();i++){
		delete sccFile[i];
		sccFile[i] = NULL;
	}
	sccFile.clear();
	delete global_scc_indexFile;

	delete tempEnt;

	return OK;
}

status Graph::readGraph2(istream& in, istream& inScc, string path, string path2, string path3, string path4){
	string buf;
	getline(in, buf);

	//strTrimRight(buf);
	int index = buf.find("Directed graph");
	if (index == string::npos) {
		cout << "BAD FILE FORMAT!" << endl;
		exit(0);
	}
	//cout<<"exit"<<endl;
	getline(in, buf);
	getline(in, buf);
	index = buf.find_first_of(' ', 2);
	int index2 = buf.find_first_of(' ', index + 1);
	//cout<<"find"<<endl;

	string str_maxID;
	str_maxID = buf.substr(index + 1,index2 - index - 1);
	//cout<<"copy"<<endl;
	//cout<<"\0"<<endl;
	maxID = atoi(str_maxID.c_str()) - 1;
	//cout<<"atoi"<<endl;
	//cout<<"maxID:"<<maxID<<endl;
	getline(in, buf);

	cout<<"maxID:"<<maxID<<endl;
	//assert(false);
	cout <<"reading graph..."<<endl;
	maxID = 0;
	string sub;
	int idx;
	int sid = 0;
	int tid = 0;
	TempFile* tempFile = new TempFile("edge_format");
	while (getline(in, buf)) {
		idx = buf.find("\t");
		sub = buf.substr(0, idx);
		istringstream(sub) >> sid;
		if(maxID < sid) maxID = sid;
		//buf.erase(0, idx+1);
		sub = buf.substr(idx + 1, buf.length() - idx - 1);
		istringstream(sub) >> tid;
		if(maxID < tid)maxID = tid;
		//buf.erase(0, buf.find(" ")+1);
			//if(sid == tid)
				//cout << "Self-edge " << sid << endl;
			//cout<<"from:"<<sid<<" to:"<<tid<<endl;
			if(sid != tid){
				tempFile->writeId(sid);
				tempFile->writeId(tid);
			}
	}
	tempFile->flush();
	cout<<"maxID:"<<maxID<<endl;
	//assert(false);
	Dir = path;
	construct_adj(tempFile);
	construct_reverse_adj(tempFile);
	tempFile->discard();
	delete tempFile;

	//print_adj();
	//print_reverse_adj();

	string fileName1 = string(path+"/readGraph2_visited_vertices");
	MMapBuffer *visited_vertices = new MMapBuffer(fileName1.c_str(), (maxID+1) * sizeof(bool));
	bool *visited_arr = (bool *)(visited_vertices->get_address());

//	memset(visited_arr, false, sizeof(bool) * (maxID+1));
//	ID *scc_tag = (ID *)calloc(maxID + 1, sizeof(ID));
//	cout<<"begin:"<<endl;
//	DFS_Fst(path, scc_tag, visited_arr);
//	cout<<"end fst"<<endl;
//
//	memset(visited_arr, false, sizeof(bool) * (maxID+1));
//	DFS_Sec(path, scc_tag, visited_arr);
//	cout<<"end sec"<<endl;

	readScc(inScc);
	cout<<"scc num:"<<scc_num<<endl;

	//print_scc();
	generate_DAG(path);
	//print_DAG();
	//print_DAG_degree();

	get_forward_root_vertices(maxID);
	memset(visited_arr, false, sizeof(bool) * (maxID+1));
	TempFile* DAGtempFile = new TempFile("DAGedge_format");
	recode(DAGtempFile, visited_arr);
	cout<<"idcount:"<<idcount<<endl;
	DAGtempFile->flush();
	Dir = path2;
	construct_adj_DAG(path4, DAGtempFile);
	construct_reverse_adj_DAG(DAGtempFile);

	generate_querys(path3);

	remove(fileName1.c_str());
	delete visited_vertices;
	//free(scc_tag);
	return OK;
}

void Graph::recode(TempFile *&DAGtempFile, bool *&visited_arr){
	MemoryMappedFile *rootMap = new MemoryMappedFile();
	assert(rootMap->open((Dir+"/DAGroot.forward.0").c_str()));
	unsigned rootNum = (rootMap->getEnd() - rootMap->getBegin()) / sizeof(ID);
	ID *root_arr = (ID*) rootMap->getBegin();
	idcount = 0;
	oldID_mapto_newID = (ID *)calloc(maxID + 1, sizeof(ID));
	deque<ID>* que = new deque<ID>();
	for(unsigned index = 0;index < rootNum;index++){
		visited_arr[root_arr[index]] = true;
		que->push_front(root_arr[index]);
		while(!que->empty()){
			ID curID = que->front();
			que->pop_front();
			oldID_mapto_newID[curID] = idcount;
			idcount ++;
			ID *addr = (ID *)DAG_getOffset(curID);
			assert(curID == addr[0]);
			unsigned count = addr[1];
			addr += 2;
			for(unsigned i = 0; i < count; i ++){
				if(visited_arr[addr[i]])continue;
				que->push_back(addr[i]);
				visited_arr[addr[i]] = true;
			}
		}
	}
	idcount --;
	//cout<<"DAG maxID:"<<idcount<<endl;
	for(unsigned index = 0;index <= maxID; index++){
		ID *addr = (ID *)DAG_getOffset(index);
		assert(index == addr[0]);
		unsigned count = addr[1];
		addr += 2;
		//cout<<index<<" "<<oldID_mapto_newID[index]<<":"<<endl;
		for(unsigned i = 0; i < count; i ++){
			//cout<<addr[i]<<" "<<oldID_mapto_newID[addr[i]]<<endl;
			DAGtempFile->writeId(oldID_mapto_newID[index]);
			DAGtempFile->writeId(oldID_mapto_newID[addr[i]]);
		}
	}
}

void Graph::construct_adj_DAG(string path, TempFile *tempfile){
	TempFile* sortFile = new TempFile("sortFile");
	Sorter::sort(*tempfile, *sortFile, TempFile::skipIdId, TempFile::compare12);
	cout<<"sort endl"<<endl;

	MemoryMappedFile mappedIn;
	assert(mappedIn.open(sortFile->getFile().c_str()));
	const char* reader = mappedIn.getBegin(), *begin = reader, *limit = mappedIn.getEnd();

	bool firstInsert = true, firstTime = true;
	int fileindex = -1;
	unsigned fileOff = 0;

	vector<TempFile *> adjFile;
	unsigned vertex_estimate = idcount + 1;
	size_t pageSize = vertex_estimate * sizeof(ID) / (MemoryBuffer::pagesize) + (((vertex_estimate * sizeof(ID)) % (MemoryBuffer::pagesize)) ? 1 : 0);
	char adj_indexPath[150];
	sprintf(adj_indexPath, "%s/global_DAG_index.0", Dir.c_str());
	MMapBuffer *global_adj_indexFile = new MMapBuffer(adj_indexPath, 2 * pageSize * MemoryBuffer::pagesize);
	unsigned *global_adj_index = (unsigned *)global_adj_indexFile->get_address();

	unsigned from = *(ID*) reader, to = 0, lastfrom = *(ID*) reader;
	EntityIDBuffer* tempEnt = new EntityIDBuffer();
	ofstream outfile(path.c_str());
	if(!outfile){
		cout<<"unable to open file!"<<endl;
		exit(0);
	}
	outfile<<"graph_for_greach"<<"\n";
	outfile<<idcount + 1<<"\n";
	for(ID i = 0; i < from; i ++) {
		tempEnt->empty();
		tempEnt->insertID(i);
		tempEnt->insertID(0);
		outfile<<i<<": #"<<"\n";

		unsigned num = tempEnt->getSize() - 2;
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
			//start_ID[fileindex] = i;
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

		if (firstInsert) {
			//cout<<"insert"<<endl;
			tempEnt->insertID(from);
			tempEnt->insertID(0); // for num
			tempEnt->insertID(to);
			firstInsert = false;
			outfile<<from<<": "<<to<<" ";
		} else if (from != lastfrom) {
			unsigned num = tempEnt->getSize() - 2;
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
				//start_ID[fileindex] = lastfrom;
				adjFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
				global_adj_index[2 * lastfrom] = 1;//actually is '0', but we set '1' here, for '0' has been used to mark the unexist 'from data'
				global_adj_index[2 * lastfrom + 1] = fileindex;
				fileOff += size;
				firstTime = false;
			}
			//cout<<"from:"<<from<<"lastfrom"<<lastfrom<<endl;
			if(from != lastfrom + 1){
				for(ID i = lastfrom + 1; i < from; i ++) {
					//cout<<"i:"<<i<<endl;
					tempEnt->empty();
					tempEnt->insertID(i);
					tempEnt->insertID(0);
					outfile<<"#"<<"\n"<<i<<": ";

					unsigned num = tempEnt->getSize() - 2;
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
						//start_ID[fileindex] = i;
						adjFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
						global_adj_index[2 * i] = 1;//actually is '0', but we set '1' here, for '0' has been used to mark the unexist 'from data'
						global_adj_index[2 * i + 1] = fileindex;
						fileOff += size;
						firstTime = false;
					}
				}
			}
			outfile<<"#"<<"\n"<<from<<": "<<to<<" ";
			tempEnt->empty();
			tempEnt->insertID(from);
			tempEnt->insertID(0);
			tempEnt->insertID(to);
			lastfrom = from;
		} else {
			//cout<<"insert to:"<<to<<endl;
			tempEnt->insertID(to);
			outfile<<to<<" ";
		}
	}

	unsigned num = tempEnt->getSize() - 2;
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
		//start_ID[fileindex] = lastfrom;
		adjFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
		global_adj_index[2 * lastfrom] = 1;//actually is '0', but we set '1' here, for '0' has been used to mark the unexist 'from data'
		global_adj_index[2 * lastfrom + 1] = fileindex;
		fileOff += size;
		firstTime = false;
	}
	outfile<<"#"<<"\n";
	for(ID i = lastfrom + 1; i <= idcount; i ++) {
		//cout<<"i:"<<i<<endl;
		tempEnt->empty();
		tempEnt->insertID(i);
		tempEnt->insertID(0);
		outfile<<i<<": #"<<"\n";

		unsigned num = tempEnt->getSize() - 2;
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
			//start_ID[fileindex] = i;
			adjFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
			global_adj_index[2 * i] = 1;//actually is '0', but we set '1' here, for '0' has been used to mark the unexist 'from data'
			global_adj_index[2 * i + 1] = fileindex;
			fileOff += size;
			firstTime = false;
		}
	}

	outfile.close();
	cout << "fileIndex:" << fileindex << ",  maxID " << idcount << endl;
	adjFile[fileindex]->close();
	global_adj_indexFile->flush();

	//adjNum = adjFile.size();
	for(int i = 0;i < adjFile.size();i++){
		delete adjFile[i];
		adjFile[i] = NULL;
	}
	adjFile.clear();
	delete global_adj_indexFile;

	mappedIn.close();
	sortFile->discard();
	delete tempEnt;
	delete sortFile;
}
void Graph::construct_reverse_adj_DAG(TempFile *tempfile){
	TempFile* sortFile = new TempFile("sortFile");
	Sorter::sort(*tempfile, *sortFile, TempFile::skipIdId, TempFile::compare21);
	cout<<"sort end"<<endl;

	MemoryMappedFile mappedIn;
	assert(mappedIn.open(sortFile->getFile().c_str()));
	const char* reader = mappedIn.getBegin(), *begin = reader, *limit = mappedIn.getEnd();

	bool firstInsert = true, firstTime = true;
	int fileindex = -1;
	unsigned fileOff = 0;
	unsigned lastto = *(ID *)(reader + sizeof(ID));
	EntityIDBuffer* tempEnt = new EntityIDBuffer();
	vector<TempFile*> adjFile;
	size_t pageSize = (idcount + 1) * sizeof(ID) / (MemoryBuffer::pagesize) + ((((idcount + 1) * sizeof(ID)) % (MemoryBuffer::pagesize)) ? 1 : 0);
	char adj_indexPath[150];
	sprintf(adj_indexPath, "%s/DAG_reverse_global_adj_index.0", Dir.c_str());
	MMapBuffer *global_adj_indexFile = new MMapBuffer(adj_indexPath, 2 * pageSize * MemoryBuffer::pagesize);
	unsigned *global_adj_index = (unsigned *)global_adj_indexFile->get_address();

	ID from = 0, to = *(ID*) (reader + sizeof(ID));

	//to = *(ID*) reader;
	for(ID i = 0; i < to; i ++) {
		tempEnt->empty();
		tempEnt->insertID(i);
		tempEnt->insertID(0);

		unsigned num = tempEnt->getSize() - 2;
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
			adjFile.push_back(new TempFile(Dir + "/DAG_reverse_adj", fileindex));
			//reverse_start_ID[fileindex] = i;
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
		reader += sizeof(ID);
		//cout<<"from:"<<from<<" to:"<<to<<endl;
		//cout<<"lastto:"<<lastto<<endl;

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
				adjFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
				global_adj_index[2 * lastto] = fileOff;
				global_adj_index[2 * lastto + 1] = fileindex;
				fileOff += size;
			} else {
				if (fileindex >= 0) {
					adjFile[fileindex]->close();
				}

				fileindex++;
				fileOff = 0;
				adjFile.push_back(new TempFile(Dir + "/DAG_reverse_adj", fileindex));
				//reverse_start_ID[fileindex] = lastto;
				adjFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
				global_adj_index[2 * lastto] = 1;//actually is '0', but we set '1' here, for '0' has been used to mark the unexist 'to data'
				global_adj_index[2 * lastto + 1] = fileindex;
				fileOff += size;
				firstTime = false;

				//cout << "fileIndex: " << fileindex << ", from minID " << to<< " to maxID " << maxID << endl;
			}

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
						adjFile.push_back(new TempFile(Dir + "/DAG_reverse_adj", fileindex));
						//reverse_start_ID[fileindex] = i;
						adjFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
						global_adj_index[2 * i] = 1;//actually is '0', but we set '1' here, for '0' has been used to mark the unexist 'from data'
						global_adj_index[2 * i + 1] = fileindex;
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

	unsigned num = tempEnt->getSize() - 2;
	tempEnt->getBuffer()[1] = num; //update num
	unsigned size = tempEnt->getSize() * sizeof(ID);
	if (!firstTime && fileOff + size < (unsigned) (-1) / 8) {
		adjFile[fileindex]->write(size, (const char*) tempEnt->getBuffer());
		global_adj_index[2 * lastto] = fileOff;
		global_adj_index[2 * lastto + 1] = fileindex;
		fileOff += size;
	} else {
		if (fileindex >= 0) {
			adjFile[fileindex]->close();
		}

		fileindex++;
		fileOff = 0;
		adjFile.push_back(new TempFile(Dir + "/DAG_reverse_adj", fileindex));
		//reverse_start_ID[fileindex] = lastto;
		adjFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
		global_adj_index[2 * lastto] = 1;//actually is '0', but we set '1' here, for '0' has been used to mark the unexist 'from data'
		global_adj_index[2 * lastto + 1] = fileindex;
		fileOff += size;
		firstTime = false;
	}

	for(ID i = lastto + 1; i <= idcount; i ++) {
		//cout<<"i:"<<i<<endl;
		tempEnt->empty();
		tempEnt->insertID(i);
		tempEnt->insertID(0);

		unsigned num = tempEnt->getSize() - 2;
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
			adjFile.push_back(new TempFile(Dir + "/DAG_reverse_adj", fileindex));
			//reverse_start_ID[fileindex] = i;
			adjFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
			global_adj_index[2 * i] = 1;//actually is '0', but we set '1' here, for '0' has been used to mark the unexist 'from data'
			global_adj_index[2 * i + 1] = fileindex;
			fileOff += size;
			firstTime = false;
		}
	}

	cout << "fileIndex:" << fileindex << ", maxID " << idcount << endl;

	adjFile[fileindex]->close();
	global_adj_indexFile->flush();
	cout<<"success convert into reverse adjacency list"<<endl;

	//reverse_adjNum = adjFile.size();
	for(int i = 0;i < adjFile.size();i++){
		delete adjFile[i];
		adjFile[i] = NULL;
	}
	adjFile.clear();
	delete global_adj_indexFile;

	mappedIn.close();
	sortFile->discard();
	delete sortFile;
	delete tempEnt;
}

void Graph::construct_adj(TempFile *tempfile){
	TempFile* sortFile = new TempFile("sortFile");
	Sorter::sort(*tempfile, *sortFile, TempFile::skipIdId, TempFile::compare12);
	cout<<"sort endl"<<endl;

	MemoryMappedFile mappedIn;
	assert(mappedIn.open(sortFile->getFile().c_str()));
	const char* reader = mappedIn.getBegin(), *begin = reader, *limit = mappedIn.getEnd();

	bool firstInsert = true, firstTime = true;
	int fileindex = -1;
	unsigned fileOff = 0;

	vector<TempFile *> adjFile;
	unsigned vertex_estimate = maxID + 1;
	size_t pageSize = vertex_estimate * sizeof(ID) / (MemoryBuffer::pagesize) + (((vertex_estimate * sizeof(ID)) % (MemoryBuffer::pagesize)) ? 1 : 0);
	char adj_indexPath[150];
	sprintf(adj_indexPath, "%s/global_adj_index.0", Dir.c_str());
	MMapBuffer *global_adj_indexFile = new MMapBuffer(adj_indexPath, 2 * pageSize * MemoryBuffer::pagesize);
	unsigned *global_adj_index = (unsigned *)global_adj_indexFile->get_address();

	degree = (Degree*) calloc(vertex_estimate, sizeof(Degree));
	assert(degree);
	globalDegree = (Degree*) calloc(vertex_estimate, sizeof(Degree));
	assert(globalDegree);

	unsigned from = *(ID*) reader, to = 0, lastfrom = *(ID*) reader;
	EntityIDBuffer* tempEnt = new EntityIDBuffer();
	for(ID i = 0; i < from; i ++) {
		tempEnt->empty();
		tempEnt->insertID(i);
		tempEnt->insertID(0);

		unsigned num = tempEnt->getSize() - 2;
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
			adjFile.push_back(new TempFile(Dir + "/adj", fileindex));
			start_ID[fileindex] = i;
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

		degree[from].outdeg ++;
		degree[to].indeg ++;

		globalDegree[from].outdeg ++;
		globalDegree[to].indeg ++;

		if (firstInsert) {
			//cout<<"insert"<<endl;
			tempEnt->insertID(from);
			tempEnt->insertID(0); // for num
			tempEnt->insertID(to);
			firstInsert = false;
		} else if (from != lastfrom) {
			unsigned num = tempEnt->getSize() - 2;
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
				adjFile.push_back(new TempFile(Dir + "/adj", fileindex));
				start_ID[fileindex] = lastfrom;
				adjFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
				global_adj_index[2 * lastfrom] = 1;//actually is '0', but we set '1' here, for '0' has been used to mark the unexist 'from data'
				global_adj_index[2 * lastfrom + 1] = fileindex;
				fileOff += size;
				firstTime = false;
			}
			//cout<<"from:"<<from<<"lastfrom"<<lastfrom<<endl;
			if(from != lastfrom + 1){
				for(ID i = lastfrom + 1; i < from; i ++) {
					//cout<<"i:"<<i<<endl;
					tempEnt->empty();
					tempEnt->insertID(i);
					tempEnt->insertID(0);

					unsigned num = tempEnt->getSize() - 2;
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
						adjFile.push_back(new TempFile(Dir + "/adj", fileindex));
						start_ID[fileindex] = i;
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

	unsigned num = tempEnt->getSize() - 2;
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
		adjFile.push_back(new TempFile(Dir + "/adj", fileindex));
		start_ID[fileindex] = lastfrom;
		adjFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
		global_adj_index[2 * lastfrom] = 1;//actually is '0', but we set '1' here, for '0' has been used to mark the unexist 'from data'
		global_adj_index[2 * lastfrom + 1] = fileindex;
		fileOff += size;
		firstTime = false;
	}
	for(ID i = lastfrom + 1; i <= maxID; i ++) {
		//cout<<"i:"<<i<<endl;
		tempEnt->empty();
		tempEnt->insertID(i);
		tempEnt->insertID(0);

		unsigned num = tempEnt->getSize() - 2;
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
			adjFile.push_back(new TempFile(Dir + "/adj", fileindex));
			start_ID[fileindex] = i;
			adjFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
			global_adj_index[2 * i] = 1;//actually is '0', but we set '1' here, for '0' has been used to mark the unexist 'from data'
			global_adj_index[2 * i + 1] = fileindex;
			fileOff += size;
			firstTime = false;
		}
	}

	cout << "fileIndex:" << fileindex << ",  maxID " << maxID << endl;
	adjFile[fileindex]->close();
	global_adj_indexFile->flush();

	adjNum = adjFile.size();
	for(int i = 0;i < adjFile.size();i++){
		delete adjFile[i];
		adjFile[i] = NULL;
	}
	adjFile.clear();
	delete global_adj_indexFile;

	mappedIn.close();
	sortFile->discard();
	delete tempEnt;
	delete sortFile;

	init(true);
}
void Graph::construct_reverse_adj(TempFile *tempfile){
	TempFile* sortFile = new TempFile("sortFile");
	Sorter::sort(*tempfile, *sortFile, TempFile::skipIdId, TempFile::compare21);
	cout<<"sort end"<<endl;

	MemoryMappedFile mappedIn;
	assert(mappedIn.open(sortFile->getFile().c_str()));
	const char* reader = mappedIn.getBegin(), *begin = reader, *limit = mappedIn.getEnd();

	bool firstInsert = true, firstTime = true;
	int fileindex = -1;
	unsigned fileOff = 0;
	unsigned lastto = *(ID *)(reader + sizeof(ID));
	EntityIDBuffer* tempEnt = new EntityIDBuffer();
	vector<TempFile*> adjFile;
	size_t pageSize = (maxID + 1) * sizeof(ID) / (MemoryBuffer::pagesize) + ((((maxID + 1) * sizeof(ID)) % (MemoryBuffer::pagesize)) ? 1 : 0);
	char adj_indexPath[150];
	sprintf(adj_indexPath, "%s/reverse_global_adj_index.0", Dir.c_str());
	MMapBuffer *global_adj_indexFile = new MMapBuffer(adj_indexPath, 2 * pageSize * MemoryBuffer::pagesize);
	unsigned *global_adj_index = (unsigned *)global_adj_indexFile->get_address();

	ID from = 0, to = *(ID*) (reader + sizeof(ID));

	//to = *(ID*) reader;
	for(ID i = 0; i < to; i ++) {
		tempEnt->empty();
		tempEnt->insertID(i);
		tempEnt->insertID(0);

		unsigned num = tempEnt->getSize() - 2;
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
			adjFile.push_back(new TempFile(Dir + "/reverse_adj", fileindex));
			reverse_start_ID[fileindex] = i;
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
		reader += sizeof(ID);
		//cout<<"from:"<<from<<" to:"<<to<<endl;
		//cout<<"lastto:"<<lastto<<endl;

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
				adjFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
				global_adj_index[2 * lastto] = fileOff;
				global_adj_index[2 * lastto + 1] = fileindex;
				fileOff += size;
			} else {
				if (fileindex >= 0) {
					adjFile[fileindex]->close();
				}

				fileindex++;
				fileOff = 0;
				adjFile.push_back(new TempFile(Dir + "/reverse_adj", fileindex));
				reverse_start_ID[fileindex] = lastto;
				adjFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
				global_adj_index[2 * lastto] = 1;//actually is '0', but we set '1' here, for '0' has been used to mark the unexist 'to data'
				global_adj_index[2 * lastto + 1] = fileindex;
				fileOff += size;
				firstTime = false;

				//cout << "fileIndex: " << fileindex << ", from minID " << to<< " to maxID " << maxID << endl;
			}

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
						adjFile.push_back(new TempFile(Dir + "/reverse_adj", fileindex));
						reverse_start_ID[fileindex] = i;
						adjFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
						global_adj_index[2 * i] = 1;//actually is '0', but we set '1' here, for '0' has been used to mark the unexist 'from data'
						global_adj_index[2 * i + 1] = fileindex;
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

	unsigned num = tempEnt->getSize() - 2;
	tempEnt->getBuffer()[1] = num; //update num
	unsigned size = tempEnt->getSize() * sizeof(ID);
	if (!firstTime && fileOff + size < (unsigned) (-1) / 8) {
		adjFile[fileindex]->write(size, (const char*) tempEnt->getBuffer());
		global_adj_index[2 * lastto] = fileOff;
		global_adj_index[2 * lastto + 1] = fileindex;
		fileOff += size;
	} else {
		if (fileindex >= 0) {
			adjFile[fileindex]->close();
		}

		fileindex++;
		fileOff = 0;
		adjFile.push_back(new TempFile(Dir + "/reverse_adj", fileindex));
		reverse_start_ID[fileindex] = lastto;
		adjFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
		global_adj_index[2 * lastto] = 1;//actually is '0', but we set '1' here, for '0' has been used to mark the unexist 'from data'
		global_adj_index[2 * lastto + 1] = fileindex;
		fileOff += size;
		firstTime = false;
	}

	for(ID i = lastto + 1; i <= maxID; i ++) {
		//cout<<"i:"<<i<<endl;
		tempEnt->empty();
		tempEnt->insertID(i);
		tempEnt->insertID(0);

		unsigned num = tempEnt->getSize() - 2;
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
			adjFile.push_back(new TempFile(Dir + "/reverse_adj", fileindex));
			reverse_start_ID[fileindex] = i;
			adjFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
			global_adj_index[2 * i] = 1;//actually is '0', but we set '1' here, for '0' has been used to mark the unexist 'from data'
			global_adj_index[2 * i + 1] = fileindex;
			fileOff += size;
			firstTime = false;
		}
	}

	cout << "fileIndex:" << fileindex << ", maxID " << maxID << endl;

	adjFile[fileindex]->close();
	global_adj_indexFile->flush();
	cout<<"success convert into reverse adjacency list"<<endl;

	reverse_adjNum = adjFile.size();
	for(int i = 0;i < adjFile.size();i++){
		delete adjFile[i];
		adjFile[i] = NULL;
	}
	adjFile.clear();
	delete global_adj_indexFile;

	mappedIn.close();
	sortFile->discard();
	delete sortFile;
	delete tempEnt;

	init(false);
}

status Graph::convert_edge(string inputfile, unsigned lineCount) {
	maxID = 0;
	maxFromID = 0;
	maxCount = 0;
	totalCouple = 0;
	FILE * inf = fopen(inputfile.c_str(), "r");
	size_t bytesread = 0;
	size_t linenum = 0;

	if (inf == NULL) {
		cout << "Could not load :" << inputfile << " error: " << strerror(errno)
				<< std::endl;
	}
	assert(inf != NULL);

	TempFile* tempFile = new TempFile("edge_format");
	cout << "Reading in edge list format!" << std::endl;
	char s[1024];
	while (fgets(s, 1024, inf) != NULL) {
		linenum++;
		if (linenum % 10000000 == 0) {
			cout << "Read " << linenum << " lines, " << bytesread / 1024 / 1024.
					<< " MB" << std::endl;
		}
		FIXLINE(s);
		bytesread += strlen(s);
		if (s[0] == '#')
			continue; // Comment
		if (s[0] == '%')
			continue; // Comment

		char delims[] = "\t ";
		char * t;
		t = strtok(s, delims);
		if (t == NULL) {
			cout << "Input file is not in right format. "
					<< "Expecting \"<from>\t<to>\". " << "Current line: \"" << s
					<< "\"\n";
			assert(false);
		}
		ID from = atoi(t);
		t = strtok(NULL, delims);
		if (t == NULL) {
			cout << "Input file is not in right format. "
					<< "Expecting \"<from>\t<to>\". " << "Current line: \"" << s
					<< "\"\n";
			assert(false);
		}
		ID to = atoi(t);
		if(from != to){
			tempFile->writeId(from);
			tempFile->writeId(to);
		}
	}
	fclose(inf);
	tempFile->flush();

	TempFile* sortFile = new TempFile("sortFile");
	Sorter::sort(*tempFile, *sortFile, TempFile::skipIdId, TempFile::compare12);
	tempFile->discard();
	delete tempFile;
	cout<<"sort end"<<endl;

	MemoryMappedFile mappedIn;
	assert(mappedIn.open(sortFile->getFile().c_str()));
	const char* reader = mappedIn.getBegin(), *begin = reader, *limit = mappedIn.getEnd();

	bool firstInsert = true, firstTime = true;
	int fileindex = -1;
	unsigned fileOff = 0;

	vector<TempFile *> adjFile;
	unsigned vertex_estimate = (unsigned) (lineCount * 1.1);
	size_t pageSize = vertex_estimate * sizeof(ID) / (MemoryBuffer::pagesize) + (((vertex_estimate * sizeof(ID)) % (MemoryBuffer::pagesize)) ? 1 : 0);
	char adj_indexPath[150];
	sprintf(adj_indexPath, "%s/global_adj_index.0", Dir.c_str());
	MMapBuffer *global_adj_indexFile = new MMapBuffer(adj_indexPath, 2 * pageSize * MemoryBuffer::pagesize);
	unsigned *global_adj_index = (unsigned *)global_adj_indexFile->get_address();

	degree = (Degree*) calloc(vertex_estimate, sizeof(Degree));
	assert(degree);
	globalDegree = (Degree*) calloc(vertex_estimate, sizeof(Degree));
	assert(globalDegree);
	unsigned from = *(ID*) reader, to = 0, lastfrom = *(ID*) reader;
	EntityIDBuffer* tempEnt = new EntityIDBuffer();
	for(ID i = 0; i < from; i ++) {
		tempEnt->empty();
		tempEnt->insertID(i);
		tempEnt->insertID(0);

		unsigned num = tempEnt->getSize() - 2;
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
			adjFile.push_back(new TempFile(Dir + "/adj", fileindex));
			start_ID[fileindex] = i;
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

		degree[from].outdeg ++;
		degree[to].indeg ++;

		globalDegree[from].outdeg ++;
		globalDegree[to].indeg ++;

		maxID = std::max(from, maxID);
		maxID = std::max(to, maxID);
		maxFromID = std::max(from, maxFromID);

		if (firstInsert) {
			//cout<<"insert"<<endl;
			tempEnt->insertID(from);
			tempEnt->insertID(0); // for num
			tempEnt->insertID(to);
			firstInsert = false;
		} else if (from != lastfrom) {
			unsigned num = tempEnt->getSize() - 2;
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
				adjFile.push_back(new TempFile(Dir + "/adj", fileindex));
				start_ID[fileindex] = lastfrom;
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

					unsigned num = tempEnt->getSize() - 2;
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
						adjFile.push_back(new TempFile(Dir + "/adj", fileindex));
						start_ID[fileindex] = i;
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

	unsigned num = tempEnt->getSize() - 2;
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
		adjFile.push_back(new TempFile(Dir + "/adj", fileindex));
		start_ID[fileindex] = lastfrom;
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

		unsigned num = tempEnt->getSize() - 2;
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
			adjFile.push_back(new TempFile(Dir + "/adj", fileindex));
			start_ID[fileindex] = i;
			adjFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
			global_adj_index[2 * i] = 1;//actually is '0', but we set '1' here, for '0' has been used to mark the unexist 'from data'
			global_adj_index[2 * i + 1] = fileindex;
			fileOff += size;
			firstTime = false;
		}
	}
	maxFromID = maxID;

	cout << "fileIndex:" << fileindex << ",  maxID " << maxID << endl;
	adjFile[fileindex]->close();
	global_adj_indexFile->flush();

	adjNum = adjFile.size();
	for(int i = 0;i < adjFile.size();i++){
		delete adjFile[i];
		adjFile[i] = NULL;
	}
	adjFile.clear();
	delete global_adj_indexFile;

	mappedIn.close();
	sortFile->discard();
	delete tempEnt;
	delete sortFile;

	return OK;
}

status Graph::convert_edge_to_reverse_adj(string inputfile, unsigned lineCount) {
	maxID = 0;
	maxFromID = 0;
	maxCount = 0;
	totalCouple = 0;
	unsigned vertex_estimate = lineCount * 10;
	FILE * inf = fopen(inputfile.c_str(), "r");
	size_t bytesread = 0;
	size_t linenum = 0;

	if (inf == NULL) {
		cout << "Could not load :" << inputfile << " error: " << strerror(errno)
				<< std::endl;
	}
	assert(inf != NULL);

	TempFile* tempFile = new TempFile("edge_format");
	cout << "Reading in edge list format!" << std::endl;
	char s[1024];
	while (fgets(s, 1024, inf) != NULL) {
		linenum++;
		if (linenum % 10000000 == 0) {
			cout << "Read " << linenum << " lines, " << bytesread / 1024 / 1024.
					<< " MB" << std::endl;
		}
		FIXLINE(s);
		bytesread += strlen(s);
		if (s[0] == '#')
			continue; // Comment
		if (s[0] == '%')
			continue; // Comment

		char delims[] = "\t ";
		char * t;
		t = strtok(s, delims);
		if (t == NULL) {
			cout << "Input file is not in right format. "
					<< "Expecting \"<from>\t<to>\". " << "Current line: \"" << s
					<< "\"\n";
			assert(false);
		}
		ID from = atoi(t);
		t = strtok(NULL, delims);
		if (t == NULL) {
			cout << "Input file is not in right format. "
					<< "Expecting \"<from>\t<to>\". " << "Current line: \"" << s
					<< "\"\n";
			assert(false);
		}
		ID to = atoi(t);
		if(from != to) {
			tempFile->writeId(from);
			tempFile->writeId(to);
		}
	}
	fclose(inf);
	tempFile->flush();

	TempFile* sortFile = new TempFile("sortFile");
	Sorter::sort(*tempFile, *sortFile, TempFile::skipIdId, TempFile::compare21);
	//Sorter::sort(*tempFile, *sortFile, TempFile::skipIdId, TempFile::compare12);
	tempFile->discard();
	delete tempFile;

	MemoryMappedFile mappedIn;
	assert(mappedIn.open(sortFile->getFile().c_str()));
	const char* reader = mappedIn.getBegin(), *begin = reader, *limit = mappedIn.getEnd();

	bool firstInsert = true, firstTime = true;
	int fileindex = -1;
	unsigned fileOff = 0;
	unsigned lastto = *(ID *)(reader + sizeof(ID));
	EntityIDBuffer* tempEnt = new EntityIDBuffer();
	vector<TempFile*> adjFile;
	size_t pageSize = vertex_estimate * sizeof(ID) / (MemoryBuffer::pagesize) + (((vertex_estimate * sizeof(ID)) % (MemoryBuffer::pagesize)) ? 1 : 0);
	char adj_indexPath[150];
	sprintf(adj_indexPath, "%s/reverse_global_adj_index.0", Dir.c_str());
	MMapBuffer *global_adj_indexFile = new MMapBuffer(adj_indexPath, 2 * pageSize * MemoryBuffer::pagesize);
	unsigned *global_adj_index = (unsigned *)global_adj_indexFile->get_address();

	ID from = 0, to = *(ID*) (reader + sizeof(ID));

	//to = *(ID*) reader;
	for(ID i = 0; i < to; i ++) {
		tempEnt->empty();
		tempEnt->insertID(i);
		tempEnt->insertID(0);

		unsigned num = tempEnt->getSize() - 2;
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
			adjFile.push_back(new TempFile(Dir + "/reverse_adj", fileindex));
			reverse_start_ID[fileindex] = i;
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
		reader += sizeof(ID);
		//cout<<"from:"<<from<<" to:"<<to<<endl;
		//cout<<"lastto:"<<lastto<<endl;

		maxID = std::max(from, maxID);
		maxID = std::max(to, maxID);
		maxFromID = std::max(to, maxFromID);

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
				adjFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
				global_adj_index[2 * lastto] = fileOff;
				global_adj_index[2 * lastto + 1] = fileindex;
				fileOff += size;
			} else {
				if (fileindex >= 0) {
					adjFile[fileindex]->close();
				}

				fileindex++;
				fileOff = 0;
				adjFile.push_back(new TempFile(Dir + "/reverse_adj", fileindex));
				reverse_start_ID[fileindex] = lastto;
				adjFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
				global_adj_index[2 * lastto] = 1;//actually is '0', but we set '1' here, for '0' has been used to mark the unexist 'to data'
				global_adj_index[2 * lastto + 1] = fileindex;
				fileOff += size;
				firstTime = false;

				//cout << "fileIndex: " << fileindex << ", from minID " << to<< " to maxID " << maxID << endl;
			}
			maxCount = max(maxCount, num);

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
						adjFile.push_back(new TempFile(Dir + "/reverse_adj", fileindex));
						reverse_start_ID[fileindex] = i;
						adjFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
						global_adj_index[2 * i] = 1;//actually is '0', but we set '1' here, for '0' has been used to mark the unexist 'from data'
						global_adj_index[2 * i + 1] = fileindex;
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

	unsigned num = tempEnt->getSize() - 2;
	tempEnt->getBuffer()[1] = num; //update num
	unsigned size = tempEnt->getSize() * sizeof(ID);
	if (!firstTime && fileOff + size < (unsigned) (-1) / 8) {
		adjFile[fileindex]->write(size, (const char*) tempEnt->getBuffer());
		global_adj_index[2 * lastto] = fileOff;
		global_adj_index[2 * lastto + 1] = fileindex;
		fileOff += size;
	} else {
		if (fileindex >= 0) {
			adjFile[fileindex]->close();
		}

		fileindex++;
		fileOff = 0;
		adjFile.push_back(new TempFile(Dir + "/reverse_adj", fileindex));
		reverse_start_ID[fileindex] = lastto;
		adjFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
		global_adj_index[2 * lastto] = 1;//actually is '0', but we set '1' here, for '0' has been used to mark the unexist 'from data'
		global_adj_index[2 * lastto + 1] = fileindex;
		fileOff += size;
		firstTime = false;
	}
	maxCount = max(maxCount, num);
	for(ID i = lastto + 1; i <= maxID; i ++) {
		//cout<<"i:"<<i<<endl;
		tempEnt->empty();
		tempEnt->insertID(i);
		tempEnt->insertID(0);

		unsigned num = tempEnt->getSize() - 2;
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
			adjFile.push_back(new TempFile(Dir + "/reverse_adj", fileindex));
			reverse_start_ID[fileindex] = i;
			adjFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
			global_adj_index[2 * i] = 1;//actually is '0', but we set '1' here, for '0' has been used to mark the unexist 'from data'
			global_adj_index[2 * i + 1] = fileindex;
			fileOff += size;
			firstTime = false;
		}
	}

	maxFromID = maxID;
	cout << "fileIndex:" << fileindex << ", maxID " << maxID << endl;

	adjFile[fileindex]->close();
	global_adj_indexFile->flush();
	cout<<"success convert into reverse adjacency list"<<endl;

	reverse_adjNum = adjFile.size();
	for(int i = 0;i < adjFile.size();i++){
		delete adjFile[i];
		adjFile[i] = NULL;
	}
	adjFile.clear();
	delete global_adj_indexFile;

	mappedIn.close();
	sortFile->discard();
	delete sortFile;
	delete tempEnt;

	return OK;
}

status Graph::convert_adj(string inputfile, unsigned lineCount) {
	maxID = 0;
	maxFromID = 0;
	maxCount = 0;
	totalCouple = 0;
	unsigned vertex_estimate = (unsigned) (lineCount * 1.1);
	degree = (Degree*) calloc(vertex_estimate, sizeof(Degree));
	assert(degree);
	//globalDegree = degree;

	FILE * inf = fopen(inputfile.c_str(), "r");
	if (inf == NULL) {
		cerr << "Could not load :" << inputfile << " error: " << strerror(errno)
				<< std::endl;
	}
	assert(inf != NULL);
	cout << "Reading in adjacency list format!" << inputfile << std::endl;

	int maxlen = 100000000;
	char * s = (char*) malloc(maxlen);

	size_t bytesread = 0;

	char delims[] = " \t";
	size_t linenum = 0;
	size_t lastlog = 0;

	unsigned from = 0, to = 0;
	EntityIDBuffer* tempEnt = new EntityIDBuffer();

	/* PHASE 1 - count */
	bool firstTime = true;
	int fileindex = -1;
	unsigned fileOff = 0;
	vector<TempFile*> adjFile;
	size_t pageSize = vertex_estimate * sizeof(ID) / (MemoryBuffer::pagesize) + (((vertex_estimate * sizeof(ID)) % (MemoryBuffer::pagesize)) ? 1 : 0);
	char adj_indexPath[150];
	sprintf(adj_indexPath, "%s/global_adj_index.0", Dir.c_str());
	MMapBuffer *global_adj_indexFile = new MMapBuffer(adj_indexPath, 2 * pageSize * MemoryBuffer::pagesize);
	unsigned *global_adj_index = (unsigned *)global_adj_indexFile->get_address();

	while (fgets(s, maxlen, inf) != NULL) {
		linenum++;
		if (bytesread - lastlog >= 500000000) {
			cerr << "Read " << linenum << " lines, " << bytesread / 1024 / 1024.
					<< " MB" << std::endl;
			lastlog = bytesread;
		}
		FIXLINE(s);
		bytesread += strlen(s);

		if (s[0] == '#')
			continue; // Comment
		if (s[0] == '%')
			continue; // Comment
		char *t = strtok(s, delims);
		from = atoi(t);

		maxID = std::max(from, maxID);
		maxFromID = std::max(from, maxFromID);

		tempEnt->empty();
		tempEnt->insertID(from);

		t = strtok(NULL, delims);
		if (t != NULL) {
			unsigned num = atoi(t);
			unsigned i = 0;
			if (num == 0)
				continue;
			tempEnt->insertID(num);

			while ((t = strtok(NULL, delims)) != NULL) {
				to = atoi(t);

				if (from != to) {
					maxID = std::max(to, maxID);
					degree[from].outdeg++;
					degree[to].indeg++;
					tempEnt->insertID(to);
				}
				i++;
			}

			unsigned size = tempEnt->getSize() * sizeof(ID);
			if (!firstTime && fileOff + size < (unsigned) (-1) / 8) {
				adjFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
				global_adj_index[2 * from] = fileOff;
				global_adj_index[2 * from + 1] = fileindex;
				fileOff += size;
			} else {
				if (fileindex >= 0) {
					adjFile[fileindex]->close();
				}
				fileindex++;
				fileOff = 0;
				adjFile.push_back(new TempFile(Dir + "/adj", fileindex));
				start_ID[fileindex] = from;
				adjFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
				global_adj_index[2 * from] = 1;//actually is '0', but we set '1' here, for '0' has been used to mark the unexist 'from data'
				global_adj_index[2 * from + 1] = fileindex;
				fileOff += size;
				firstTime = false;

				cout << "fileIndex:" << fileindex << "  from minID " << from
						<< " to maxID " << maxID << endl;
			}
			maxCount = max(maxCount, i);
			if (num != i)
				cerr << "Mismatch when reading adjacency list: " << num
						<< " != " << i << " s: " << std::string(s)
						<< " on line: " << linenum << std::endl;
			assert(num == i);
			totalCouple += num;
		}

	}
	adjFile[fileindex]->close();
	global_adj_indexFile->flush();

	adjNum = adjFile.size();
	for(int i = 0;i < adjFile.size();i++){
		delete adjFile[i];
		adjFile[i] = NULL;
	}
	adjFile.clear();
	delete global_adj_indexFile;

	cout << "finish encode...  maxID:" << maxID << endl;
	delete tempEnt;
	free(s);
	fclose(inf);
	return OK;
}

status Graph::convert_adj_nonum(string inputfile, unsigned lineCount) {
	maxID = 0;
	maxFromID = 0;
	maxCount = 0;
	totalCouple = 0;
	unsigned vertex_estimate = lineCount * 10;
	//unsigned vertex_estimate = (unsigned) (lineCount * 1.1);
	cout << "vertex_estimate: " << vertex_estimate << endl;
	degree = (Degree*) calloc(vertex_estimate, sizeof(Degree));
	assert(degree);
	globalDegree = (Degree*) calloc(vertex_estimate, sizeof(Degree));
	assert(globalDegree);

	FILE * inf = fopen(inputfile.c_str(), "r");
	if (inf == NULL) {
		cerr << "Could not load :" << inputfile << " error: " << strerror(errno)
				<< std::endl;
	}
	assert(inf != NULL);
	cout << "Reading in adjacency list format!" << std::endl;

	int maxlen = 100000000;
	char * s = (char*) malloc(maxlen);

	size_t bytesread = 0;

	char delims[] = " \t";
	size_t linenum = 0;
	size_t lastlog = 0;

	unsigned from = 0, to = 0;
	EntityIDBuffer* tempEnt = new EntityIDBuffer();

	/* PHASE 1 - count */
	bool firstTime = true;
	int fileindex = -1;
	unsigned fileOff = 0;
	vector<TempFile*> adjFile;
	size_t pageSize = vertex_estimate * sizeof(ID) / (MemoryBuffer::pagesize) + (((vertex_estimate * sizeof(ID)) % (MemoryBuffer::pagesize)) ? 1 : 0);
	char adj_indexPath[150];
	sprintf(adj_indexPath, "%s/global_adj_index.0", Dir.c_str());
	MMapBuffer *global_adj_indexFile = new MMapBuffer(adj_indexPath, 2 * pageSize * MemoryBuffer::pagesize);
	unsigned *global_adj_index = (unsigned *)global_adj_indexFile->get_address();

	while (fgets(s, maxlen, inf) != NULL) {
		linenum++;
		if (bytesread - lastlog >= 500000000) {
			cerr << "Read " << linenum << " lines, " << bytesread / 1024 / 1024.
					<< " MB" << std::endl;
			lastlog = bytesread;
		}
		FIXLINE(s);
		bytesread += strlen(s);

		if (s[0] == '#')
			continue; // Comment
		if (s[0] == '%')
			continue; // Comment
		char * t = strtok(s, delims);
		from = atoi(t);

		maxID = std::max(from, maxID);
		maxFromID = std::max(from, maxFromID);

		//cout<<"from:"<<from<<" ";
		tempEnt->empty();
		tempEnt->insertID(from);
		unsigned num = 0;
		tempEnt->insertID(num);

		while ((t = strtok(NULL, delims)) != NULL) {
			to = atoi(t);
			if (from != to) {
				//cout<<"to:"<<to<<" ";
				maxID = std::max(to, maxID);

				degree[from].outdeg++;
				degree[to].indeg++;

				globalDegree[from].outdeg ++;
				globalDegree[to].indeg ++;

				tempEnt->insertID(to);
				num++;
			}
		}

		//cout<<endl;
		tempEnt->getBuffer()[1] = num;
		totalCouple += num;
		unsigned size = tempEnt->getSize() * sizeof(ID);
		if (!firstTime && fileOff + size < (unsigned) (-1) / 8) {
			adjFile[fileindex]->write(size, (const char*) tempEnt->getBuffer());
			global_adj_index[2 * from] = fileOff;
			global_adj_index[2 * from + 1] = fileindex;
			fileOff += size;
		} else {
			if (fileindex >= 0) {
				adjFile[fileindex]->close();
			}

			fileindex++;
			fileOff = 0;
			adjFile.push_back(new TempFile(Dir + "/adj", fileindex));
			start_ID[fileindex] = from;
			adjFile[fileindex]->write(size, (const char*) tempEnt->getBuffer());
			global_adj_index[2 * from] = 1;//actually is '0', but we set '1' here, for '0' has been used to mark the unexist 'from data'
			global_adj_index[2 * from + 1] = fileindex;
			fileOff += size;
			firstTime = false;

			cout << "fileIndex: " << fileindex << ", from minID " << from << " to maxID "<< maxID << endl;
		}
		maxCount = max(maxCount, num);

	}

	//---------------
	//cout<<"maxID:"<<maxID<<endl;
	for(ID i = from + 1; i <= maxID; i ++) {
		//cout<<"from:"<<i<<" ";
		tempEnt->empty();
		tempEnt->insertID(i);
		unsigned num = 0;
		tempEnt->insertID(num);

		tempEnt->getBuffer()[1] = num;
		totalCouple += num;
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
			adjFile.push_back(new TempFile(Dir + "/adj", fileindex));
			start_ID[fileindex] = i;
			adjFile[fileindex]->write(size, (const char*) tempEnt->getBuffer());
			global_adj_index[2 * i] = 1;//actually is '0', but we set '1' here, for '0' has been used to mark the unexist 'from data'
			global_adj_index[2 * i + 1] = fileindex;
			fileOff += size;
			firstTime = false;

			cout << "fileIndex: " << fileindex << ", from minID " << i << " to maxID "<< maxID << endl;
		}
	}
	//---------------

	adjFile[fileindex]->close();
	global_adj_indexFile->flush();

	adjNum = adjFile.size();
	for(int i = 0;i < adjFile.size();i++){
		delete adjFile[i];
		adjFile[i] = NULL;
	}
	adjFile.clear();
	delete global_adj_indexFile;

	cout << "finish encode...  maxID:" << maxID << endl;
	delete tempEnt;
	free(s);
	fclose(inf);
	return OK;
}

status Graph::convert_adj_to_reverse_adj(string inputfile, unsigned lineCount) {
	maxID = 0;
	maxFromID = 0;
	maxCount = 0;
	totalCouple = 0;
	unsigned vertex_estimate = lineCount * 10;
	//unsigned vertex_estimate = (unsigned) (lineCount * 1.1);
	cout << "vertex_estimate: " << vertex_estimate << endl;
	degree = (Degree*) calloc(vertex_estimate, sizeof(Degree));
	assert(degree);
	//globalDegree = degree;

	FILE * inf = fopen(inputfile.c_str(), "r");
	if (inf == NULL) {
		cerr << "Could not load :" << inputfile << ", error: "
				<< strerror(errno) << std::endl;
	}
	assert(inf != NULL);
	cout << "Reading in adjacency list format!" << std::endl;

	int maxlen = 100000000;
	char *s = (char*) malloc(maxlen);
	size_t bytesread = 0, lastlog = 0;
	char delims[] = " \t";
	size_t linenum = 0;
	unsigned from = 0, to = 0;
	TempFile* tempFile = new TempFile("edge_format");

	while (fgets(s, maxlen, inf) != NULL) {
		linenum++;
		if (bytesread - lastlog >= 500000000) {
			cerr << "Read " << linenum << " lines, " << bytesread / 1024 / 1024.
					<< " MB" << std::endl;
			lastlog = bytesread;
		}
		FIXLINE(s);
		bytesread += strlen(s);

		if (s[0] == '#')
			continue; // Comment
		if (s[0] == '%')
			continue; // Comment
		char *t = strtok(s, delims);
		from = atoi(t);

		while ((t = strtok(NULL, delims)) != NULL) {
			to = atoi(t);
////
			if (from != to) {
				degree[from].outdeg++;
				degree[to].indeg++;
				tempFile->writeId(from);
				tempFile->writeId(to);
			}
			totalCouple++;
		}
	}
	tempFile->flush();
	fclose(inf);
	free(s);

	TempFile* sortFile = new TempFile("sortFile");
	Sorter::sort(*tempFile, *sortFile, TempFile::skipIdId, TempFile::compare21);
	tempFile->discard();
	delete tempFile;
	cout<<"success convert into edge format"<<endl;

	MemoryMappedFile mappedIn;
	assert(mappedIn.open(sortFile->getFile().c_str()));
	const char* reader = mappedIn.getBegin(), *begin = reader, *limit = mappedIn.getEnd();

	bool firstInsert = true, firstTime = true;
	int fileindex = -1;
	unsigned fileOff = 0;
	unsigned lastto = *(ID *)(reader + sizeof(ID));
	EntityIDBuffer* tempEnt = new EntityIDBuffer();
	vector<TempFile*> adjFile;
	size_t pageSize = vertex_estimate * sizeof(ID) / (MemoryBuffer::pagesize) + (((vertex_estimate * sizeof(ID)) % (MemoryBuffer::pagesize)) ? 1 : 0);
	char adj_indexPath[150];
	sprintf(adj_indexPath, "%s/reverse_global_adj_index.0", Dir.c_str());
	MMapBuffer *global_adj_indexFile = new MMapBuffer(adj_indexPath, 2 * pageSize * MemoryBuffer::pagesize);
	unsigned *global_adj_index = (unsigned *)global_adj_indexFile->get_address();

	while (reader < limit) {
		from = *(ID*) reader;
		reader += sizeof(ID);
		to = *(ID*) reader;
		reader += sizeof(ID);
		//cout<<"from:"<<from<<" to:"<<to<<endl;
		//cout<<"lastto:"<<lastto<<endl;

		maxID = std::max(from, maxID);
		maxID = std::max(to, maxID);
		maxFromID = std::max(to, maxFromID);

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
				adjFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
				global_adj_index[2 * lastto] = fileOff;
				global_adj_index[2 * lastto + 1] = fileindex;
				fileOff += size;
			} else {
				if (fileindex >= 0) {
					adjFile[fileindex]->close();
				}

				fileindex++;
				fileOff = 0;
				adjFile.push_back(new TempFile(Dir + "/reverse_adj", fileindex));
				reverse_start_ID[fileindex] = lastto;
				adjFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
				global_adj_index[2 * lastto] = 1;//actually is '0', but we set '1' here, for '0' has been used to mark the unexist 'to data'
				global_adj_index[2 * lastto + 1] = fileindex;
				fileOff += size;
				firstTime = false;

				cout << "fileIndex: " << fileindex << ", from minID " << to<< " to maxID " << maxID << endl;
			}
			maxCount = max(maxCount, num);

			tempEnt->empty();
			tempEnt->insertID(to);
			tempEnt->insertID(0);
			tempEnt->insertID(from);
			lastto = to;
		} else {
			tempEnt->insertID(from);
		}
	}
	unsigned num = tempEnt->getSize() - 2;
	tempEnt->getBuffer()[1] = num; //update num
	unsigned size = tempEnt->getSize() * sizeof(ID);
	adjFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
	global_adj_index[2 * lastto] = fileOff;
	global_adj_index[2 * lastto + 1] = fileindex;
	fileOff += size;

	adjFile[fileindex]->close();
	global_adj_indexFile->flush();
	cout<<"success convert into reverse adjacency list"<<endl;

	reverse_adjNum = adjFile.size();
	for(int i = 0;i < adjFile.size();i++){
		delete adjFile[i];
		adjFile[i] = NULL;
	}
	adjFile.clear();
	delete global_adj_indexFile;

	mappedIn.close();
	sortFile->discard();
	delete sortFile;
	delete tempEnt;

	return OK;
}

char* Graph::getOffset(ID id) {
	if (id > maxID || id < start_ID[0])return NULL;

	unsigned fileindex = global_adj_index[2 * id + 1];
	unsigned adjOff = global_adj_index[2 * id];
	if(adjOff == 0)return NULL; //'0' indicate unexist 'from or to data'
	if(adjOff == 1)adjOff = 0; //we modify the '1' back to '0' here
	char *temp = (adj[fileindex] + adjOff);
	return temp;
}

char* Graph::reverse_getOffset(ID id) {
	if (id > maxID || id < reverse_start_ID[0])return NULL;

	unsigned fileindex = reverse_global_adj_index[2 * id + 1];
	unsigned adjOff = reverse_global_adj_index[2 * id];
	if(adjOff == 0)return NULL; //'0' indicate unexist 'from or to data'
	if(adjOff == 1)adjOff = 0; //we modify the '1' back to '0' here
	char *temp = (reverse_adj[fileindex] + adjOff);
	return temp;
}

char* Graph::scc_getOffset(ID id){
	//if (id < scc_start_ID[0])return NULL;

//	unsigned i = 0;
//	for(; i < scc_num; i ++) {
//		if(scc_start_ID[i] == id)
//			break;
//	}

	unsigned i = scc_num_located[id] - 1;

	unsigned fileindex = global_scc_index[2 * i + 1];
	unsigned adjOff = global_scc_index[2 * i];
	if(adjOff == 0)return NULL; //'0' indicate unexist 'from or to data'
	if(adjOff == 1)adjOff = 0; //we modify the '1' back to '0' here
	char *temp = (scc[fileindex] + adjOff);
	return temp;
}

char* Graph::DAG_getOffset(ID id){
	if (id < 0)return NULL;

	unsigned fileindex = global_DAG_index[2 * id + 1];
	unsigned adjOff = global_DAG_index[2 * id];
	if(adjOff == 0)return NULL; //'0' indicate unexist 'from or to data'
	if(adjOff == 1)adjOff = 0; //we modify the '1' back to '0' here
	char *temp = (DAG[fileindex] + adjOff);
	return temp;
}

char* Graph::DAG_reverse_getOffset(ID id) {
	if (id < 0)return NULL;

	unsigned fileindex = DAG_reverse_global_adj_index[2 * id + 1];
	unsigned adjOff = DAG_reverse_global_adj_index[2 * id];
	if(adjOff == 0)return NULL; //'0' indicate unexist 'from or to data'
	if(adjOff == 1)adjOff = 0; //we modify the '1' back to '0' here
	char *temp = (DAG_reverse_adj[fileindex] + adjOff);
	return temp;
}

char* Graph::excp_getOffset(unsigned count) {
	if (count < 0 || count >= excp_idcount)return NULL;

	//count --;
	unsigned fileindex = global_exceptions_index[2 * count + 1];
	unsigned adjOff = global_exceptions_index[2 * count];
	if(adjOff == 0)return NULL; //'0' indicate unexist 'from or to data'
	if(adjOff == 1)adjOff = 0; //we modify the '1' back to '0' here
	char *temp = (exceptions[fileindex] + adjOff);
	return temp;
}

unsigned Graph::find_idcount(ID id) {
	unsigned min = 0, max = newGraph_idcount - 1;
	while(min != max){
		if(id < newGraph_id[(max - min + 1) / 2 + min])
			max = (max - min + 1) / 2 + min - 1;
		else
			min = (max - min + 1) / 2 + min;
	}

	if(newGraph_id[min] != id){
		cout<<"cannot find newGraph ID!"<<endl;
		assert(false);
	}
	return min;
}

char* Graph::newGraph_getOffset(ID id) {
	//if (id < newGraph_start_ID[0])return NULL;
	if(id  < newGraph_id[0] || id > newGraph_id[newGraph_idcount - 1]){
		cout<<"newGraph_getOffset null: "<<id<<" "<<newGraph_id[0]<<endl;
		return NULL;
	}

	unsigned i = id_belong_tree[id];
	unsigned fileindex = global_newGrpah_index[2 * i + 1];
	unsigned adjOff = global_newGrpah_index[2 * i];
	if(adjOff == 0)return NULL; //'0' indicate unexist 'from or to data'
	if(adjOff == 1)adjOff = 0; //we modify the '1' back to '0' here
	char *temp = (newGrpah[fileindex] + adjOff);
	return temp;
}

char* Graph::reverse_newGraph_getOffset(ID id) {
	if(id  < newGraph_id[0] || id > newGraph_id[newGraph_idcount - 1]){
		cout<<"reverse_newGraph_getOffset null: "<<id<<" "<<newGraph_id[0]<<endl;
		return NULL;
	}

	unsigned i = id_belong_tree[id];
	unsigned fileindex = global_reverse_newGrpah_index[2 * i + 1];
	unsigned adjOff = global_reverse_newGrpah_index[2 * i];
	if(adjOff == 0)return NULL; //'0' indicate unexist 'from or to data'
	if(adjOff == 1)adjOff = 0; //we modify the '1' back to '0' here
	char *temp = (reverse_newGrpah[fileindex] + adjOff);
	return temp;
}

char* Graph::hugeVertices_getOffset(ID id) {
	if(id  < newGraph_id[0] || id > newGraph_id[newGraph_idcount - 1]){
		cout<<"newGraph_getOffset null: "<<id<<" "<<newGraph_id[0]<<endl;
		return NULL;
	}

	unsigned i = id_belong_tree[id];
	unsigned fileindex = global_hugeVertices_index[2 * i + 1];
	unsigned adjOff = global_hugeVertices_index[2 * i];
	if(adjOff == 0)return NULL; //'0' indicate unexist 'from or to data'
	if(adjOff == 1)adjOff = 0; //we modify the '1' back to '0' here
	char *temp = (hugeVertices[fileindex] + adjOff);
	return temp;
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

int qcompare_ass_indegree(const void* a, const void* b) {
	//sort from small to large
	ID p1 = *(const ID*) a;
	ID p2 = *(const ID*) b;
	p1 = globalDegree_ass[p1].indeg;
	p2 = globalDegree_ass[p2].indeg;
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

bool Graph::zeroDegree(ID id) {
	if (!degree[id].indeg && !degree[id].outdeg)
		return true;
	else
		return false;
}

bool Graph::both_in_and_out_vertex(ID id) {
	if (degree[id].indeg && degree[id].outdeg)
		return true;
	else
		return false;
}

void Graph::get_root_vertices(ID maxID) {
	EntityIDBuffer* buffer = new EntityIDBuffer();
	//buffer->setIDCount(1);

	//cout<<"root:"<<endl;
	//cout<<"start:";
	for (ID start = 0; start <= maxID; start++) {
		if (!degree[start].indeg && degree[start].outdeg) {
			buffer->insertID(start);
			//cout<<start<<" ";
		}
	}
	//cout<<endl;
	qsort((void*) buffer->getBuffer(), buffer->getSize(), 4, qcompareoutdegree);
	if (buffer->getSize() == 0)buffer->insertID(0);
	cout << "forward root number: " << buffer->getSize() << endl;
	TempFile* rootFile = new TempFile(Dir + "/root.forward", 0);
	rootFile->write((size_t) buffer->getSize() * sizeof(ID),(const char*) buffer->getBuffer());
	rootFile->flush();
	delete rootFile;

	buffer->empty();
	for (ID start = 0; start <= maxID; start++) {
		if (degree[start].indeg && !degree[start].outdeg) {
		//if ((degree[start].indeg > 1) && !degree[start].outdeg) {
			buffer->insertID(start);
		}
	}
	qsort((void*) buffer->getBuffer(), buffer->getSize(), 4, qcompareindegree21);
	if (buffer->getSize() == 0)buffer->insertID(0);
	cout << "backward root number: " << buffer->getSize() << endl;
	rootFile = new TempFile(Dir + "/root.backward", 0);
	rootFile->write((size_t) buffer->getSize() * sizeof(ID),(const char*) buffer->getBuffer());
	rootFile->flush();
	delete rootFile;

	cout << "finish writing rootFile" << endl;
	delete buffer;
}

void Graph::sort_degree(unsigned fileindex) {
	cout << "sort:" << fileindex << "  begin..." << endl;
	if (fileindex == adj.size()) {
		get_root_vertices(maxID);
		return;
	}

	size_t whenflush = 0;
	char* begin = adjMap[fileindex]->get_address();
	char* limit = begin + adjMap[fileindex]->get_length();
	char* reader = begin;
	size_t count = 0;
	size_t mcount = maxCount;
	ID* sortBuf = (ID*) malloc(mcount * 4);
	while (reader < limit) {
		reader += 4; //skip from ID;
		count = *(ID*) reader;
		reader += 4; //skip count;
		if (count > mcount) {
			mcount = count;
			sortBuf = (ID*) realloc(sortBuf, mcount * sizeof(ID));
		}
		memcpy(sortBuf, reader, count * sizeof(ID));
		//qsort((void*)sortBuf,count,sizeof(ID),qcomparedegree);
		qsort((void*) sortBuf, count, sizeof(ID), qcompareindegree);
		memcpy(reader, sortBuf, count * sizeof(ID));
		reader += count * 4; //skip to ID;

		whenflush += (8 + count * 4);

		if (whenflush > (2 << 27)) {
			adjMap[fileindex]->flush();
			whenflush = 0;
		}
	}

	adjMap[fileindex]->flush();
	free(sortBuf);
	cout << "sort:" << fileindex << " end..." << endl;
}

size_t Graph::loadFileinMemory(const char* filePath, char*& buf) {
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

void Graph::parallel_load_task(char *&buf, MemoryMappedFile *&temp,
		unsigned long cur_pos, size_t copy_size) {
	memcpy(buf + cur_pos, temp->getBegin() + cur_pos, copy_size);
}

void Graph::parallel_load_inmemory(const char* filePath, char*& buf) {
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
				boost::bind(&Graph::parallel_load_task, buf, temp, cur_pos, MEMCPY_SIZE));
		cur_pos += MEMCPY_SIZE;
	}
	if (left)
		CThreadPool::getInstance().AddTask(
				boost::bind(&Graph::parallel_load_task, buf, temp, cur_pos, left));
	CThreadPool::getInstance().Wait();

	temp->close();
	delete temp;
}

unsigned Graph::DFS_V(ID v) {
	s.push(v);
	unsigned off = 0;
	ID* addr = NULL;
	unsigned count = 0;

	while (!s.empty()) {
		int w = s.top();
		s.pop();
		if (isNewIDExist(w, off) == false) {
			setOld(w, idcount);
			setNew(idcount, w);
			idcount++;

			addr = (ID*) getOffset(w);
			if (addr == NULL)
				continue;
			assert(w == *(addr));
			count = *(addr + 1);
			addr += 2; //skip the id and count

			for (int i = 0; i < count; i++) {
				ID temp = *(addr + i);
				if (isNewIDExist(temp, off) == false) {
					s.push(temp);
				}
			}
		}
	}

	return idcount;
}

unsigned Graph::DFS_V(ID v, bool* &neighbors_visited, ID &newRootID) {
	s.push(v);
	unsigned off = 0;
	ID* addr = NULL;
	unsigned count = 0;

	while (!s.empty()) {
		int w = s.top();
		s.pop();
		if (isNewIDExist(w, off) == false) {
			setOld(w, idcount);
			setNew(idcount, w);
			idcount++;

			addr = (ID*) getOffset(w);
			if (addr == NULL)
				continue;
			assert(w == *(addr));
			count = *(addr + 1);
			addr += 2; //skip the id and count

			for (int i = 0; i < count; i++) {
				ID temp = *(addr + i);
				if (isNewIDExist(temp, off) == false) {
					setOld(temp, idcount);
					setNew(idcount, temp);
					idcount++;
					s.push(temp);
				}
			}
			neighbors_visited[w] = true;
		} else if (!neighbors_visited[w]) {
			addr = (ID*) getOffset(w);
			if (addr == NULL)
				continue;
			assert(w == *(addr));
			count = *(addr + 1);
			addr += 2; //skip the id and count

			for (int i = 0; i < count; i++) {
				ID temp = *(addr + i);
				if (isNewIDExist(temp, off) == false) {
					setOld(temp, idcount);
					setNew(idcount, temp);
					idcount++;
					s.push(temp);
				}
			}
			neighbors_visited[w] = true;
		}
	}

	newRootID = oldID_mapto_newID[v];
	return idcount;
}

void Graph::DFS(bool forward_or_backward) {
	cout << "in DFS Encode, maxID: " << maxID << endl;
	unsigned leftLimit = 10;
	idcount = 1;
	newID_mapto_oldID = (ID *)calloc(maxID + 1, sizeof(ID));
	oldID_mapto_newID = (ID *)calloc(maxID + 1, sizeof(ID));

	MemoryMappedFile *rootMap = new MemoryMappedFile();
	if(forward_or_backward == true)assert(rootMap->open((Dir+"/root.forward.0").c_str()));
	else assert(rootMap->open((Dir+"/root.backward.0").c_str()));
	unsigned rootNum = (rootMap->getEnd() - rootMap->getBegin()) / sizeof(ID);
	ID *root = (ID*) rootMap->getBegin();
	if (rootNum > 0)cout << "root:" << *(root) << "  rootNum:" << rootNum << endl;

	unsigned newNum = 0;
	unsigned i = 0;
	bool *neighbor_visited = (bool *) calloc((maxID + 1), sizeof(bool));

	ID newRootID = 0;
	TempFile *newRoot;
	if(forward_or_backward == true)newRoot= new TempFile(Dir + "/new_root.forward", 0);
	else newRoot= new TempFile(Dir + "/new_root.backward", 0);
	for (i = 0; i < rootNum; i++) {
		newNum = DFS_V(*(root + i), neighbor_visited, newRootID);
		newRoot->writeId(newRootID);
		if (newNum >= maxID - leftLimit)
			break;
	}
	cout << "root compute end" << endl;
	unsigned off = 0;

	if (newNum < maxID - leftLimit) {
		for (i = start_ID[0]; i <= maxID; i++) {
			if (zeroDegree(i) == false && isNewIDExist(i, off) == false) {
				newNum = DFS_V(i, neighbor_visited, newRootID);
				newRoot->writeId(newRootID);
				if (newNum >= maxID - leftLimit)
					break;
			}
		}

	}
	free(neighbor_visited);

	if (newNum >= maxID - leftLimit) {
		for (i = start_ID[0]; i <= maxID; i++) {
			if (zeroDegree(i) == false && isNewIDExist(i, off) == false) {
				setOld(i, newNum);
				setNew(newNum, i);
				newRoot->writeId(newNum);
				newNum++;
			}
		}
		cout << "newNum:" << newNum << "  maxID:" << maxID << endl;
	}

	cout << "newNum:" << newNum << "  maxID:" << maxID << endl;
	newRoot->flush();
	rootMap->close();
	delete newRoot;
	delete rootMap;
	remove((Dir+"/root.forward.0").c_str());
	remove((Dir+"/root.backward.0").c_str());
	rootMap = NULL;
	maxID = newNum - 1;

	TempFile *old_mapto_new = new TempFile(Dir + "/old_mapto_new", 0);
	old_mapto_new->write((maxID + 1) * sizeof(ID), (const char*) oldID_mapto_newID);
	old_mapto_new->close();
	delete old_mapto_new;
	old_mapto_new = NULL;

	TempFile *new_mapto_old = new TempFile(Dir + "/new_mapto_old", 0);
	new_mapto_old->write((maxID + 1) * sizeof(ID), (const char*) newID_mapto_oldID);
	new_mapto_old->close();
	delete new_mapto_old;
	new_mapto_old = NULL;

	free(newID_mapto_oldID);
	newID_mapto_oldID = NULL;
	cout << "end write file old_mapto_new and new_mapto_old, size: " << (maxID + 1) * sizeof(ID) << endl;
}

int compquick(const void *a, const void *b) {
	return *(int *) a - *(int *) b;
}

//------------------------------------------------------------------------------------
//--------------------------------------------following is topology encode----------------------------------------------------
void Graph::get_forward_root_vertices(ID maxID) {
	EntityIDBuffer* buffer = new EntityIDBuffer();
	buffer->setIDCount(1);

	//cout<<"root:"<<endl;
	for (ID start = 0; start <= maxID; start++) {
		if (!degree[start].indeg && degree[start].outdeg) {
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

void Graph::deal_with_cross_vertex(bool *&visited_arr, bool *cross_vertices_arr, ID crossid, TempFile &excp_id){
	deque<ID>* que = new deque<ID>();
	ID curID;
	unsigned count;

	visited_arr[crossid] = true;
	que->push_front(crossid);
	EntityIDBuffer* tempEnt = new EntityIDBuffer();
	EntityIDBuffer* NG_tempEnt = new EntityIDBuffer();

	while(!que->empty()){
		curID = que->front();
		que->pop_front();
		DAG_maxID = std::max(curID, DAG_maxID);

		visited_arr[curID] = true;

		oldID_mapto_newID[curID] = idcount;
		newID_mapto_oldID[idcount] = curID;
		idcount++;

		ID *addr = (ID*) DAG_getOffset(curID);
		if (addr == NULL){
			cout<<"curID:"<<curID<<" NULL"<<endl;
			continue;
		}
		if(curID != addr[0]) cout<<"curID:"<<curID<<" addr[0]"<<addr[0]<<endl;
		assert(curID == addr[0]);
		count = addr[1];
		addr += 2; //skip the id and count

		bool have_excp_id = false;
		for (size_t i = 0; i < count; i++) {
			DAG_maxID = std::max(addr[i], DAG_maxID);
			if(degree[addr[i]].indeg)degree[addr[i]].indeg--;
			if(cross_vertices_arr[addr[i]]){
				continue;
			}
			if(visited_arr[addr[i]])continue;
			if(degree[addr[i]].indeg){
				excp_count ++;
				excp_id.writeId(oldID_mapto_newID[curID]);
				excp_id.writeId(addr[i]);
				have_excp_id = true;
				continue;
			}

			que->push_front(addr[i]);
		}
		if(have_excp_id) {
			excp_idcount ++;
			excp_located[oldID_mapto_newID[curID]] = excp_idcount;
		}
	}

	delete que;
}

void Graph::cross_collector(ID *cross_id, unsigned cross_idcount, bool *cross_vertices_arr, bool *&visited_arr, TempFile &excp_id, TempFile &new_graph_root) {
	ID last_crossid = cross_id[0];
	unsigned count = 0;
	new_graph_root.writeId(cross_id[0]);
	deal_with_cross_vertex(visited_arr, cross_vertices_arr, cross_id[0], excp_id);
	count ++;
	for(unsigned i = 1; i < cross_idcount; i ++) {
		if(cross_id[i] != last_crossid) {
			new_graph_root.writeId(cross_id[i]);
			deal_with_cross_vertex(visited_arr, cross_vertices_arr, cross_id[i], excp_id);
			count ++;
			last_crossid = cross_id[i];
		}
	}
	cout<<"the number of cross id is:"<<count<<endl;
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

bool Graph::is_exceptions(ID id1, ID id2) {
	if(!excp_located[id1])return false;

	unsigned count;
	ID *addr = (ID*) excp_getOffset(excp_located[id1] - 1);
	//cout<<"id1:"<<oldID_mapto_newID[id1]<<"addr0:"<<addr[0]<<endl;
	assert(excp_located[id1] - 1 == addr[0]);
	count = addr[1];
	addr += 2; //skip the id and count
	assert(addr[0] == id1);
	for (size_t i = 1; i < count; i++){
		if(addr[i] == id2)
			return true;
	}
	return false;
}

/////啊
ID Graph::posttranv_tree(ID id, unsigned count, bool *cross_vertices_arr){
	unsigned ct;
	ID *addr = (ID*) DAG_getOffset(id);
	ID max = oldID_mapto_newID[id];
	if (addr == NULL) return max;
	assert(id == addr[0]);
	ct = addr[1];
	addr += 2; //skip the id and count

	ID *adj = (ID *)calloc(ct, sizeof(ID));
	for (size_t i = 0; i < ct; i++){
		//id_belong_tree[oldID_mapto_newID[addr[i]]] = count;
		if(! cross_vertices_arr[addr[i]] && ! is_exceptions(oldID_mapto_newID[id], oldID_mapto_newID[addr[i]])) {
			adj[i] = posttranv_tree(addr[i], count, cross_vertices_arr);
			idtag[oldID_mapto_newID[addr[i]]] = adj[i];
			//cout<<oldID_mapto_newID[addr[i]]<<"子标签："<<adj[i]<<endl;
		}
	}

	for(size_t i = 0; i < ct; i++){
		if(adj[i] > max)
			max = adj[i];
	}
	return max;
}

void Graph::mark_vertices_tag(TempFile &new_graph_root, bool *cross_vertices_arr){
	ID id;
	unsigned count = 0;
	MemoryMappedFile mappedIn;
	assert(mappedIn.open(new_graph_root.getFile().c_str()));
	const char* reader = mappedIn.getBegin(), *begin = reader, *limit = mappedIn.getEnd();

	while (reader < limit){
		id = *(ID*) reader;
		//id = oldID_mapto_newID[id];
		reader += sizeof(ID);
		//id = newGraph_id[count];
		//cout<<"newGraph:"<<id<<endl;
		idtag[oldID_mapto_newID[id]] = posttranv_tree(id, count, cross_vertices_arr);
		//id_belong_tree[id] = count;
		count ++;
		//cout<<id<<"根标签："<<idtag[id]<<endl;
	}

	mappedIn.close();
}

//----------------------------Following is the NL code by 双双-----------------------------------------------
void Graph::get_old_root_vertices(ID maxID) {
	EntityIDBuffer* buffer = new EntityIDBuffer();
	buffer->setIDCount(1);

	//cout<<"root:"<<endl;
	for (ID start = 0; start <= maxID; start++) {
		if (!degree[start].indeg && degree[start].outdeg) {
			buffer->insertID(start);
			//cout<<"start:"<<start<<" ";
		}
	}
	//cout<<endl;
	qsort((void*) buffer->getBuffer(), buffer->getSize(), 4, qcompareoutdegree);
	if (buffer->getSize() == 0)buffer->insertID(0);
	cout << "old root number: " << buffer->getSize() << endl;
	TempFile* rootFile = new TempFile(Dir + "/root.forward", 0);
	rootFile->write((size_t) buffer->getSize() * sizeof(ID),(const char*) buffer->getBuffer());
	rootFile->flush();
	delete rootFile;
	cout << "finish writing rootFile" << endl;
	delete buffer;
}

void Graph::get_backward_root_vertices(ID maxID) {
	EntityIDBuffer* buffer = new EntityIDBuffer();
	buffer->setIDCount(1);

	//cout<<"root:"<<endl;
	for (ID start = 0; start <= maxID; start++) {
		if (degree[start].indeg && !degree[start].outdeg) {
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

ID *Graph::get_unvisited_neighbors(ID curID, bool* visited_arr, ID *&tmpvalue_arr, ID component) {
	unsigned count;
	ID *addr = (ID*) getOffset(curID);
	if (addr == NULL)return NULL;
	//cout<<"curID:"<<curID<<" addr[0]:"<<addr[0]<<endl;
	assert(curID == addr[0]);
	count = addr[1];
	addr += 2; //skip the id and count
	for(size_t i = 0; i < count; i++) {
		if(!visited_arr[addr[i]]) {
			tmpvalue_arr[addr[i]] = component;
			return &addr[i];
		}
//		else if(visited_arr[addr[i]] && tmpvalue_arr[addr[i]] != component) {
//			cross_vertices_arr[addr[i]] = true;
//			//newGraph_idcount ++;
//			continue;
//		}
	}
	return NULL;
}

ID *Graph::get_unvisited_neighbors(ID curID, bool* visited_arr) {
	unsigned count;
	ID *addr = (ID*) getOffset(curID);
	if (addr == NULL)return NULL;
	//cout<<"curID:"<<curID<<" addr[0]:"<<addr[0]<<endl;
	assert(curID == addr[0]);
	count = addr[1];
	addr += 2; //skip the id and count
	for(size_t i = 0; i < count; i++) {
		if(!visited_arr[addr[i]])
			return &addr[i];
	}
	return NULL;
}

void Graph::DFS_V(ID* root_arr, unsigned rootNum, ID *&scc_tag, bool* &visited_arr){
	deque<ID>* que = new deque<ID>();
	ID curID, component;

	//unsigned idcount = 1;

	//string fileName = string(path+"/tmpvalue");
	//MMapBuffer *tmpvalue = new MMapBuffer(fileName.c_str(), maxID * sizeof(ID));
	ID *tmpvalue_arr = (ID *) calloc((maxID + 1), sizeof(ID));

	for(size_t index = 0;index < rootNum;index++){
		if(zeroDegree(root_arr[index]) || visited_arr[root_arr[index]])continue;
		visited_arr[root_arr[index]] = true;
		que->push_back(root_arr[index]);
		//newGraph_idcount ++;
		component = root_arr[index];
		tmpvalue_arr[root_arr[index]] = component;

		while(!que->empty()){
			curID = que->back();

			ID* unvisited_neighbors = get_unvisited_neighbors(curID, visited_arr, tmpvalue_arr, component);

			if(unvisited_neighbors == NULL){
				scc_tag[ID_count] = curID;
				ID_count ++;
				que->pop_back();
			}else {
				visited_arr[*unvisited_neighbors] = true;
				que->push_back(*unvisited_neighbors);
				//tmpvalue_arr[*unvisited_neighbors] = component;
			}
		}
	}
	cout<<endl;

	//cout<<"idcount为："<<idcount<<endl;
	//remove(fileName.c_str());
	//delete tmpvalue;
	delete que;

	//return idcount;
}

void Graph::DFS_V(ID id, ID *&scc_tag, bool *&visited_arr){
	deque<ID>* que = new deque<ID>();
	ID curID;

	visited_arr[id] = true;
	que->push_back(id);

	while(!que->empty()){
		curID = que->back();

		ID* unvisited_neighbors = get_unvisited_neighbors(curID, visited_arr);
		if(unvisited_neighbors == NULL){
			scc_tag[ID_count] = curID;
			ID_count ++;
			que->pop_back();
		}else {
			//cout<<" "<<*unvisited_neighbors<<":";
			visited_arr[*unvisited_neighbors] = true;
			que->push_back(*unvisited_neighbors);
		}
	}
	delete que;
	//return idcount;
}

void Graph::find_max_scc_tag(ID *scc_tag, bool* visited_arr) {
	for(int i = ID_count; i >= 0; i --) {
		if(!visited_arr[scc_tag[i]]){
			//cout<<"find_max:"<<scc_tag[i]<<":"<<i<<endl;
			ID_count = i;
			return;
		}
	}
	ID_count = -1;
}

bool Graph::reverse_DFS_V(ID id, bool* &visited_arr, EntityIDBuffer*& tempEnt) {
	deque<ID>* que = new deque<ID>();
	ID curID;
	unsigned count, at = 0;
	//cout<<"dfs"<<" find id:";

	visited_arr[id] = true;
	//cout<<id<<" ";
	que->push_back(id);
	bool tag = false;
	//scc_ID[scc_num_count].scc_id = (ID*) calloc(ID_count, sizeof(ID));
	while(at < que->size()){
		curID = que->at(at);
		at ++;
		//cout<<curID<<" ";
		//que->pop_front();
		ID *addr = (ID*) reverse_getOffset(curID);
		if (addr == NULL)continue ;
		//cout<<"curID:"<<curID<<" "<<addr[0]<<endl;
		assert(curID == addr[0]);
		count = addr[1];
		addr += 2; //skip the id and count
		for(size_t i = 0; i < count; i++) {
			if(!visited_arr[addr[i]]){
				que->push_back(addr[i]);
				visited_arr[addr[i]] = true;
				//cout<<addr[i]<<" ";
				tag = true;
			}
		}
	}
	if(tag == true){
		ID first_id = que->front();
		if(first_id == 0) first_id = que->back();
		tempEnt->insertID(first_id);
		tempEnt->insertID(0); // for num
		scc_start_ID[scc_num] = first_id;
		while(!que->empty()){
			ID id = que->front();
		//	is_scc_vertices_arr[id] = true;
			que->pop_front();
			tempEnt->insertID(id);
			scc_located[id] = first_id;
			scc_num_located[id] = scc_num + 1;
			degree[id].indeg = 0;
			globalDegree[id].indeg = 0;
		}

		//scc_num ++;
	}
	delete que;
	if(tag == true)return true;
	else return false;

}



bool Graph::find_cycles(ID id, bool *&visited_arr, bool *cross_vertices_arr) {
	ID *scc_addr = (ID*) scc_getOffset(scc_located[id]);
	if (scc_addr == NULL){
		cout<<"null"<<endl;
		return true ;
	}
	assert(scc_located[id] == scc_addr[0]);
	unsigned count = scc_addr[1];
	scc_addr += 2;
	bool tag = false;

	for(size_t i = 0; i < count; i++) {


		if(cross_vertices_arr[scc_addr[i]])
			tag =true;
	}
	if(tag == true)
		return false;
	else return true;
}

void Graph::DFS_Sec(string path, ID *&scc_tag, bool *&visited_arr){
	//print_adj();
	//print_reverse_adj();

	bool firstTime = true;
	int fileindex = -1;
	unsigned fileOff = 0;
	EntityIDBuffer* tempEnt = new EntityIDBuffer();
	vector<TempFile*> sccFile;
	size_t pageSize = (ID_count + 1) * sizeof(ID) / (MemoryBuffer::pagesize) +
			((((ID_count + 1) * sizeof(ID)) % (MemoryBuffer::pagesize)) ? 1 : 0);
	char scc_indexPath[150];
	sprintf(scc_indexPath, "%s/global_scc_index.0", Dir.c_str());
	MMapBuffer *global_scc_indexFile = new MMapBuffer(scc_indexPath, 2 * pageSize * MemoryBuffer::pagesize);
	unsigned *global_scc_index = (unsigned *)global_scc_indexFile->get_address();

	scc_num = 0;
	scc_start_ID = (ID *)calloc(maxID + 1, sizeof(ID));
	scc_located = (ID *)calloc(maxID + 1, sizeof(ID));
	scc_num_located = (ID *)calloc(maxID + 1, sizeof(ID));
//	cout<<"ID_count:"<<ID_count<<endl;
//	for(size_t i = 1; i <= ID_count; i++){
//		cout<<scc_tag[i]<<":"<<i<<" ";
//	}
//	cout<<endl;

	find_max_scc_tag(scc_tag, visited_arr);
	deque<ID>* que  = new deque<ID>();
	bool have_scc = false;
	//unsigned sccC = 0;
	//return ;
	unsigned idcount = 0;
	while(ID_count >= 0){
		//cout<<"find maxid "<<*id<<endl;
		tempEnt->empty();
		bool tag = reverse_DFS_V(scc_tag[ID_count], visited_arr, tempEnt);
		//cout<<"scc_num:"<<scc_num<<endl;
		idcount ++;
		if(tag){
			//存-------------------scc----------------------------
			have_scc = true;
//			ID first_id = que->front();
//			if(first_id == 0) first_id = que->back();
//			tempEnt->empty();
//			tempEnt->insertID(first_id);
//			tempEnt->insertID(0); // for num
//			scc_start_ID[scc_num - 1] = first_id;
//			while(!que->empty()){
//				ID id = que->front();
//			//	is_scc_vertices_arr[id] = true;
//				que->pop_front();
//				tempEnt->insertID(id);
//				scc_located[id] = first_id;
//				degree[id].indeg = 0;
//				globalDegree[id].indeg = 0;
//			}

			unsigned num = tempEnt->getSize() - 2;
			tempEnt->getBuffer()[1] = num; //update num
			unsigned size = tempEnt->getSize() * sizeof(ID);
			if (!firstTime && fileOff + size < (unsigned) (-1) / 8) {
				sccFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
				global_scc_index[2 * scc_num] = fileOff;
				global_scc_index[2 * scc_num + 1] = fileindex;
				fileOff += size;
			} else {
				if (fileindex >= 0) {
					sccFile[fileindex]->close();
				}

				fileindex++;
				fileOff = 0;
				sccFile.push_back(new TempFile(Dir + "/scc", fileindex));
				//scc_start_ID[fileindex] = first_id;
				sccFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
				global_scc_index[2 * scc_num] = 1;//actually is '0', but we set '1' here, for '0' has been used to mark the unexist 'to data'
				global_scc_index[2 * scc_num + 1] = fileindex;
				fileOff += size;
				firstTime = false;

				//cout << "fileIndex: " << fileindex << ", from minID " << to<< " to maxID " << maxID << endl;
			} //else
			//存-------------------scc----------------------------
			scc_num ++;
		}
		find_max_scc_tag(scc_tag, visited_arr);
	}
	cout<<"scc num:"<<idcount<<"   scc:"<<scc_num<<endl;
	if(have_scc) {
		//cout<<"write over"<<endl;
		sccFile[fileindex]->close();
		global_scc_indexFile->flush();
		//cout<<"init_scc begin:"<<endl;
		init_scc();
		//cout<<"init_scc end:"<<endl;
	}
	//print_scc();

	TempFile *scc_start = new TempFile(path + "/scc_start_ID", 0);
	scc_start->write((scc_num) * sizeof(ID), (const char*) scc_start_ID);
	scc_start->close();
	delete scc_start;
	//free(scc_start_ID);
	//scc_start_ID = NULL;

	for(int i = 0;i < sccFile.size();i++){
		delete sccFile[i];
		sccFile[i] = NULL;
	}
	sccFile.clear();
	delete global_scc_indexFile;

	delete tempEnt;
	delete que;
}

void Graph::DFS_Fst(string path, ID *&scc_tag, bool *&visited_arr) {
	get_old_root_vertices(maxID);
	MemoryMappedFile *rootMap = new MemoryMappedFile();
	assert(rootMap->open((Dir+"/root.forward.0").c_str()));
	unsigned rootNum = (rootMap->getEnd() - rootMap->getBegin()) / sizeof(ID);
	ID *root = (ID*) rootMap->getBegin();
	if (rootNum > 0)cout << "root:" << *(root) << "  rootNum:" << rootNum << endl;

	ID_count = 0;

	DFS_V(root, rootNum, scc_tag, visited_arr);

	//cout<<"idcount:"<<ID_count<<endl;
	for (ID i = start_ID[0]; i <= maxID; i++) {
		if (zeroDegree(i) == false && !visited_arr[i]) {
			//ID count = idcount;
			//cout<<i<<"degree:"<<degree[i].indeg<<" "<<degree[i].outdeg;
			DFS_V(i, scc_tag, visited_arr);
			//cout<<" count:"<<idcount - count<<endl;
		}
	}

//	unsigned count = 0;
//	//cout<<endl<<"zeroID:";
//	for(ID i = start_ID[0]; i <= maxID; i++){
//		if(zeroDegree(i) == true){
//			count ++;
//			//cout<<i<<" ";
//		}
//	}
//	cout<<endl;
//	cout<<"zeroDegree:"<<count<<endl;
	ID_count = ID_count - 1;
	cout<<"maxID:"<<maxID<<"  idcount:"<<ID_count + 1<<endl;
//	for(size_t i = 1; i <= ID_count; i++){
//		cout<<scc_tag[i]<<":"<<i<<" ";
//	}
//	cout<<endl;

	rootMap->close();
	delete rootMap;
	rootMap = NULL;
}

bool Graph::is_new_root(ID root_id, ID *oldroot_arr, unsigned oldrootNum) {
	unsigned i = 0;
	while(root_id > oldroot_arr[i]) {
		i ++;
	}
	if(i >= oldrootNum) return false;
	else if(root_id == oldroot_arr[i])
		return true;
	else return false;
}

void Graph::construct_excp_id(TempFile &excp_id) {
	ID from, to, lastfrom;
	unsigned count = 0;
	MemoryMappedFile mappedIn;
	assert(mappedIn.open(excp_id.getFile().c_str()));
	const char* reader = mappedIn.getBegin(), *limit = mappedIn.getEnd();

	bool firstTime = true, firstInsert = true;
	int fileindex = -1;
	unsigned fileOff = 0;
	EntityIDBuffer* tempEnt = new EntityIDBuffer();
	vector<TempFile*> excpFile;
	size_t pageSize = excp_idcount * sizeof(ID) / (MemoryBuffer::pagesize) + (((excp_idcount * sizeof(ID)) % (MemoryBuffer::pagesize)) ? 1 : 0);
	char excp_indexPath[150];
	sprintf(excp_indexPath, "%s/global_excp_index.0", Dir.c_str());
	MMapBuffer *global_excp_indexFile = new MMapBuffer(excp_indexPath, 2 * pageSize * MemoryBuffer::pagesize);
	unsigned *global_excp_index = (unsigned *)global_excp_indexFile->get_address();

	TripleBitBuilder::loadTwo(reader, from, to);
	lastfrom = from;
	//cout<<"write"<<endl;
	while (reader < limit) {
		TripleBitBuilder::loadTwo(reader, from, to);
		to = oldID_mapto_newID[to];
		reader = TempFile::skipIdId(reader);

		//excp_located[to] = excp_located[from];

		if (firstInsert) {
			tempEnt->insertID(count);
			tempEnt->insertID(0); // for num
			tempEnt->insertID(from);
			tempEnt->insertID(to);
			firstInsert = false;
		} else if (from != lastfrom) {
			unsigned num = tempEnt->getSize() - 2;
			tempEnt->getBuffer()[1] = num; //update num
			unsigned size = tempEnt->getSize() * sizeof(ID);
			if (!firstTime && fileOff + size < (unsigned) (-1) / 8) {
				excpFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
				global_excp_index[2 * count] = fileOff;
				global_excp_index[2 * count + 1] = fileindex;
				fileOff += size;
			} else {
				if (fileindex >= 0) {
					excpFile[fileindex]->close();
				}

				fileindex++;
				fileOff = 0;
				excpFile.push_back(new TempFile(Dir + "/exceptions", fileindex));
				//reverse_start_ID[fileindex] = lastto;
				excpFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
				global_excp_index[2 * count] = 1;//actually is '0', but we set '1' here, for '0' has been used to mark the unexist 'to data'
				global_excp_index[2 * count + 1] = fileindex;
				fileOff += size;
				firstTime = false;
			}
			count ++;
			//excp_located[lastfrom] = count;

			tempEnt->empty();
			tempEnt->insertID(count);
			tempEnt->insertID(0);
			tempEnt->insertID(from);
			tempEnt->insertID(to);
			lastfrom = from;
		} else {
			tempEnt->insertID(to);
		}
	}

	unsigned num = tempEnt->getSize() - 2;
	tempEnt->getBuffer()[1] = num; //update num
	unsigned size = tempEnt->getSize() * sizeof(ID);
	if (!firstTime && fileOff + size < (unsigned) (-1) / 8) {
		excpFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
		global_excp_index[2 * count] = fileOff;
		global_excp_index[2 * count + 1] = fileindex;
		fileOff += size;
	} else {
		if (fileindex >= 0) {
			excpFile[fileindex]->close();
		}

		fileindex++;
		fileOff = 0;
		excpFile.push_back(new TempFile(Dir + "/exceptions", fileindex));
		//reverse_start_ID[fileindex] = lastto;
		excpFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
		global_excp_index[2 * count] = 1;//actually is '0', but we set '1' here, for '0' has been used to mark the unexist 'to data'
		global_excp_index[2 * count + 1] = fileindex;
		fileOff += size;
		firstTime = false;
	}
	count ++;
	//excp_located[lastfrom] = count;

	excpFile[fileindex]->close();
	global_excp_indexFile->flush();

	init_excp();
	mappedIn.close();
	//cout<<"the number of crossid is: "<<crossid_count<<endl;
	for(int i = 0;i < excpFile.size();i++){
		delete excpFile[i];
		excpFile[i] = NULL;
	}
	excpFile.clear();
	delete global_excp_indexFile;
	delete tempEnt;
}

void Graph::deal_with_excp(TempFile &new_graph_root, bool *&cross_vertices_arr, unsigned &cross_idcount) {
	for(unsigned i = 0; i < excp_idcount; i ++) {
		ID *addr = (ID *)excp_getOffset(i);
		assert(i == addr[0]);
		unsigned count = addr[1];
		addr += 2;
		for(size_t k = 1; k < count; k ++) {
			if((idtag[addr[0]] < idtag[addr[k]] || addr[0] > addr[k]) && ! cross_vertices_arr[newID_mapto_oldID[addr[k]]]) {
				cross_vertices_arr[newID_mapto_oldID[addr[k]]] = true;
				new_graph_root.writeId(newID_mapto_oldID[addr[k]]);
				cross_idcount ++;
				newGraph_idcount ++;
			}
		}
	}
}

void Graph::construct_newGraph(ID *root_arr, unsigned rootNum, TempFile &new_graph, TempFile &new_graph_root, bool *cross_vertices_arr, bool *visited_arr, unsigned cross_idcount) {
	deque<ID>* que = new deque<ID>();
	ID curID;
	bool have_cross_vertices = false;
	newGraph_id = (ID *)calloc(newGraph_idcount, sizeof(ID));
	unsigned ct = 0;
	idcount = 1;
//	for(size_t i = 0; i <= maxID; i ++){
//		cout<<i<<"degree:"<<globalDegree[i].indeg<<" "<<globalDegree[i].outdeg<<endl;
//	}

	if(rootNum == 1 && (!globalDegree[root_arr[0]].outdeg || globalDegree[root_arr[0]].indeg))
		cout<<"don't have any root vertices!"<<endl;
	else {
		for(size_t index = 0;index < rootNum;index++){
			if(visited_arr[root_arr[index]])continue;

			que->push_front(root_arr[index]);
			newGraph_id[ct] = idcount;

			while(!que->empty()){
				curID = que->front();
				que->pop_front();

				visited_arr[curID] = true;
				oldID_mapto_newID[curID] = idcount;
				newID_mapto_oldID[idcount] = curID;
				id_belong_tree[idcount] = ct;
				idcount++;

				ID *addr = (ID*) DAG_getOffset(curID);
				if (addr == NULL){
					cout<<"DAG_getOffset NULL id:"<<curID<<endl;
					continue;
				}

				assert(curID == addr[0]);
				unsigned count = addr[1];
				addr += 2; //skip the id and count

				for (size_t i = 0; i < count; i++) {
					if(globalDegree[addr[i]].indeg)globalDegree[addr[i]].indeg--;
					if(cross_vertices_arr[addr[i]]){
						new_graph.writeId(root_arr[index]);
						new_graph.writeId(addr[i]);
						have_cross_vertices = true;
						continue;
					}
					if(visited_arr[addr[i]])continue;
					if(globalDegree[addr[i]].indeg){
						continue;
					}//degree if
					que->push_front(addr[i]);
				}//for
			} //while que
			ct ++;
		} //for index
	} //else

	MemoryMappedFile mappedIn;
	assert(mappedIn.open(new_graph_root.getFile().c_str()));
	const char* reader = mappedIn.getBegin(), *begin = reader, *limit = mappedIn.getEnd();

	//cout<<"new_graph_root id:";
	ID* crossid = (ID *)calloc(cross_idcount, sizeof(ID));
	//unsigned* crossid_deg = (unsigned *)calloc(cross_idcount, sizeof(unsigned));
	unsigned cross_count = 0;
	if(have_cross_vertices == true) {
		while (reader < limit){
			ID id = *(ID*) reader;
			reader += sizeof(ID);
			if(! visited_arr[id]) {
				crossid[cross_count] = id;
				//crossid_deg[cross_count] = globalDegree[id].indeg;
				cross_count ++;
			}
		} //while reader

		bool finished = false;
		while(! finished){
			qsort(crossid, cross_idcount, sizeof(ID), qcompareindegree);
			cross_count = 0;
			while(cross_count < cross_idcount && ! globalDegree[crossid[cross_count]].indeg) {
				if(! visited_arr[crossid[cross_count]]) {
					que->push_front(crossid[cross_count]);
					newGraph_id[ct] = idcount;

					while(!que->empty()){
						curID = que->front();
						que->pop_front();

						visited_arr[curID] = true;

						oldID_mapto_newID[curID] = idcount;
						newID_mapto_oldID[idcount] = curID;
						id_belong_tree[idcount] = ct;
						idcount++;

						ID *addr = (ID*) DAG_getOffset(curID);
						if (addr == NULL){
							cout<<"curID:"<<curID<<" NULL"<<endl;
							continue;
						}

						assert(curID == addr[0]);
						unsigned count = addr[1];
						addr += 2; //skip the id and count

						for (size_t i = 0; i < count; i++) {
							if(globalDegree[addr[i]].indeg)globalDegree[addr[i]].indeg--;
							if(cross_vertices_arr[addr[i]]){
								new_graph.writeId(crossid[cross_count]);
								new_graph.writeId(addr[i]);
								continue;
							}
							if(visited_arr[addr[i]])continue;
							if(globalDegree[addr[i]].indeg){
								continue;
							}
							que->push_front(addr[i]);
						}
					} //while que empty
					ct ++;
				}
				cross_count ++;
			} //while indeg
			if(cross_count == cross_idcount)
				finished =true;
		} //while finished

	} //have cross
	idcount --;

	mappedIn.close();
	delete que;
	free(crossid);
}

void Graph::construct_newGraph_ass(ID *root_arr, unsigned rootNum, TempFile &new_graph_root, bool *cross_vertices_arr, bool *visited_arr, unsigned cross_idcount) {
	deque<ID>* que = new deque<ID>();
	ID curID;
	bool have_cross_vertices = false;
	ID idcount = 1;
	newID_mapto_newIDass = (ID *)calloc(maxID + 2, sizeof(ID));

	if(rootNum == 1 && (!globalDegree_ass[root_arr[0]].outdeg || globalDegree_ass[root_arr[0]].indeg))
		cout<<"don't have any root vertices!"<<endl;
	else {
		for(int index = rootNum - 1;index >= 0;index --){
			if(visited_arr[root_arr[index]])continue;

			que->push_front(oldID_mapto_newID[root_arr[index]]);

			while(!que->empty()){
				curID = que->front();
				que->pop_front();

				visited_arr[newID_mapto_oldID[curID]] = true;
				newID_mapto_newIDass[curID] = idcount;
				idcount++;

				ID *addr = (ID*) DAG_getOffset(newID_mapto_oldID[curID]);
				if (addr == NULL){
					cout<<"DAG_getOffset NULL id:"<<curID<<endl;
					continue;
				}

				assert(newID_mapto_oldID[curID] == addr[0]);
				unsigned count = addr[1];
				addr += 2; //skip the id and count

				for (size_t i = 0; i < count; i++) {
					if(globalDegree_ass[addr[i]].indeg)globalDegree_ass[addr[i]].indeg--;
					if(cross_vertices_arr[addr[i]]){
						have_cross_vertices = true;
						continue;
					}
					if(visited_arr[addr[i]])continue;
					if(globalDegree_ass[addr[i]].indeg){
						continue;
					}//degree if
					que->push_front(oldID_mapto_newID[addr[i]]);
				}//for
			} //while que
		} //for index
	} //else

	MemoryMappedFile mappedIn;
	assert(mappedIn.open(new_graph_root.getFile().c_str()));
	const char* reader = mappedIn.getBegin(), *begin = reader, *limit = mappedIn.getEnd();

	//cout<<"new_graph_root id:";
	ID* crossid = (ID *)calloc(cross_idcount, sizeof(ID));
	//unsigned* crossid_deg = (unsigned *)calloc(cross_idcount, sizeof(unsigned));
	unsigned cross_count = 0;
	if(have_cross_vertices == true) {
		while (reader < limit){
			ID id = *(ID*) reader;
			reader += sizeof(ID);
			if(! visited_arr[id]) {
				crossid[cross_count] = id;
				//crossid_deg[cross_count] = globalDegree[id].indeg;
				cross_count ++;
			}
		} //while reader

		bool finished = false;
		while(! finished){
			qsort(crossid, cross_idcount, sizeof(ID), qcompare_ass_indegree);
			int limit = 0;
			while(! globalDegree_ass[crossid[limit]].indeg) {
				limit ++;
				if(limit == cross_idcount)
					break;
			}

			limit --;
			finished = true;
			while(limit >= 0) {
				if(! visited_arr[crossid[limit]]) {
					finished = false;
					que->push_front(oldID_mapto_newID[crossid[limit]]);

					while(!que->empty()){
						curID = que->front();
						que->pop_front();

						visited_arr[newID_mapto_oldID[curID]] = true;

						newID_mapto_newIDass[curID] = idcount;
						idcount++;

						ID *addr = (ID*) DAG_getOffset(newID_mapto_oldID[curID]);
						if (addr == NULL){
							cout<<"curID:"<<curID<<" NULL"<<endl;
							continue;
						}

						assert(newID_mapto_oldID[curID] == addr[0]);
						unsigned count = addr[1];
						addr += 2; //skip the id and count

						for (size_t i = 0; i < count; i++) {
							if(globalDegree_ass[addr[i]].indeg)globalDegree_ass[addr[i]].indeg--;
							if(cross_vertices_arr[addr[i]]){
								continue;
							}
							if(visited_arr[addr[i]])continue;
							if(globalDegree_ass[addr[i]].indeg){
								continue;
							}
							que->push_front(oldID_mapto_newID[addr[i]]);
						}
					} //while que empty
				}
				limit --;
			} //while indeg
		} //while finished

	} //have cross
	idcount --;

	mappedIn.close();
	delete que;
	free(crossid);
}

void Graph::construct_newGraph_adj(TempFile &new_graph, TempFile &new_graph_path, bool *cross_vertices_arr) {
	ID from, to, lastfrom;
	unsigned count;
	MemoryMappedFile mappedIn;
	assert(mappedIn.open(new_graph.getFile().c_str()));
	const char* reader = mappedIn.getBegin(), *limit = mappedIn.getEnd();
	TripleBitBuilder::loadTwo(reader, from, to);
	lastfrom = oldID_mapto_newID[from];
	vector< vector<ID> > data(newGraph_idcount);
	count = 0;
	for(unsigned i = 0; i < id_belong_tree[lastfrom]; i++) {
		data[count].push_back(newGraph_id[i]);
		count ++;
	}
	bool first = true;
	while (reader < limit) {
		TripleBitBuilder::loadTwo(reader, from, to);
		from = oldID_mapto_newID[from];
		to = oldID_mapto_newID[to];
		reader = TempFile::skipIdId(reader);

		if(first){
			data[count].push_back(from);
			data[count].push_back(to);
			first = false;
		} else if(from != lastfrom) {
			count ++;
			if(newGraph_id[count] != from){
				for(; count < id_belong_tree[from]; count ++) {
					data[count].push_back(newGraph_id[count]);
				}
			}

			data[count].push_back(from);
			data[count].push_back(to);
			lastfrom = from;
		}
		else {
			data[count].push_back(to);
		}
	} //while reader<limit
	count ++;
	if(count != newGraph_idcount){
		for(; count < newGraph_idcount; count ++) {
			data[count].push_back(newGraph_id[count]);
		}
	}
	cout<<"copy end"<<endl;

//	for(int i = newGraph_idcount - 1; i >= 0; i --) { //根不考虑
//		if(!cross_vertices_arr[newID_mapto_oldID[data[i][0]]]) continue;
//
//		unsigned len = data[i].size();
//		for(unsigned j = 1; j < len; j ++) {
//			//ID id = data[i][j];
//			unsigned id_belong = id_belong_tree[data[i][j]];
//			//assert(id == data[id_belong][0]);
//			for(unsigned k = 1; k < data[id_belong].size(); k ++) {
//				data[i].push_back(data[id_belong][k]);
//			}
//		}
//	}
//	cout<<"collect end"<<endl;

//	for(int i = 0; i < newGraph_idcount; i ++) {
//		unsigned len = data[i].size();
//		for(unsigned j = 1; j < len; j ++) {
//			new_graph_path.writeId(data[i][0]);
//			new_graph_path.writeId(data[i][j]);
//		}
//	}
//	cout<<"write end"<<endl;
//
//	TempFile* sortFile = new TempFile("sortFile");
//	Sorter::sort(new_graph_path, *sortFile, TempFile::skipIdId, TempFile::compare12);

	cout<<"write adj"<<endl;
	//wtrite adj
	bool firstTime = true, firstInsert = true;
	int fileindex = -1;
	unsigned fileOff = 0;
	EntityIDBuffer* tempEnt = new EntityIDBuffer();
	vector<TempFile*> excpFile;
	size_t pageSize = newGraph_idcount * sizeof(ID) / (MemoryBuffer::pagesize) + (((newGraph_idcount * sizeof(ID)) % (MemoryBuffer::pagesize)) ? 1 : 0);
	char excp_indexPath[150];
	sprintf(excp_indexPath, "%s/global_newGraph_index.0", Dir.c_str());
	MMapBuffer *global_excp_indexFile = new MMapBuffer(excp_indexPath, 2 * pageSize * MemoryBuffer::pagesize);
	unsigned *global_excp_index = (unsigned *)global_excp_indexFile->get_address();

	//qsort(cross_id, cross_idcount, sizeof(ID), qcompare);
	newGraph_degree = (Degree*) calloc(newGraph_idcount, sizeof(Degree));
	assert(newGraph_degree);
	ID lastto;
	for(int i = 0; i < newGraph_idcount; i ++) {
		unsigned len = data[i].size();
		sort(data[i].begin(), data[i].end());
		//cout<<"graph:"<<newGraph_id[i]<<" data:"<<data[i][0]<<endl;
		tempEnt->empty();
		tempEnt->insertID(data[i][0]);
		tempEnt->insertID(0);
		if(len > 1){
			tempEnt->insertID(data[i][1]);
			lastto = data[i][1];

			newGraph_degree[i].outdeg ++;
			newGraph_degree[id_belong_tree[data[i][1]]].indeg ++;
		}
		for(unsigned k = 2; k < len; k ++) {
			if(data[i][k] != lastto){
				tempEnt->insertID(data[i][k]);
				lastto = data[i][k];
				newGraph_degree[i].outdeg ++;
				newGraph_degree[id_belong_tree[data[i][k]]].indeg ++;
			}
		}

		unsigned num = tempEnt->getSize() - 2;
		tempEnt->getBuffer()[1] = num; //update num
		unsigned size = tempEnt->getSize() * sizeof(ID);
		if (!firstTime && fileOff + size < (unsigned) (-1) / 8) {
			excpFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
			global_excp_index[2 * i] = fileOff;
			global_excp_index[2 * i + 1] = fileindex;
			fileOff += size;
		} else {
			if (fileindex >= 0) {
				excpFile[fileindex]->close();
			}

			fileindex++;
			fileOff = 0;
			excpFile.push_back(new TempFile(Dir + "/newGraph", fileindex));
			//reverse_start_ID[fileindex] = lastto;
			excpFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
			global_excp_index[2 * i] = 1;//actually is '0', but we set '1' here, for '0' has been used to mark the unexist 'to data'
			global_excp_index[2 * i + 1] = fileindex;
			fileOff += size;
			firstTime = false;
		}
	}

//	assert(mappedIn.open(sortFile->getFile().c_str()));
//	reader = mappedIn.getBegin(), limit = mappedIn.getEnd();
//	TripleBitBuilder::loadTwo(reader, from, to);
//	lastfrom = from;
//	ID lastto = to;
//	count = 0;
//
//	for(unsigned i = 0; i < id_belong_tree[lastfrom]; i++) {
//		tempEnt->empty();
//		tempEnt->insertID(newGraph_id[i]);
//		tempEnt->insertID(0); // for num
//
//		unsigned num = tempEnt->getSize() - 2;
//		tempEnt->getBuffer()[1] = num; //update num
//		unsigned size = tempEnt->getSize() * sizeof(ID);
//		if (!firstTime && fileOff + size < (unsigned) (-1) / 8) {
//			excpFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
//			global_excp_index[2 * count] = fileOff;
//			global_excp_index[2 * count + 1] = fileindex;
//			fileOff += size;
//		} else {
//			if (fileindex >= 0) {
//				excpFile[fileindex]->close();
//			}
//
//			fileindex++;
//			fileOff = 0;
//			excpFile.push_back(new TempFile(Dir + "/newGraph", fileindex));
//			//reverse_start_ID[fileindex] = lastto;
//			excpFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
//			global_excp_index[2 * count] = 1;//actually is '0', but we set '1' here, for '0' has been used to mark the unexist 'to data'
//			global_excp_index[2 * count + 1] = fileindex;
//			fileOff += size;
//			firstTime = false;
//		}
//		count ++;
//	}
//	tempEnt->empty();
//
//	while (reader < limit) {
//		TripleBitBuilder::loadTwo(reader, from, to);
//		reader = TempFile::skipIdId(reader);
//
//		//cout<<"from:"<<from<<" to:"<<to<<" lastto:"<<lastto<<endl;
//		if (firstInsert) {
//			tempEnt->insertID(from);
//			tempEnt->insertID(0); // for num
//			tempEnt->insertID(to);
//			firstInsert = false;
//		} else if (from != lastfrom) {
//			unsigned num = tempEnt->getSize() - 2;
//			tempEnt->getBuffer()[1] = num; //update num
//			unsigned size = tempEnt->getSize() * sizeof(ID);
//			if (!firstTime && fileOff + size < (unsigned) (-1) / 8) {
//				excpFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
//				global_excp_index[2 * count] = fileOff;
//				global_excp_index[2 * count + 1] = fileindex;
//				fileOff += size;
//			} else {
//				if (fileindex >= 0) {
//					excpFile[fileindex]->close();
//				}
//
//				fileindex++;
//				fileOff = 0;
//				excpFile.push_back(new TempFile(Dir + "/newGraph", fileindex));
//				//reverse_start_ID[fileindex] = lastto;
//				excpFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
//				global_excp_index[2 * count] = 1;//actually is '0', but we set '1' here, for '0' has been used to mark the unexist 'to data'
//				global_excp_index[2 * count + 1] = fileindex;
//				fileOff += size;
//				firstTime = false;
//			}
//			count ++;
//
//			if(newGraph_id[count] != from){
//				for(; count < id_belong_tree[from]; count ++) {
//					tempEnt->empty();
//					tempEnt->insertID(newGraph_id[count]);
//					tempEnt->insertID(0); // for num
//
//					unsigned num = tempEnt->getSize() - 2;
//					tempEnt->getBuffer()[1] = num; //update num
//					unsigned size = tempEnt->getSize() * sizeof(ID);
//					if (!firstTime && fileOff + size < (unsigned) (-1) / 8) {
//						excpFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
//						global_excp_index[2 * count] = fileOff;
//						global_excp_index[2 * count + 1] = fileindex;
//						fileOff += size;
//					} else {
//						if (fileindex >= 0) {
//							excpFile[fileindex]->close();
//						}
//
//						fileindex++;
//						fileOff = 0;
//						excpFile.push_back(new TempFile(Dir + "/newGraph", fileindex));
//						//reverse_start_ID[fileindex] = lastto;
//						excpFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
//						global_excp_index[2 * count] = 1;//actually is '0', but we set '1' here, for '0' has been used to mark the unexist 'to data'
//						global_excp_index[2 * count + 1] = fileindex;
//						fileOff += size;
//						firstTime = false;
//					}
//				}
//			}
//
//			tempEnt->empty();
//			tempEnt->insertID(from);
//			tempEnt->insertID(0);
//			tempEnt->insertID(to);
//			lastfrom = from;
//			lastto = to;
//		} else {
//			if(to != lastto){
//				tempEnt->insertID(to);
//				lastto = to;
//			}
//		}
//	}
//
//	unsigned num = tempEnt->getSize() - 2;
//	tempEnt->getBuffer()[1] = num; //update num
//	unsigned size = tempEnt->getSize() * sizeof(ID);
//	if (!firstTime && fileOff + size < (unsigned) (-1) / 8) {
//		excpFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
//		global_excp_index[2 * count] = fileOff;
//		global_excp_index[2 * count + 1] = fileindex;
//		fileOff += size;
//	} else {
//		if (fileindex >= 0) {
//			excpFile[fileindex]->close();
//		}
//
//		fileindex++;
//		fileOff = 0;
//		excpFile.push_back(new TempFile(Dir + "/newGraph", fileindex));
//		//reverse_start_ID[fileindex] = lastto;
//		excpFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
//		global_excp_index[2 * count] = 1;//actually is '0', but we set '1' here, for '0' has been used to mark the unexist 'to data'
//		global_excp_index[2 * count + 1] = fileindex;
//		fileOff += size;
//		firstTime = false;
//	}
//	count ++;
//
//	if(count != newGraph_idcount){
//		for(; count < newGraph_idcount; count ++) {
//			tempEnt->empty();
//			tempEnt->insertID(newGraph_id[count]);
//			tempEnt->insertID(0); // for num
//
//			unsigned num = tempEnt->getSize() - 2;
//			tempEnt->getBuffer()[1] = num; //update num
//			unsigned size = tempEnt->getSize() * sizeof(ID);
//			if (!firstTime && fileOff + size < (unsigned) (-1) / 8) {
//				excpFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
//				global_excp_index[2 * count] = fileOff;
//				global_excp_index[2 * count + 1] = fileindex;
//				fileOff += size;
//			} else {
//				if (fileindex >= 0) {
//					excpFile[fileindex]->close();
//				}
//
//				fileindex++;
//				fileOff = 0;
//				excpFile.push_back(new TempFile(Dir + "/newGraph", fileindex));
//				//reverse_start_ID[fileindex] = lastto;
//				excpFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
//				global_excp_index[2 * count] = 1;//actually is '0', but we set '1' here, for '0' has been used to mark the unexist 'to data'
//				global_excp_index[2 * count + 1] = fileindex;
//				fileOff += size;
//				firstTime = false;
//			}
//		}
//	}
//	cout<<"count:"<<count<<" newGraph_idcount:"<<newGraph_idcount<<endl;

	excpFile[fileindex]->close();
	global_excp_indexFile->flush();

	init_newGraph();
	mappedIn.close();
	//cout<<"the number of crossid is: "<<crossid_count<<endl;
	for(int i = 0;i < excpFile.size();i++){
		delete excpFile[i];
		excpFile[i] = NULL;
	}
	excpFile.clear();
	delete global_excp_indexFile;
	delete tempEnt;
	//sortFile->discard();
	//delete sortFile;
}

void Graph::construct_newGraph_reverse_adj(TempFile &new_graph) {
	ID from, to, lastfrom, lastto;
	unsigned count;
	MemoryMappedFile mappedIn;
	assert(mappedIn.open(new_graph.getFile().c_str()));
	const char* reader = mappedIn.getBegin(), *limit = mappedIn.getEnd();
	TripleBitBuilder::loadTwo(reader, from, to);
	//lastfrom = oldID_mapto_newID[from];
	//lastto = oldID_mapto_newID[to];
	vector< vector<ID> > data(newGraph_idcount);
	count = 0;
	for(unsigned i = 0; i < newGraph_idcount; i++) {
		data[i].push_back(newGraph_id[i]);
	}
	while (reader < limit) {
		TripleBitBuilder::loadTwo(reader, from, to);
		from = oldID_mapto_newID[from];
		to = oldID_mapto_newID[to];
		reader = TempFile::skipIdId(reader);

		data[id_belong_tree[to]].push_back(from);
		//cout<<"from:"<<from<<" to:"<<to<<endl;
		//cout<<"id_belong_tree:"<<id_belong_tree[to]<<endl;
	}

	cout<<"copy end"<<endl;

	cout<<"write adj"<<endl;
	//wtrite adj
	bool firstTime = true, firstInsert = true;
	int fileindex = -1;
	unsigned fileOff = 0;
	EntityIDBuffer* tempEnt = new EntityIDBuffer();
	vector<TempFile*> excpFile;
	size_t pageSize = newGraph_idcount * sizeof(ID) / (MemoryBuffer::pagesize) + (((newGraph_idcount * sizeof(ID)) % (MemoryBuffer::pagesize)) ? 1 : 0);
	char excp_indexPath[150];
	sprintf(excp_indexPath, "%s/global_reverse_newGraph_index.0", Dir.c_str());
	MMapBuffer *global_excp_indexFile = new MMapBuffer(excp_indexPath, 2 * pageSize * MemoryBuffer::pagesize);
	unsigned *global_excp_index = (unsigned *)global_excp_indexFile->get_address();

	for(int i = 0; i < newGraph_idcount; i ++) {
		unsigned len = data[i].size();
		sort(data[i].begin() + 1, data[i].end());
		//cout<<" data:"<<data[i][0]<<" ";
		tempEnt->empty();
		tempEnt->insertID(data[i][0]);
		tempEnt->insertID(0);
		if(len > 1){
			tempEnt->insertID(data[i][1]);
			lastto = data[i][1];
			//cout<<" data:"<<data[i][1]<<" ";
		}
		for(unsigned k = 2; k < len; k ++) {
			if(data[i][k] != lastto){
				tempEnt->insertID(data[i][k]);
				lastto = data[i][k];
				//cout<<" data:"<<data[i][k]<<" ";
			}
		}
		//cout<<endl;
		unsigned num = tempEnt->getSize() - 2;
		tempEnt->getBuffer()[1] = num; //update num
		unsigned size = tempEnt->getSize() * sizeof(ID);
		if (!firstTime && fileOff + size < (unsigned) (-1) / 8) {
			excpFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
			global_excp_index[2 * i] = fileOff;
			global_excp_index[2 * i + 1] = fileindex;
			fileOff += size;
		} else {
			if (fileindex >= 0) {
				excpFile[fileindex]->close();
			}

			fileindex++;
			fileOff = 0;
			excpFile.push_back(new TempFile(Dir + "/reverse_newGraph", fileindex));
			//reverse_start_ID[fileindex] = lastto;
			excpFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
			global_excp_index[2 * i] = 1;//actually is '0', but we set '1' here, for '0' has been used to mark the unexist 'to data'
			global_excp_index[2 * i + 1] = fileindex;
			fileOff += size;
			firstTime = false;
		}
	}

	excpFile[fileindex]->close();
	global_excp_indexFile->flush();

	init_reverse_newGraph();
	mappedIn.close();
	//cout<<"the number of crossid is: "<<crossid_count<<endl;
	for(int i = 0;i < excpFile.size();i++){
		delete excpFile[i];
		excpFile[i] = NULL;
	}
	excpFile.clear();
	delete global_excp_indexFile;
	delete tempEnt;
}

unsigned Graph::cal_Lup(ID id, bool *&visited_arr) {
	//ID *addr = (ID *)newGraph_getOffset(id);
	ID *addr = (ID *)DAG_getOffset(id);
	unsigned count = addr[1];
	addr += 2;

	unsigned max = 0;
	for(unsigned j = 0; j < count; j ++) {
		unsigned l;
		//if(visited_arr[id_belong_tree[addr[j]]]) l = Lup[id_belong_tree[addr[j]]] + 1;
		if(visited_arr[addr[j]]) l = Lup[addr[j]] + 1;
		else l = cal_Lup(addr[j], visited_arr) + 1;
		if(l > max)
			max = l;
	}
	Lup[id] = max;
	//visited_arr[id_belong_tree[id]] = true;
	visited_arr[id] = true;
	//cout<<Lup[id_belong_tree[id]]<<"up:"<<max<<endl;
	return max;
}

unsigned Graph::cal_Ldown(ID id, bool *&visited_arr) {
	ID *addr = (ID *)DAG_reverse_getOffset(id);
	unsigned count = addr[1];
	addr += 2;

	unsigned max = 0;
	for(unsigned j = 0; j < count; j ++) {
		unsigned l;
		if(visited_arr[addr[j]]) l = Ldown[addr[j]] + 1;
		else l = cal_Ldown(addr[j], visited_arr) + 1;
		if(l > max)
			max = l;
	}
	Ldown[id] = max;
	//visited_arr[id_belong_tree[id]] = true;
	visited_arr[id] = true;
	//cout<<Ldown[id_belong_tree[id]]<<"Ldown:"<<max<<endl;
	return max;
}

unsigned Graph::cal_gLup(ID id, bool *&visited_arr) {
	ID *addr = (ID *)newGraph_getOffset(id);
	//ID *addr = (ID *)DAG_getOffset(id);
	unsigned count = addr[1];
	addr += 2;

	unsigned max = 0;
	for(unsigned j = 0; j < count; j ++) {
		//cout<<addr[j]<<"   ";
		unsigned l;
		if(visited_arr[id_belong_tree[addr[j]]]) l = gLup[id_belong_tree[addr[j]]] + 1;
		//if(visited_arr[addr[j]]) l = gLup[oldID_mapto_newID[addr[j]]] + 1;
		else l = cal_gLup(addr[j], visited_arr) + 1;
		if(l > max)
			max = l;
	}
	gLup[id_belong_tree[id]] = max;
	visited_arr[id_belong_tree[id]] = true;
	//visited_arr[id] = true;
	//cout<<id<<"up:"<<max<<endl;
	return max;
}

unsigned Graph::cal_gLdown(ID id, bool *&visited_arr) {
	ID *addr = (ID *)reverse_newGraph_getOffset(id);
	unsigned count = addr[1];
	addr += 2;

	unsigned max = 0;
	for(unsigned j = 0; j < count; j ++) {
		//cout<<addr[j]<<"   ";
		unsigned l;
		if(visited_arr[id_belong_tree[addr[j]]]) l = gLdown[id_belong_tree[addr[j]]] + 1;
		else l = cal_gLdown(addr[j], visited_arr) + 1;
		if(l > max)
			max = l;
	}
	gLdown[id_belong_tree[id]] = max;
	visited_arr[id_belong_tree[id]] = true;
	//visited_arr[id] = true;
	//cout<<id<<"Ldown:"<<max<<endl;
	return max;
}

void Graph::optimization(ID *root_arr, unsigned rootNum, bool *&visited_arr) {
	//find new_graph leafs
	vector<ID> leafs;
	for (unsigned start = 0; start <= maxID; start ++) {
		if (!degree[start].outdeg) {
		//if ((degree[start].indeg > 1) && !degree[start].outdeg) {
			leafs.push_back(start);
			//cout<<newGraph_id[start]<<" ";
		}
	}
	//cout<<endl;

	//calculate Lup
	memset(visited_arr, false, sizeof(bool) * (maxID + 1));
	Lup = (unsigned *)calloc((maxID + 1), sizeof(unsigned));

	for(unsigned i = 0; i < rootNum; i ++) {
		ID *addr = (ID *)DAG_getOffset(root_arr[i]);
		unsigned count = addr[1];
		addr += 2;

		unsigned max = 0;
		for(unsigned j = 0; j < count; j ++) {
			unsigned l;
			//if(visited_arr[id_belong_tree[addr[j]]]) l = Lup[id_belong_tree[addr[j]]] + 1;
			if(visited_arr[addr[j]]) l = Lup[addr[j]] + 1;
			else l = cal_Lup(addr[j], visited_arr) + 1;
			if(l > max)
				max = l;
		}
		Lup[root_arr[i]] = max;
	}

	//calculate Ldown
	memset(visited_arr, false, sizeof(bool) * (maxID + 1));
	Ldown = (unsigned *)calloc((maxID + 1), sizeof(unsigned));
	for(unsigned i = 0; i < leafs.size(); i ++) {
		ID *addr = (ID *)DAG_reverse_getOffset(leafs[i]);
		unsigned count = addr[1];
		addr += 2;

		unsigned max = 0;
		for(unsigned j = 0; j < count; j ++) {
			unsigned l;
			if(visited_arr[addr[j]]) l = Ldown[addr[j]] + 1;
			//if(visited_arr[addr[j]]) l = gLup[oldID_mapto_newID[addr[j]]] + 1;
			else l = cal_Ldown(addr[j], visited_arr) + 1;
			if(l > max)
				max = l;
		}
		Ldown[leafs[i]] = max;
	}

//	for (unsigned start = 1; start <= idcount; start ++) {
//		cout<<start<<":"<<Lup[start]<<" "<<Ldown[start]<<endl;
//	}
}

void Graph::newGraph_Optimization(ID *root_arr, unsigned rootNum, unsigned h) {
	//find new_graph leafs
	vector<ID> leafs;
	for (unsigned start = 0; start < newGraph_idcount; start ++) {
		if (!newGraph_degree[start].outdeg) {
		//if ((degree[start].indeg > 1) && !degree[start].outdeg) {
			leafs.push_back(newGraph_id[start]);
			//cout<<newGraph_id[start]<<" ";
		}
	}
	//cout<<endl;

	//reverse code new_graph
//	deque<ID>* que = new deque<ID>();
//	ID curID;
//	newID_mapto_newIDass = (ID *)calloc(newGraph_idcount, sizeof(ID));
	bool *visited_arr = (bool *)calloc(newGraph_idcount, sizeof(bool));

//	ID idcount = 1;
//
//	for(int i = leafs.size() - 1; i >= 0; i --) {
//		if(visited_arr[leafs[i]])continue;
//
//		que->push_front(leafs[i]);
//
//		while(!que->empty()){
//			curID = que->front();
//			que->pop_front();
//
//			visited_arr[id_belong_tree[curID]] = true;
//			newID_mapto_newIDass[id_belong_tree[curID]] = idcount;
//			idcount++;
//			//cout<<endl;
//			//cout<<curID<<":"<<newID_mapto_newIDass[id_belong_tree[curID]]<<endl;
//
//			ID *addr = (ID*) reverse_newGraph_getOffset(curID);
//			if (addr == NULL){
//				cout<<"reverse_newGraph_getOffset NULL id:"<<curID<<endl;
//				continue;
//			}
//
//			assert(curID == addr[0]);
//			unsigned count = addr[1];
//			addr += 2; //skip the id and count
//
//			for (size_t i = 0; i < count; i++) {
//				//cout<<addr[i]<<" ";
//				if(newGraph_degree[id_belong_tree[addr[i]]].outdeg)newGraph_degree[id_belong_tree[addr[i]]].outdeg--;
//				if(visited_arr[id_belong_tree[addr[i]]])continue;
//				if(newGraph_degree[id_belong_tree[addr[i]]].outdeg){
//					continue;
//				}//degree if
//				que->push_front(addr[i]);
//				//cout<<":"<<addr[i]<<endl;
//			}//for
//		}
//	} //for
//
//	delete que;

//	for (unsigned start = 0; start < newGraph_idcount; start ++) {
//		cout<<newID_mapto_newIDass[start]<<":"<<newGraph_id[start]<<endl;
//	}

	//calculate Lup
	memset(visited_arr, false, sizeof(bool) * newGraph_idcount);
	gLup = (unsigned *)calloc(newGraph_idcount, sizeof(unsigned));

	//cout<<"begin"<<endl;
	for(unsigned i = 0; i < rootNum; i ++) {
		ID *addr = (ID *)newGraph_getOffset(oldID_mapto_newID[root_arr[i]]);
		unsigned count = addr[1];
		addr += 2;

		//cout<<oldID_mapto_newID[root_arr[i]]<<":"<<endl;
		unsigned max = 0;
		for(unsigned j = 0; j < count; j ++) {
			//cout<<addr[j]<<" ";
			unsigned l;
			if(visited_arr[id_belong_tree[addr[j]]]) l = gLup[id_belong_tree[addr[j]]] + 1;
			//if(visited_arr[addr[j]]) l = gLup[oldID_mapto_newID[addr[j]]] + 1;
			else l = cal_gLup(addr[j], visited_arr) + 1;
			if(l > max)
				max = l;
		}
		//cout<<endl;
		gLup[id_belong_tree[oldID_mapto_newID[root_arr[i]]]] = max;
	}

	//calculate Ldown
	memset(visited_arr, false, sizeof(bool) * newGraph_idcount);
	gLdown = (unsigned *)calloc(newGraph_idcount, sizeof(unsigned));
	for(unsigned i = 0; i < leafs.size(); i ++) {
		ID *addr = (ID *)reverse_newGraph_getOffset(leafs[i]);
		unsigned count = addr[1];
		addr += 2;
		//cout<<leafs[i]<<":"<<endl;
		unsigned max = 0;
		for(unsigned j = 0; j < count; j ++) {
			//cout<<addr[j]<<" ";
			unsigned l;
			if(visited_arr[id_belong_tree[addr[j]]]) l = gLdown[id_belong_tree[addr[j]]] + 1;
			//if(visited_arr[addr[j]]) l = gLup[oldID_mapto_newID[addr[j]]] + 1;
			else l = cal_gLdown(addr[j], visited_arr) + 1;
			if(l > max)
				max = l;
		}
		gLdown[id_belong_tree[leafs[i]]] = max;
		//cout<<leafs[i]<<":"<<max<<endl;
	}

//	for (unsigned start = 0; start < newGraph_idcount; start ++) {
//		cout<<newGraph_id[start]<<":"<<gLup[start]<<" "<<gLdown[start]<<endl;
//	}

	//calculate Hugevertices
	//unsigned h = 2;
	hv.resize(newGraph_idcount);
	memset(visited_arr, false, sizeof(bool) * newGraph_idcount);
	for(size_t i = 0; i < newGraph_idcount; i ++) {
		if(newGraph_degree[i].outdeg > maxDegree){
			//cout<<newGraph_id[i]<<endl;
			hv[i].push_back(1);
			hv[i].push_back(i);
		}
		else hv[i].push_back(0);
	}
	for(unsigned i = 0; i < leafs.size(); i ++) {
		cal_hv(leafs[i], h, visited_arr);
	}

//	for (unsigned start = 0; start < newGraph_idcount; start ++) {
//		if(hv[start][0]){
//			cout<<newGraph_id[start]<<":";
//			for(unsigned i = 1; i < hv[start].size(); i ++)
//				cout<<newGraph_id[hv[start][i]]<<" ";
//		}
//		else cout<<newGraph_id[start]<<"none";
//		cout<<endl;
//	}
}

bool qcompare_newGraph_outdegree(unsigned a, unsigned b) {
	//sort from large to small
	//ID p1 = *(const ID*) a;
	//ID p2 = *(const ID*) b;
	a = newGraph_degree[a].outdeg;
	b = newGraph_degree[b].outdeg;
	return a>b;
}

void Graph::cal_hv(ID id, unsigned h, bool *&visited_arr) {
	ID *addr = (ID *) reverse_newGraph_getOffset(id);
	assert(id == addr[0]);
	unsigned count = addr[1];
	addr += 2;

	vector<ID> data;
	//cout<<"id:"<<id<<endl;
	for(unsigned i = 0; i < count; i ++) {
		if(!visited_arr[id_belong_tree[addr[i]]]) {
			cal_hv(addr[i], h, visited_arr);
		}
		for(unsigned k = 1; k < hv[id_belong_tree[addr[i]]].size(); k ++){
			//cout<<"data push"<<addr[i]<<endl;
			data.push_back(hv[id_belong_tree[addr[i]]][k]);
		}
	}
	if(!data.empty()){
		hv[id_belong_tree[id]][0] = 1;
		sort(data.begin(), data.end(), qcompare_newGraph_outdegree);
		int len = data.size();
		unsigned lastdata= data[0];
		hv[id_belong_tree[id]].push_back(data[0]);
		for(int i = 1; i < h && i < len; i ++){
			//cout<<"hv push"<<data[i]<<" :"<<newGraph_id[data[i]]<<endl;
			if(lastdata != data[i]){
				hv[id_belong_tree[id]].push_back(data[i]);
				lastdata = data[i];
			}
		}
		sort(hv[id_belong_tree[id]].begin() + 1, hv[id_belong_tree[id]].end(), qcompare_newGraph_outdegree);
	}
	visited_arr[id_belong_tree[id]] = true;
}

void Graph::construct_newGraph_levelLabel(bool *cross_vertices_arr) {
	Lup = (unsigned *)calloc(newGraph_idcount, sizeof(unsigned));
	Ldown = (unsigned *)calloc(newGraph_idcount, sizeof(unsigned));

	//Lup
	ID id;
	unsigned count = 0;

	while (count < newGraph_idcount){
		id = newGraph_id[count];
		//cout<<"newGraph:"<<id<<endl;
		Lup[id] = posttranv_tree(newID_mapto_oldID[id], cross_vertices_arr);
		//id_belong_tree[id] = count;
		count ++;
		//cout<<id<<"根标签："<<idtag[id]<<endl;
	}
}

ID Graph::posttranv_tree(ID id, bool *cross_vertices_arr) {
	unsigned ct;
	ID *addr = (ID*) DAG_getOffset(id);
	ID max = oldID_mapto_newID[id];
	if (addr == NULL) return max;
	assert(id == addr[0]);
	ct = addr[1];
	addr += 2; //skip the id and count

	ID *adj = (ID *)calloc(ct, sizeof(ID));
	for (size_t i = 0; i < ct; i++){
		//id_belong_tree[oldID_mapto_newID[addr[i]]] = count;
		if(! cross_vertices_arr[addr[i]]) {
			adj[i] = posttranv_tree(addr[i], cross_vertices_arr);
			idtag[oldID_mapto_newID[addr[i]]] = adj[i];
			//cout<<oldID_mapto_newID[addr[i]]<<"子标签："<<adj[i]<<endl;
		}
	}

	for(size_t i = 0; i < ct; i++){
		if(adj[i] > max)
			max = adj[i];
	}
	return max;
}

void Graph::construct_newGraph_idtag(bool *cross_vertices_arr) {
	ID id;
	unsigned count = 0;

	while (count < newGraph_idcount){
		id = newGraph_id[count];
		//cout<<"newGraph:"<<id<<endl;
		idtag[id] = posttranv_tree(newID_mapto_oldID[id], cross_vertices_arr);
		//id_belong_tree[id] = count;
		count ++;
		//cout<<id<<"根标签："<<idtag[id]<<endl;
	}
}

void Graph::tranverse(ID id, EntityIDBuffer*& tempEnt) {
	bool *visit = (bool *)calloc(newGraph_idcount, sizeof(bool));
	memset(visit, false, sizeof(bool) * newGraph_idcount);
	deque<ID>* que = new deque<ID>();
	if(! visit[id]){
		que->push_back(id);
		visit[id] = true;
		while(!que->empty()) {
			ID curID = que->front();
			que->pop_front();

			ID *addr = (ID*) newGraph_getOffset(curID);
			assert(addr[0] == curID);
			unsigned count = addr[1];
			addr += 2;
			for(unsigned i = 0; i < count; i ++) {
				if(! visit[addr[i]]) {
					que->push_back(addr[i]);
					tempEnt->insertID(addr[i]);
					visit[addr[i]] = true;
				}
			}
		}
	}
}

void Graph::construct_hugeVertices() {
	vector< vector<ID> > hv(newGraph_idcount);
	for(size_t i = 0; i < newGraph_idcount; i ++) {
		if(newGraph_degree[newGraph_id[i]].outdeg > maxDegree){
			hv[i].push_back(1);
			hv[i].push_back(i);
		}
		else hv[i].push_back(0);
	}


	bool firstTime = true, firstInsert = true;
	int fileindex = -1;
	unsigned fileOff = 0;
	EntityIDBuffer* tempEnt = new EntityIDBuffer();
	vector<TempFile*> excpFile;
	size_t pageSize = newGraph_idcount * sizeof(ID) / (MemoryBuffer::pagesize) + (((newGraph_idcount * sizeof(ID)) % (MemoryBuffer::pagesize)) ? 1 : 0);
	char excp_indexPath[150];
	sprintf(excp_indexPath, "%s/global_hugeVertices_index.0", Dir.c_str());
	MMapBuffer *global_excp_indexFile = new MMapBuffer(excp_indexPath, 2 * pageSize * MemoryBuffer::pagesize);
	unsigned *global_excp_index = (unsigned *)global_excp_indexFile->get_address();

	unsigned count = 0;
	for(size_t i = 0; i < newGraph_idcount; i ++) {
		if(newGraph_degree[newGraph_id[i]].outdeg >= maxDegree){
			tempEnt->insertID(newGraph_id[i]);
			tempEnt->insertID(0);
			tranverse(newGraph_id[i], tempEnt);

			unsigned num = tempEnt->getSize() - 2;
			tempEnt->getBuffer()[1] = num; //update num
			unsigned size = tempEnt->getSize() * sizeof(ID);
			if (!firstTime && fileOff + size < (unsigned) (-1) / 8) {
				excpFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
				global_excp_index[2 * i] = fileOff;
				global_excp_index[2 * i + 1] = fileindex;
				fileOff += size;
			} else {
				if (fileindex >= 0) {
					excpFile[fileindex]->close();
				}

				fileindex++;
				fileOff = 0;
				excpFile.push_back(new TempFile(Dir + "/hugeVertices", fileindex));
				excpFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
				global_excp_index[2 * i] = 1;//actually is '0', but we set '1' here, for '0' has been used to mark the unexist 'to data'
				global_excp_index[2 * i + 1] = fileindex;
				fileOff += size;
				firstTime = false;
			}

			count ++;
		}
	}

	cout<<"hugeVertices count:"<<count<<endl;

	excpFile[fileindex]->close();
	global_excp_indexFile->flush();

	init_hugeVertices();
	for(int i = 0;i < excpFile.size();i++){
		delete excpFile[i];
		excpFile[i] = NULL;
	}
	excpFile.clear();
	delete global_excp_indexFile;
	delete tempEnt;
}

void Graph::is_excp(bool *&cross_vertices_arr, bool *&visited_arr) {
	get_forward_root_vertices(maxID);

	MemoryMappedFile *rootMap = new MemoryMappedFile();
	assert(rootMap->open((Dir+"/DAGroot.forward.0").c_str()));
	unsigned rootNum = (rootMap->getEnd() - rootMap->getBegin()) / sizeof(ID);
	ID *root_arr = (ID*) rootMap->getBegin();
	if (rootNum > 0)cout << "root:" << *(root_arr) << "  rootNum:" << rootNum << endl;

	deque<ID>* que = new deque<ID>();
	ID curID;
	unsigned count, excp_count = 0;
	for(size_t index = 0;index < rootNum;index++){
		//if(visited_arr[root_arr[index]])continue;
		memset(visited_arr, false, sizeof(bool) * (maxID + 1));
		que->push_front(root_arr[index]);

		while(!que->empty()){
			curID = que->front();
			que->pop_front();

			visited_arr[curID] = true;

			ID *addr = (ID*) DAG_getOffset(curID);
			if (addr == NULL){
				cout<<"DAG_getOffset NULL id:"<<curID<<endl;
				continue;
			}
			assert(curID == addr[0]);
			count = addr[1];
			addr += 2; //skip the id and count

			for (size_t i = 0; i < count; i++) {
				if(degree[addr[i]].indeg)degree[addr[i]].indeg--;
				if(visited_arr[addr[i]])continue;
				if(degree[addr[i]].indeg){
					excp_count ++;
					continue;
				}//degree if
				que->push_front(addr[i]);
			}//for
		} //while
	} //for

	cout<<"excp_count:"<<excp_count<<endl;
}

void Graph::dfs_code(string path, bool *&cross_vertices_arr, bool *&visited_arr, unsigned h) {
	get_forward_root_vertices(maxID);

	MemoryMappedFile *rootMap = new MemoryMappedFile();
	assert(rootMap->open((Dir+"/DAGroot.forward.0").c_str()));
	unsigned rootNum = (rootMap->getEnd() - rootMap->getBegin()) / sizeof(ID);
	ID *root_arr = (ID*) rootMap->getBegin();
	if (rootNum > 0)cout << "root:" << *(root_arr) << "  rootNum:" << rootNum << endl;

	newGraph_idcount = 0;
	unsigned cross_idcount = 0;
	vector <ID> cross_id;
	//mark_cross_vertices(path, root_arr, rootNum, visited_arr, cross_vertices_arr, cross_idcount, cross_id);
	//assert(false);
	return ;
	memset(visited_arr, false, sizeof(bool) * (maxID + 1));
	cout<<"cross_idcount:"<<cross_idcount<<endl;

	cout<<"newGraph_idcount:"<<newGraph_idcount<<endl;
	idcount = 1;
	newID_mapto_oldID = (ID *)calloc(maxID + 2, sizeof(ID));
	oldID_mapto_newID = (ID *)calloc(maxID + 1, sizeof(ID));
	excp_located = (ID *)calloc(maxID + 1, sizeof(ID));
	memset(excp_located, 0, sizeof(unsigned) * (maxID + 1));
	TempFile excp_id(path + "/excp_id");
	TempFile new_graph(path + "/new_graph");
	TempFile new_graph_root(path + "/new_graph_root");

	deque<ID>* que = new deque<ID>();
	ID curID;
	unsigned count;
	bool have_cross_vertices = false;
	DAG_maxID = 0;
	excp_count = 0;
	excp_idcount = 0;
	bool have_excp_id;

	if(rootNum == 1 && (!degree[root_arr[0]].outdeg || degree[root_arr[0]].indeg))
		cout<<"don't have any root vertices!"<<endl;
	else {
		for(size_t index = 0;index < rootNum;index++){
			if(visited_arr[root_arr[index]])continue;

			que->push_front(root_arr[index]);
			new_graph_root.writeId(root_arr[index]);
			//cout<<"root:"<<root_arr[index]<<" "<<newGraph_idcount<<endl;
			while(!que->empty()){
				curID = que->front();
				que->pop_front();
				DAG_maxID = std::max(curID, DAG_maxID);

				visited_arr[curID] = true;
				oldID_mapto_newID[curID] = idcount;
				newID_mapto_oldID[idcount] = curID;
				idcount++;

				ID *addr = (ID*) DAG_getOffset(curID);
				if (addr == NULL){
					cout<<"DAG_getOffset NULL id:"<<curID<<endl;
					continue;
				}
				assert(curID == addr[0]);
				count = addr[1];
				addr += 2; //skip the id and count
				have_excp_id = false;

				for (size_t i = 0; i < count; i++) {
					DAG_maxID = std::max(addr[i], DAG_maxID);

					if(degree[addr[i]].indeg)degree[addr[i]].indeg--;
					if(cross_vertices_arr[addr[i]]){
						have_cross_vertices = true;
						continue;
					}
					if(visited_arr[addr[i]])continue;
					if(degree[addr[i]].indeg){
						excp_count ++;
						excp_id.writeId(oldID_mapto_newID[curID]);
						excp_id.writeId(addr[i]);
						have_excp_id = true;
						continue;
					}//degree if
					que->push_front(addr[i]);
				}//for
				if(have_excp_id) {
					excp_idcount ++;
					excp_located[oldID_mapto_newID[curID]] = excp_idcount;
				}
			} //while
		} //for
	} //else

	excp_id.flush();
	new_graph_root.flush();

	cout<<"root code finish"<<endl;

	if(have_cross_vertices == true) {
//		qsort(cross_id, cross_idcount, sizeof(ID), qcompare);
//		cross_collector(cross_id, cross_idcount, cross_vertices_arr, visited_arr, excp_id, new_graph_root);
	}
	idcount --;

	new_graph_root.flush();
	excp_id.flush();
	cout<<"in subtree_collector(), idcount = "<<idcount<<" DAG_maxID:"<<DAG_maxID<<endl;
	cout<<"cross code finish!"<<endl;
	cout<<"excp_count:"<<excp_count<<endl;

	if(excp_idcount != 0)
		construct_excp_id(excp_id);

//	for(size_t index = 1;index <= idcount;index++){
//
//		cout<<"oldID:"<<newID_mapto_oldID[index]<<" newID:"<<oldID_mapto_newID[newID_mapto_oldID[index]]<<endl;
//	}

	cout<<"maxID:"<<maxID<<endl;

	idtag = (ID *)calloc(idcount + 1, sizeof(ID));
	cout<<"mark tag begin:"<<endl;
	mark_vertices_tag(new_graph_root, cross_vertices_arr);
	cout<<"mark tag end:"<<endl;
//	for(size_t index = 1;index <= idcount;index++) {
//		cout<<"id:"<<index<<": "<<idtag[index]<<" ";
//	}
//	cout<<endl;

	if(excp_idcount != 0){
		deal_with_excp(new_graph_root, cross_vertices_arr, cross_idcount);
		new_graph_root.flush();
		cout<<"deal_with_excp end"<<endl;
	}
	memset(visited_arr, false, sizeof(bool) * (maxID + 1));
	memset(oldID_mapto_newID, 0, sizeof(ID) * (maxID + 1));
	memset(newID_mapto_oldID, 0, sizeof(ID) * (maxID + 2));

	id_belong_tree = (unsigned *)calloc(idcount + 1, sizeof(unsigned));
	construct_newGraph(root_arr, rootNum, new_graph, new_graph_root, cross_vertices_arr, visited_arr, cross_idcount);
	cout<<"construct_newGraph end"<<endl;
	memset(visited_arr, false, sizeof(bool) * (maxID + 1));
	construct_newGraph_ass(root_arr, rootNum, new_graph_root, cross_vertices_arr, visited_arr, cross_idcount);
	//cout<<"construct_newGraph ass end"<<endl;
//	for(size_t index = 1;index <= idcount;index++){
//
//		cout<<"oldID:"<<newID_mapto_oldID[index]<<" newID:"<<oldID_mapto_newID[newID_mapto_oldID[index]]<<endl;
//	}

//	for(unsigned i = 1; i <= idcount; i ++) {
//		cout<<i<<":"<<newID_mapto_newIDass[i]<<endl;
//	}

	new_graph.flush();
	TempFile new_graph_path(path + "/new_graph_path");
	construct_newGraph_adj(new_graph, new_graph_path, cross_vertices_arr);
	cout<<"construct_newGraph_adj end"<<endl;
	construct_newGraph_reverse_adj(new_graph);
	cout<<"construct_newGraph_reverse_adj end"<<endl;
	//print_newGraph();
	newGraph_Optimization(root_arr, rootNum, h);
	cout<<"newGraph_Optimization end"<<endl;
	optimization(root_arr, rootNum, visited_arr);
	cout<<"optimization end"<<endl;
	memset(idtag, 0, sizeof(ID) * (idcount + 1));
	construct_newGraph_idtag(cross_vertices_arr);
	cout<<"construct_newGraph_idtag end"<<endl;
	//construct_newGraph_levelLabel();
	//cout<<"construct_newGraph_levelLabel end"<<endl;
	cout<<"root num:"<<rootNum<<endl;
	cout<<"newGraph_idcount:"<<newGraph_idcount<<endl;
	cout<<"idcount:"<<idcount<<endl;
	cout<<"cross_idcount:"<<cross_idcount<<endl;
	cout<<"excp_count:"<<excp_count<<endl;
	//print_newGraph();
//	for(size_t i = 1; i <= idcount; i ++){
//		if(i % 5 == 0)
//			cout<<endl;
//		cout<<i<<"标签："<<idtag[i]<<" ";
//	}
//
//	cout<<endl;

	TempFile *old_mapto_new = new TempFile(path + "/old_mapto_new", 0);
	TempFile *new_mapto_old = new TempFile(path + "/new_mapto_old", 0);
	old_mapto_new->write((maxID + 1) * sizeof(ID), (const char*) oldID_mapto_newID);
	old_mapto_new->close();
	delete old_mapto_new;
	new_mapto_old->write((maxID + 2) * sizeof(ID), (const char*) newID_mapto_oldID);
	new_mapto_old->close();
	delete new_mapto_old;
//	free(oldID_mapto_newID);
//	oldID_mapto_newID = NULL;
//	free(newID_mapto_oldID);
//	newID_mapto_oldID = NULL;

	TempFile *id_tag = new TempFile(path + "/id_tag",0);
	TempFile *newGraphID = new TempFile(path + "/newGraphID",0);
	id_tag->write((idcount + 1) * sizeof(ID), (const char*)idtag);
	//cout<<"write newGraph"<<endl;
	newGraphID->write((newGraph_idcount) * sizeof(ID), (const char*)newGraph_id);
	//cout<<"write newGraph1"<<endl;
	newGraphID->close();
	id_tag->close();
	delete id_tag;
	delete newGraphID;
	delete que;
	rootMap->close();
	delete rootMap;

	remove(new_graph_path.getFile().c_str());
	remove(new_graph.getFile().c_str());
	remove(new_graph_root.getFile().c_str());
	remove(excp_id.getFile().c_str());
}

bool Graph::find_visited(ID id, ID *addr, unsigned k) {
	for(size_t i = 0; i < k; i ++){
		if(scc_located[id] == addr[i])
			return true;
	}
	return false;
}

bool Graph::is_equal(deque<ID>* que, ID curID) {
	for(size_t i = 0; i < que->size(); i ++){
		if(que->at(i) == curID){
			return true;
		}
	}

	return false;
}

void Graph::deal_with_scc(ID id, unsigned &ct, ID *&scc_visited_firstid, EntityIDBuffer *&tempEnt, TempFile &DAGfile) {
	//deque<ID>* que = new deque<ID>();
	//que->clear();
	ID *sccaddr = (ID*) scc_getOffset(scc_located[id]);
	assert(scc_located[id] == sccaddr[0]);
	unsigned scount = sccaddr[1];
	sccaddr += 2; //skip the id and count

	for(size_t i = 0; i < scount; i ++){
		ID *addr = (ID*) getOffset(sccaddr[i]);
		assert(sccaddr[i] == addr[0]);
		unsigned count = addr[1];
		addr += 2; //skip the id and count
		for(size_t k = 0; k < count; k++) {
			//cout<<"i:"<<i<<endl;
			//if(que->empty()) cout<<"empty true"<<endl;
			//cout<<"que."<<que->size()<<endl;
			if(scc_located[addr[k]] == 0){
				if(visit_flag[addr[k]] == querycnt) {
					degree[addr[k]].indeg --;
					globalDegree[addr[k]].indeg --;
				}
				else {
					visit_flag[addr[k]] = querycnt;
					tempEnt->insertID(addr[k]);
					DAGfile.writeId(id);
					DAGfile.writeId(addr[k]);
				}
				//else que->push_back(addr[k]);
			}
			else if(scc_located[addr[k]] != 0 && scc_located[addr[k]] != scc_located[id] && visit_flag[scc_located[addr[k]]] != querycnt){
				//que->push_back(scc_located[addr[k]]);
				visit_flag[scc_located[addr[k]]] = querycnt;
				tempEnt->insertID(scc_located[addr[k]]);
				degree[scc_located[addr[k]]].indeg ++;
				globalDegree[scc_located[addr[k]]].indeg ++;

				DAGfile.writeId(id);
				DAGfile.writeId(scc_located[addr[k]]);
			}
		}
	}

//	scc_visited_firstid[ct] = scc_located[id];
//	ct ++;

	//delete que;
}

status Graph::generate_DAG_reverse_adj(TempFile &DAGfile) {
	TempFile* sortFile = new TempFile("sortFile");
	Sorter::sort(DAGfile, *sortFile, TempFile::skipIdId, TempFile::compare21);
	//Sorter::sort(*tempFile, *sortFile, TempFile::skipIdId, TempFile::compare12);
	remove(DAGfile.getFile().c_str());

	MemoryMappedFile mappedIn;
	assert(mappedIn.open(sortFile->getFile().c_str()));
	const char* reader = mappedIn.getBegin(), *begin = reader, *limit = mappedIn.getEnd();

	bool firstInsert = true, firstTime = true;
	int fileindex = -1;
	unsigned fileOff = 0;
	unsigned lastto = *(ID *)(reader + sizeof(ID));
	EntityIDBuffer* tempEnt = new EntityIDBuffer();
	vector<TempFile*> adjFile;
	size_t pageSize = DAG_idcount * sizeof(ID) / (MemoryBuffer::pagesize) + (((DAG_idcount * sizeof(ID)) % (MemoryBuffer::pagesize)) ? 1 : 0);
	char adj_indexPath[150];
	sprintf(adj_indexPath, "%s/DAG_reverse_global_adj_index1.0", Dir.c_str());
	MMapBuffer *global_adj_indexFile = new MMapBuffer(adj_indexPath, 2 * pageSize * MemoryBuffer::pagesize);
	unsigned *global_adj_index = (unsigned *)global_adj_indexFile->get_address();

	ID from = 0, to = *(ID*) (reader + sizeof(ID));

	//to = *(ID*) reader;
	for(ID i = 0; i < to; i ++) {
		tempEnt->empty();
		tempEnt->insertID(i);
		tempEnt->insertID(0);

		unsigned num = tempEnt->getSize() - 2;
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
			adjFile.push_back(new TempFile(Dir + "/DAG_reverse_adj1", fileindex));
			//reverse_start_ID[fileindex] = i;
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
		reader += sizeof(ID);
		//cout<<"from:"<<from<<" to:"<<to<<endl;
		//cout<<"lastto:"<<lastto<<endl;

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
				adjFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
				global_adj_index[2 * lastto] = fileOff;
				global_adj_index[2 * lastto + 1] = fileindex;
				fileOff += size;
			} else {
				if (fileindex >= 0) {
					adjFile[fileindex]->close();
				}

				fileindex++;
				fileOff = 0;
				adjFile.push_back(new TempFile(Dir + "/DAG_reverse_adj1", fileindex));
				//reverse_start_ID[fileindex] = lastto;
				adjFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
				global_adj_index[2 * lastto] = 1;//actually is '0', but we set '1' here, for '0' has been used to mark the unexist 'to data'
				global_adj_index[2 * lastto + 1] = fileindex;
				fileOff += size;
				firstTime = false;

				//cout << "fileIndex: " << fileindex << ", from minID " << to<< " to maxID " << maxID << endl;
			}

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
						adjFile.push_back(new TempFile(Dir + "/DAG_reverse_adj1", fileindex));
						//reverse_start_ID[fileindex] = i;
						adjFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
						global_adj_index[2 * i] = 1;//actually is '0', but we set '1' here, for '0' has been used to mark the unexist 'from data'
						global_adj_index[2 * i + 1] = fileindex;
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

	unsigned num = tempEnt->getSize() - 2;
	tempEnt->getBuffer()[1] = num; //update num
	unsigned size = tempEnt->getSize() * sizeof(ID);
	if (!firstTime && fileOff + size < (unsigned) (-1) / 8) {
		adjFile[fileindex]->write(size, (const char*) tempEnt->getBuffer());
		global_adj_index[2 * lastto] = fileOff;
		global_adj_index[2 * lastto + 1] = fileindex;
		fileOff += size;
	} else {
		if (fileindex >= 0) {
			adjFile[fileindex]->close();
		}

		fileindex++;
		fileOff = 0;
		adjFile.push_back(new TempFile(Dir + "/DAG_reverse_adj1", fileindex));
		//reverse_start_ID[fileindex] = lastto;
		adjFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
		global_adj_index[2 * lastto] = 1;//actually is '0', but we set '1' here, for '0' has been used to mark the unexist 'from data'
		global_adj_index[2 * lastto + 1] = fileindex;
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
			adjFile.push_back(new TempFile(Dir + "/DAG_reverse_adj1", fileindex));
			//reverse_start_ID[fileindex] = i;
			adjFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
			global_adj_index[2 * i] = 1;//actually is '0', but we set '1' here, for '0' has been used to mark the unexist 'from data'
			global_adj_index[2 * i + 1] = fileindex;
			fileOff += size;
			firstTime = false;
		}
	}

	//maxFromID = maxID;
	//cout << "fileIndex:" << fileindex << ", maxID " << maxID << endl;

	adjFile[fileindex]->close();
	global_adj_indexFile->flush();
	cout<<"success convert into DAG reverse adjacency list"<<endl;

	//reverse_adjNum = adjFile.size();
	for(int i = 0;i < adjFile.size();i++){
		delete adjFile[i];
		adjFile[i] = NULL;
	}
	adjFile.clear();
	delete global_adj_indexFile;

	mappedIn.close();
	sortFile->discard();
	delete sortFile;
	delete tempEnt;

	return OK;
}

void Graph::generate_DAG(string path) {
	bool firstInsert = true, firstTime = true;
	int fileindex = -1;
	unsigned fileOff = 0;
	EntityIDBuffer* tempEnt = new EntityIDBuffer();
	vector<TempFile*> DAGFile;
	size_t pageSize = (maxID + 1) * sizeof(ID) / (MemoryBuffer::pagesize) + ((((maxID + 1) * sizeof(ID)) % (MemoryBuffer::pagesize)) ? 1 : 0);
	char DAG_indexPath[150];
	sprintf(DAG_indexPath, "%s/global_DAG_index1.0", Dir.c_str());
	MMapBuffer *global_DAG_indexFile = new MMapBuffer(DAG_indexPath, 2 * pageSize * MemoryBuffer::pagesize);
	unsigned *global_DAG_index = (unsigned *)global_DAG_indexFile->get_address();

	//bool *scc_visited = (bool *)calloc(scc_num, sizeof(bool));
	//memset(scc_visited, false, (scc_num + 1));
	ID *scc_visited_firstid = (ID *)calloc(scc_num, sizeof(ID));
	visit_flag = (ID *)calloc(maxID + 1, sizeof(ID));
	querycnt = 0;

	DAG_idcount = 0;
	TempFile DAGfile(path + "/DAGfile");
	//cout<<"startID:"<<start_ID[0]<<" maxID:"<<maxID<<endl;
	for (ID i = start_ID[0]; i <= maxID; i ++) {

		ID *addr;
		addr = (ID*) getOffset(i);
		//cout<<i<<"邻接点：";
		if (addr == NULL){
			//cout<<"id:"<<i<<" null"<<endl;
			tempEnt->empty();
			tempEnt->insertID(i);
			tempEnt->insertID(0);
			//continue ;
		}
		else{
			assert(i == addr[0]);
			unsigned count = addr[1];
			addr += 2; //skip the id and count

			tempEnt->empty();
			tempEnt->insertID(i);
			tempEnt->insertID(0);
			unsigned ct = 0;
			querycnt ++;

			if(scc_located[i] == 0){
				for(unsigned k = 0; k < count; k++) {
					//cout<<addr[k]<<" ";
					if(scc_located[addr[k]] != 0){
						//if(!tempEnt->is_existed(scc_located[addr[k]])){
						if(visit_flag[scc_located[addr[k]]] != querycnt){
							tempEnt->insertID(scc_located[addr[k]]);
							visit_flag[scc_located[addr[k]]] = querycnt;
							degree[scc_located[addr[k]]].indeg ++;
							globalDegree[scc_located[addr[k]]].indeg ++;
							DAGfile.writeId(i);
							DAGfile.writeId(scc_located[addr[k]]);
						}
					}
					else{
						if(visit_flag[addr[k]] != querycnt){
							tempEnt->insertID(addr[k]);
							visit_flag[addr[k]] = querycnt;
							DAGfile.writeId(i);
							DAGfile.writeId(addr[k]);
						}
					}
				}
			}//if scc
			else if(scc_located[i] == i) {
//				bool tag = false;
//				for(size_t n = 0; n < ct; n ++) {
//					if(scc_located[i] == scc_visited_firstid[n])
//						tag = true;
//				}
//				//cout<<"i"<<i<<endl;
//				if(tag == false)
					deal_with_scc(i, ct, scc_visited_firstid, tempEnt, DAGfile);
			}
		}

		unsigned num = tempEnt->getSize() - 2;
		tempEnt->getBuffer()[1] = num; //update num
		degree[i].outdeg = num;
		globalDegree[i].outdeg = num;
		unsigned size = tempEnt->getSize() * sizeof(ID);
		if (!firstTime && fileOff + size < (unsigned) (-1) / 8) {
			DAGFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
			global_DAG_index[2 * i] = fileOff;
			global_DAG_index[2 * i + 1] = fileindex;
			fileOff += size;
		} else {
			if (fileindex >= 0) {
				DAGFile[fileindex]->close();
			}

			fileindex++;
			fileOff = 0;
			DAGFile.push_back(new TempFile(Dir + "/DAG1", fileindex));
			DAG_start_ID[fileindex] = i;
			DAGFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
			global_DAG_index[2 * i] = 1;//actually is '0', but we set '1' here, for '0' has been used to mark the unexist 'to data'
			global_DAG_index[2 * i + 1] = fileindex;
			fileOff += size;
			firstTime = false;

			//cout << "fileIndex: " << fileindex << ", from minID " << to<< " to maxID " << maxID << endl;
		}

		DAG_idcount ++;
	} //for

//	init_scc();

	DAGfile.flush();
	DAGFile[fileindex]->close();
	global_DAG_indexFile->flush();
	//cout<<"success convert into reverse adjacency list"<<endl;

	cout<<endl;
	init_DAG();
	generate_DAG_reverse_adj(DAGfile);
	init_reverse_DAG();

	for(int i = 0;i < DAGFile.size();i++){
		delete DAGFile[i];
		DAGFile[i] = NULL;
	}
	DAGFile.clear();
	delete global_DAG_indexFile;

	delete tempEnt;
	remove(DAGfile.getFile().c_str());
	//delete que;
}

unsigned Graph::belong_tree(ID id) {
	unsigned i = 0;
	while(i < newGraph_idcount && id >= newGraph_id[i])
		i ++;
	return i - 1;
}

bool Graph::tree_reach(ID x, ID y) {
	if(x <= y && idtag[x] >= idtag[y])
		return true;
	return false;
}

bool Graph::is_existed(ID x, ID y) {
	ID *addr = (ID *) newGraph_getOffset(x);
	assert(addr[0] == x);
	unsigned count = addr[1];
	addr += 2;

	if(count == 0)return false;
	if(count == 1) {
		if(addr[0] == y) return true;
		else return false;
	}
	if(y < addr[0] || y > addr[count - 1]) return false;

	unsigned m = count / 2 - 1, l = 0, r = count - 1;
	while(l != r){
		if(y == addr[m]) return true;
		if(y < addr[m]){
			r = m;
			m = (r - l + 1) / 2 - 1 + l;
		}
		else {
			l = m + 1;
			m = (r - l + 1) / 2 - 1 + l;
		}
	}
	if(y == addr[l]) return true;
	else return false;
}

bool Graph::is_reachable(ID x, ID y, unsigned h, bool *&visit) {
//	unsigned h = 2;
	visit[x] = true;
	if(newGraph_degree[id_belong_tree[x]].outdeg > maxDegree){
		unsigned len = hv[id_belong_tree[y]].size();
		if(len == 1) return false;
		bool result = false;
		for(unsigned i = 1; i < len; i ++){
			if(hv[id_belong_tree[y]][i] == id_belong_tree[x])
				result = true;
		}
		if(result == true) return true;
		if(result == false) {
			if(len - 1 < h) return false;
			if(newGraph_degree[id_belong_tree[x]].outdeg > newGraph_degree[hv[id_belong_tree[y]][len - 1]].outdeg) return false;
		}

	}


	ID *addr = (ID *) newGraph_getOffset(x);
	assert(addr[0] == x);
	unsigned count = addr[1];
	addr += 2;
	for(size_t i = 0; i < count; i++) {
		if(!visit[addr[i]]){
			if(addr[i] == y) return true;
			//if(addr[i] > y || newID_mapto_newIDass[id_belong_tree[addr[i]]] > newID_mapto_newIDass[id_belong_tree[y]]) continue;
			if(addr[i] > y || newID_mapto_newIDass[addr[i]] > newID_mapto_newIDass[y]) continue;

			else if(gLup[id_belong_tree[addr[i]]] <= gLup[id_belong_tree[y]] || gLdown[id_belong_tree[addr[i]]] >= gLdown[id_belong_tree[y]])
				//else if(Lup[addr[i]] <= Lup[y] || Ldown[addr[i]] >= Ldown[y])
				continue;
			else if(Lup[addr[i]] <= Lup[y] || Ldown[addr[i]] >= Ldown[y]) continue;
			else {
				if(is_reachable(addr[i], y, h, visit))
					return true;
			}
		}
	}

//	deque<ID>* que  = new deque<ID>();
//	que->push_back(x);
//	while(!que->empty()){
//		ID curID = que->front();
//		que->pop_front();
//
//		ID *addr = (ID *) newGraph_getOffset(curID);
//		assert(addr[0] == curID);
//		unsigned count = addr[1];
//		addr += 2;
//		for(size_t i = 0; i < count; i++) {
//			if(addr[i] == y) return true;
//			//if(addr[i] > y || newID_mapto_newIDass[id_belong_tree[addr[i]]] > newID_mapto_newIDass[id_belong_tree[y]]) continue;
//			if(addr[i] > y || newID_mapto_newIDass[addr[i]] > newID_mapto_newIDass[y]) continue;
//
//			else if(gLup[id_belong_tree[addr[i]]] <= gLup[id_belong_tree[y]] || gLdown[id_belong_tree[addr[i]]] >= gLdown[id_belong_tree[y]])
//				//else if(Lup[addr[i]] <= Lup[y] || Ldown[addr[i]] >= Ldown[y])
//				continue;
//			else if(Lup[addr[i]] <= Lup[y] || Ldown[addr[i]] >= Ldown[y]) continue;
//			else if(!visit[addr[i]]){
//				que->push_back(addr[i]);
//				visit[addr[i]] = true;
//			}
//		}
//	}
	return false;
}

bool Graph::graph_reach(ID x, ID y, unsigned h) {
	//bool r = false;

	ID x_b_t = newGraph_id[id_belong_tree[x]];
	ID y_b_t = newGraph_id[id_belong_tree[y]];
	if(gLup[id_belong_tree[x_b_t]] <= gLup[id_belong_tree[y_b_t]] || gLdown[id_belong_tree[x_b_t]] >= gLdown[id_belong_tree[y_b_t]]){
		//cout<<x_b_t<<":"<<gLup[id_belong_tree[x_b_t]]<<"  "<<y_b_t<<":"<<gLup[id_belong_tree[y_b_t]]<<endl;
		//cout<<x_b_t<<":"<<gLdown[id_belong_tree[x_b_t]]<<"  "<<y_b_t<<":"<<gLdown[id_belong_tree[y_b_t]]<<endl;
		//cout<<"first gL"<<endl;
		return false;
	}

	//unsigned h = 2;
	if(newGraph_degree[id_belong_tree[x]].outdeg > maxDegree){
		unsigned len = hv[id_belong_tree[y]].size();
		if(len == 1) return false;
		bool result = false;
		for(unsigned i = 1; i < len; i ++){
			if(hv[id_belong_tree[y]][i] == id_belong_tree[x])
				result = true;
		}
		//if(result != hv[id_belong_tree[y]].end()) return true;
		if(result == false) {
			if(len - 1 < h) return false;
			if(newGraph_degree[id_belong_tree[x]].outdeg > newGraph_degree[hv[id_belong_tree[y]][len - 1]].outdeg) return false;
		}

	}

	bool *visit = (bool *)calloc(maxID + 2, sizeof(bool));

	ID *addr = (ID *) newGraph_getOffset(x_b_t);
	assert(addr[0] == x_b_t);
	unsigned count = addr[1];
	addr += 2;

	for(size_t i = 0; i < count; i++) {
		memset(visit, false, sizeof(bool) * (maxID + 2));
		if(addr[i] == y_b_t){
			if(x == x_b_t) return true;
			else {
				ID *addr_ = (ID *)DAG_reverse_getOffset(newID_mapto_oldID[addr[i]]);
				assert(addr_[0] == newID_mapto_oldID[addr[i]]);
				unsigned count_ = addr_[1];
				addr_ += 2;
				for(size_t k = 0; k < count_; k ++) {
					if(id_belong_tree[oldID_mapto_newID[addr_[k]]] == id_belong_tree[x])
						if(tree_reach(x, oldID_mapto_newID[addr_[k]])) return true;
				}
			}
		}
		//else if(addr[i] > y_b_t || newID_mapto_newIDass[id_belong_tree[addr[i]]] > newID_mapto_newIDass[id_belong_tree[y_b_t]]) return false;
		else if(addr[i] > y_b_t) {
			//cout<<addr[i]<<":"<<newID_mapto_newIDass[addr[i]]<<"  "<<y_b_t<<":"<<newID_mapto_newIDass[y_b_t]<<endl;
			//cout<<"ass"<<endl;
			return false;
		}
		else if(newID_mapto_newIDass[addr[i]] > newID_mapto_newIDass[y_b_t]) continue;
		else if(gLup[id_belong_tree[addr[i]]] <= gLup[id_belong_tree[y]] || gLdown[id_belong_tree[addr[i]]] >= gLdown[id_belong_tree[y]]){
			//else if(Lup[addr[i]] <= Lup[y] || Ldown[addr[i]] >= Ldown[y])
			//cout<<"gl"<<endl;
			continue;
		}
		else if(Lup[addr[i]] <= Lup[y] || Ldown[addr[i]] >= Ldown[y]) continue;
		else if(Lup[addr[i]] <= Lup[y_b_t] || Ldown[addr[i]] >= Ldown[y_b_t]) continue;
		else {
			if(x == x_b_t && is_reachable(addr[i], y_b_t, h, visit))
				return true;
			else if(x != x_b_t){
				ID *addr_ = (ID *)DAG_reverse_getOffset(newID_mapto_oldID[addr[i]]);
				assert(addr_[0] == newID_mapto_oldID[addr[i]]);
				unsigned count_ = addr_[1];
				addr_ += 2;
				for(size_t k = 0; k < count_; k ++) {
					if(id_belong_tree[oldID_mapto_newID[addr_[k]]] == id_belong_tree[x])
						if(tree_reach(x, oldID_mapto_newID[addr_[k]])) {
							if(is_reachable(addr[i], y_b_t, h, visit))
								return true;
						}
				}
			}
		}
	} //for i
	return false;
}

bool Graph::newGraph_reachable(ID id1, ID id2) {
	deque<ID>* que  = new deque<ID>();
	que->push_back(id1);
	//cout<<"id:"<<id1<<" ";

	if(id1 == id2) return true;
	while(! que->empty()) {
		ID curID = que->front();
		que->pop_front();
		//cout<<"curID:"<<curID<<endl;
		ID *addr = (ID *) newGraph_getOffset(curID);
		if (addr == NULL){
			cout<<"id:"<<curID<<" newGraph_reachable NULL!"<<endl;
			break;
		}
		assert(curID == addr[0]);
		unsigned count = addr[1];
		//cout<<"count:"<<count<<endl;
		addr += 2; //skip the id and count
		for(size_t i = 0; i < count; i++) {
			if(addr[i] == id2) return true;
			que->push_back(addr[i]);
		}
	}
	return false;
}

bool Graph::find_reach_path(ID x, ID y) {
	unsigned id_num = 0;
	bool r = false;
	ID x_b = newGraph_id[id_belong_tree[x]];
	ID y_b = newGraph_id[id_belong_tree[y]];

	ID *addr = (ID *) newGraph_getOffset(x_b);
	if (addr == NULL){
		cout<<"find_reach_path NULL!"<<endl;
		return NULL;
	}
	assert(x_b == addr[0]);
	unsigned count = addr[1];
	ID *id = (ID *)calloc(count, sizeof(ID));
	//unsigned id_num = 0;
	addr += 2; //skip the id and count

	for(size_t i = 0; i < count; i++) {
		if(newGraph_reachable(addr[i] , y_b)){
			id[id_num] = addr[i];
			id_num ++;
		}
	}

	if(id_num == 0)
		 return false;
	else {
		for(size_t j = 0; j < id_num; j++) {
			//cout<<id[j]<<" ";
			ID *addr = (ID *)DAG_reverse_getOffset(newID_mapto_oldID[id[j]]);
			assert(addr[0] == newID_mapto_oldID[id[j]]);
			unsigned count = addr[1];
			addr += 2;
			for(size_t k = 0; k < count; k ++) {
				if(id_belong_tree[oldID_mapto_newID[addr[k]]] == id_belong_tree[x])
					r = tree_reach(x, oldID_mapto_newID[addr[k]]);
				if(r) {
					//reachable[i] = true;
					return true;
				}
			} //for k
		} //for j
	} //else

	return false;
}

void Graph::vertices_reachability(struct timeval &start_time, struct timeval &end_time1) {
	 srand((unsigned int)time(NULL));
	 ID min = 1, max = idcount;
	 unsigned num = 1000000;
	 ID *random_x = (ID *)calloc(num, sizeof(ID));
	 ID *random_y = (ID *)calloc(num, sizeof(ID));
	 bool *reachable = (bool *)calloc(num, sizeof(bool));
	 memset(reachable, false, num);
	 bool r;
	vector<ID> src;
	vector<ID> dest;
	 for (size_t i = 0; i < num; i++) {
		 random_x[i] = rand() % (max - min + 1) + min;
		 random_y[i] = rand() % (max - min + 1) + min;
		 while (random_x[i] == random_y[i]) {
			 random_y[i] = rand() % (max - min + 1) + min;
		 }
		 src.push_back(random_x[i]);
		 dest.push_back(random_y[i]);
	}

	cout<<"reach query begin:"<<endl;
	gettimeofday(&start_time, NULL);
	for(vector<ID>::iterator x=src.begin(), y=dest.begin(); x!=src.end(); x++, y++) {
		if(id_belong_tree[*y] == id_belong_tree[*x] ) {
			r = tree_reach(*x, *y);
		}
		else {
			r = find_reach_path(*x, *y);
		} //else
	} //for i
	gettimeofday(&end_time1, NULL);
	cout<<"reachability build time elapse:"<<(end_time1.tv_sec - start_time.tv_sec) * 1000.0 + (end_time1.tv_usec - start_time.tv_usec)*1.0/ 1000.0<<" ms"<<endl;
}

void Graph::convert_DAG_adj() {
	init_DAG();
	init_reverse_DAG();
}

//void Graph::pre_code(string path, unsigned is_first, struct timeval &start_time, struct timeval &end_time, struct timeval &end_time1) {
//	cout << "in DFS Encode, maxID: " << maxID << endl;
//
//	string fileName1 = string(path+"/visited_vertices");
//	MMapBuffer *visited_vertices = new MMapBuffer(fileName1.c_str(), (maxID+1) * sizeof(bool));
//	bool *visited_arr = (bool *)(visited_vertices->get_address());
//	memset(visited_arr, false, sizeof(bool) * (maxID+1));
//
//	string fileName2 = string(path+"/cross_vertices");
//	MMapBuffer *cross_vertices = new MMapBuffer(fileName2.c_str(), (maxID+1) * sizeof(bool));
//	bool *cross_vertices_arr = (bool *)(cross_vertices->get_address());
//	//memset(cross_vertices_arr, false, (maxID+1));
//
//	//print_adj();
//	//print_reverse_adj();
//	if(is_first == 1) {
//		ID *scc_tag = (ID *)calloc(maxID + 1, sizeof(ID));
//		memset(scc_tag, 0, sizeof(ID) * (maxID + 1));
//		DFS_Fst(path, scc_tag, visited_arr);
//		cout<<"DFS_Fst finish!"<<endl;
//
//		scc_located = (ID *)calloc(maxID + 1, sizeof(ID));
//		memset(scc_located, 0, sizeof(ID) * (maxID + 1));
//
//		//init(false);
//		//cout<<"init"<<endl;
//		//memset(scc_tag, 0, (maxID + 1));
//		cout<<"dfs_s"<<endl;
//		memset(visited_arr, false, sizeof(bool) * (maxID + 1));
//		DFS_Sec(path, scc_tag, visited_arr);
//		cout<<"DFS_Sec finish!"<<endl;
//		//print_adj();
//
//		cout<<"generate_DAG begin:"<<endl;
//		memset(visited_arr, false, sizeof(bool) * (maxID + 1));
//		//generate_DAG(path, cross_vertices_arr, visited_arr);
//		generate_DAG(path);
//		cout<<"generate_DAG finish!"<<endl;
//		//print_DAG();
//		//print_DAG_degree();
//		gettimeofday(&end_time, NULL);
//	} else {
//		convert_DAG_adj();
//	}
//
//	memset(visited_arr, false, sizeof(bool) * (maxID + 1));
//	memset(cross_vertices_arr, false, sizeof(bool) * (maxID+1));
//	cout<<"dfs_code begin:"<<endl;
//	dfs_code(path, cross_vertices_arr, visited_arr);
//	cout<<"dfs_code finish!"<<endl;
//	//print_newGraph();
//	//print_newID_newGraph();
//	//gettimeofday(&start_time, NULL);
//
////	for(size_t i = 0; i <= maxID; i ++) {
////		if(cross_vertices_arr[i])
////			cout<<"cross:"<<i<<" ";
////	}
//
//	vertices_reachability(start_time, end_time1);
//	//gettimeofday(&end_time, NULL);
//
//	remove(fileName1.c_str());
//	remove(fileName2.c_str());
//	delete cross_vertices;
//	delete visited_vertices;
//}

//void Graph::construct_index(string path) {
//	cout<<"begin:"<<endl;
//	string fileName1 = string(path+"/visited_vertices");
//	MMapBuffer *visited_vertices = new MMapBuffer(fileName1.c_str(), (maxID+1) * sizeof(bool));
//	bool *visited_arr = (bool *)(visited_vertices->get_address());
//	memset(visited_arr, false, sizeof(bool) * (maxID+1));
//
//	string fileName2 = string(path+"/cross_vertices");
//	MMapBuffer *cross_vertices = new MMapBuffer(fileName2.c_str(), (maxID+1) * sizeof(bool));
//	bool *cross_vertices_arr = (bool *)(cross_vertices->get_address());
//
//	memset(visited_arr, false, sizeof(bool) * (maxID + 1));
//	memset(cross_vertices_arr, false, (maxID+1));
//	cout<<"dfs_code begin:"<<endl;
//	dfs_code(path, cross_vertices_arr, visited_arr);
//	cout<<"dfs_code finish!"<<endl;
//
//	remove(fileName1.c_str());
//	remove(fileName2.c_str());
//	delete cross_vertices;
//	delete visited_vertices;
//}

bool Graph::topo_reach(ID x, ID y, bool *cross_vertices_arr, unsigned h) {
	x = oldID_mapto_newID[x];
	y = oldID_mapto_newID[y];
//	ID x_ = newID_mapto_newIDass[id_belong_tree[x]];
//	ID y_ = newID_mapto_newIDass[id_belong_tree[y]];
	ID x_ = newID_mapto_newIDass[x];
	ID y_ = newID_mapto_newIDass[y];
	if(x == y) return true;
	else if(x > y || x_ > y_){
		//cout<<x<<" "<<y<<","<<x_<<" "<<y_;
		//cout<<"x>y"<<endl;
		return false;
	}
	else if(id_belong_tree[y] == id_belong_tree[x] ) {
		//cout<<"tree_reach"<<endl;
		return tree_reach(x, y);
	}
	else {
		if(! cross_vertices_arr[newID_mapto_oldID[newGraph_id[id_belong_tree[y]]]]){
			//cout<<y<<":y root"<<endl;
			return false;
		}
		//else if(Lup[id_belong_tree[x]] <= Lup[id_belong_tree[y]] || Ldown[id_belong_tree[x]] >= Ldown[id_belong_tree[y]])
		else if(Lup[x] <= Lup[y] || Ldown[x] >= Ldown[y]){
			//cout<<"Lup"<<endl;
			return false;
		}
		else {
			//return find_reach_path(x, y);
			//cout<<"graph_reach"<<endl;
			return graph_reach(x, y, h);
		}
	} //else
	return false;
}

void Graph::prepare_query(string path, char *testfilename) {
	ID s , t, label;
	vector<ID> src;
	vector<ID> dest;
	vector<ID> labels;
	TempFile query(path + "/query");
	cout<<"prepare"<<endl;
	if(testfilename!=NULL){
		std::ifstream fstr(testfilename);
		while(!fstr.eof()) {
        	fstr >> s >> t >> label;
        	src.push_back(s);
        	dest.push_back(t);
        	labels.push_back(label);
		}
	}
}

void Graph::check(ID x, ID y){
	x = oldID_mapto_newID[x];
	y = oldID_mapto_newID[y];
	if(id_belong_tree[y] == id_belong_tree[x] ) {
		bool r = tree_reach(x, y);
		cout<<"tree reach :"<<x<<" "<<y<<" :";
		if(r) cout<<"yes"<<endl;
		else cout<<"no"<<endl;
	}
	else {
		bool r = find_reach_path(x, y);
		cout<<"graph reach :"<<x<<" "<<y<<" :";
		if(r) cout<<"yes"<<endl;
		else cout<<"no"<<endl;
	} //else
}

ID Graph::tranverse(ID x, ID y) {
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

void Graph::process(string path, char *testfilename) {
	struct timeval start_time, end_time;
	//start indexing
	cout<<"process index construction:"<<endl;
	gettimeofday(&start_time, NULL);
	string fileName1 = string(path+"/visited_vertices");
	MMapBuffer *visited_vertices = new MMapBuffer(fileName1.c_str(), (maxID+1) * sizeof(bool));
	bool *visited_arr = (bool *)(visited_vertices->get_address());
	//memset(visited_arr, false, sizeof(bool) * (maxID+1));

	string fileName2 = string(path+"/cross_vertices");
	MMapBuffer *cross_vertices = new MMapBuffer(fileName2.c_str(), (maxID+1) * sizeof(bool));
	bool *cross_vertices_arr = (bool *)(cross_vertices->get_address());

	//is_excp(cross_vertices_arr, visited_arr);
	cout<<"construct"<<endl;
	construct(path, cross_vertices_arr, visited_arr);
	//assert(false);
//	cout<<"dfs_code begin:"<<endl;
//	dfs_code(path, cross_vertices_arr, visited_arr, h);
//	cout<<"dfs_code finish!"<<endl;

	gettimeofday(&end_time, NULL);
	cout<<"index construction build time elapse:"<<(end_time.tv_sec - start_time.tv_sec) * 1000 + (end_time.tv_usec - start_time.tv_usec) * 1.0/ 1000.0<<" ms"<<endl;

	cout << "preparing queries..." << endl;
	int label;
	ID s , t;
	vector<ID> src;
	vector<ID> dest;
	vector<ID> labels;

	if(testfilename!=NULL){
		std::ifstream fstr(testfilename);
		while(!fstr.eof()) {
        	fstr >> s >> t >> label;
        	src.push_back(s);
        	dest.push_back(t);
        	labels.push_back(label);
		}
	}
	cout << "queries are ready" << endl;

	// process queries
	cout << "process queries..." << endl;
	bool r;
	int success = 0, fail = 0;
	//unsigned count = 0;
	gettimeofday(&start_time, NULL);
	for(vector<ID>::iterator x=src.begin(), y=dest.begin(); x!=src.end(); x++, y++) {
		//r = topo_reach(*x, *y, cross_vertices_arr, h);
		//count ++;
		ID id1 = *x;
		ID id2 = *y;
		unsigned root_lt = root_located[id1] - 1;
		if(id1 == id2) {
			r = true;
		}
		else if(Lup[id1] <= Lup[id2] || Ldown[id1] >= Ldown[id2])
			r = false;
		else if(root_located[id1] == root_located[id2]){

			if(oldID_mapto_newID[id1] > oldID_mapto_newID[id2]){
				r = false;
				//continue;
			}
			else if(oldID_mapto_newID[id1] <= oldID_mapto_newID[id2] && interval[root_lt][oldID_mapto_newID[id1]] >= interval[root_lt][oldID_mapto_newID[id2]])
				r = true;
			else {
				unsigned i;
				for(i = 0; i < excpID[id1].size(); i ++) {
					if(excpID[id1][i] == id2 || (root_located[excpID[id1][i]] - 1 == root_lt &&
							oldID_mapto_newID[excpID[id1][i]] <= oldID_mapto_newID[id2] &&
									interval[root_lt][oldID_mapto_newID[excpID[id1][i]]] >= interval[root_lt][oldID_mapto_newID[id2]])) {
						r = true;
						break;
					}
				} //for
				if(i == excpID[id1].size())
					r = false;
			}
		}  //if root same
		else {
			if(cross_located[id2] == 0)
				r = false;
			else {
				unsigned clt = cross_located[id2] - 1;
				//assert(crossid_mapto_newID[clt][0] == id2);
				unsigned rlt = 1;
				while(rlt < crossid_mapto_newID[clt].size() && crossid_mapto_newID[clt][rlt] + 1 < root_located[id1]) rlt += 2;
				if(crossid_mapto_newID[clt][rlt] + 1 == root_located[id1]){
					ID id = crossid_mapto_newID[clt][0];

					if(oldID_mapto_newID[id1] > crossid_mapto_newID[clt][rlt + 1]){
						r = false;
						//continue;
					}
					else if(oldID_mapto_newID[id1] <= crossid_mapto_newID[clt][rlt + 1] &&
							interval[root_lt][oldID_mapto_newID[id1]] >= interval[root_lt][crossid_mapto_newID[clt][rlt + 1]])
						r = true;
					else {

						unsigned i;
						for(i = 0; i < excpID[id1].size(); i ++) {
							if(excpID[id1][i] == id ||
									(oldID_mapto_newID[excpID[id1][i]] <= crossid_mapto_newID[clt][rlt + 1] &&
											interval[root_lt][oldID_mapto_newID[excpID[id1][i]]] >= interval[root_lt][crossid_mapto_newID[clt][rlt + 1]])) {
								r = true;
								break;
							}
						} //for
						if(i == excpID[id1].size())
							r = false;

					}
				}
				else
					r = false;
			}
		}  //else root diff
	}
	gettimeofday(&end_time, NULL);
	cout<<"reachability build time elapse:"<<(end_time.tv_sec - start_time.tv_sec) * 1000.0 + (end_time.tv_usec - start_time.tv_usec)*1.0/ 1000.0<<" ms"<<endl;
	//cout << "count: " << count << endl;

	gettimeofday(&start_time, NULL);
	for(vector<ID>::iterator x=src.begin(), y=dest.begin() ,l = labels.begin(); x!=src.end(); x++, y++, l++) {
		ID id1 = *x;
		ID id2 = *y;
		unsigned root_lt = root_located[id1] - 1;
		if(id1 == id2) {
			r = true;
		}
		else if(Lup[id1] <= Lup[id2])
			r = false;
		else if(root_located[id1] == root_located[id2]){

			if(oldID_mapto_newID[id1] > oldID_mapto_newID[id2]){
				r = false;
				//continue;
			}
			else if(oldID_mapto_newID[id1] <= oldID_mapto_newID[id2] && interval[root_lt][oldID_mapto_newID[id1]] >= interval[root_lt][oldID_mapto_newID[id2]])
				r = true;
			else {
				unsigned i;
				for(i = 0; i < excpID[id1].size(); i ++) {
					if(excpID[id1][i] == id2 || (root_located[excpID[id1][i]] - 1 == root_lt &&
							oldID_mapto_newID[excpID[id1][i]] <= oldID_mapto_newID[id2] &&
									interval[root_lt][oldID_mapto_newID[excpID[id1][i]]] >= interval[root_lt][oldID_mapto_newID[id2]])) {
						r = true;
						break;
					}
				} //for
				if(i == excpID[id1].size())
					r = false;
			}
		}  //if root same
		else {
			if(cross_located[id1] != 0 && cross_located[id2] == 0)
				r = false;
			else if(cross_located[id1] == 0 && cross_located[id2] == 0)
				r = false;
			else {
				unsigned clt = cross_located[id2] - 1;
				//assert(crossid_mapto_newID[clt][0] == id2);
				unsigned rlt = 1;
				while(rlt < crossid_mapto_newID[clt].size() && crossid_mapto_newID[clt][rlt] + 1 < root_located[id1]) rlt += 2;
				if(crossid_mapto_newID[clt][rlt] + 1 == root_located[id1]){
					ID id = crossid_mapto_newID[clt][0];

					if(oldID_mapto_newID[id1] > crossid_mapto_newID[clt][rlt + 1]){
						r = false;
						//continue;
					}
					else if(oldID_mapto_newID[id1] <= crossid_mapto_newID[clt][rlt + 1] &&
							interval[root_lt][oldID_mapto_newID[id1]] >= interval[root_lt][crossid_mapto_newID[clt][rlt + 1]])
						r = true;
					else {

						unsigned i;
						for(i = 0; i < excpID[id1].size(); i ++) {
							if(excpID[id1][i] == id ||
									(oldID_mapto_newID[excpID[id1][i]] <= crossid_mapto_newID[clt][rlt + 1] &&
											interval[root_lt][oldID_mapto_newID[excpID[id1][i]]] >= interval[root_lt][crossid_mapto_newID[clt][rlt + 1]])) {
								r = true;
								break;
							}
						} //for
						if(i == excpID[id1].size())
							r = false;

					}
				}
				else
					r = false;
			}
		}  //else root diff
	}
	gettimeofday(&end_time, NULL);
	cout<<"reachability build time elapse:"<<(end_time.tv_sec - start_time.tv_sec) * 1000.0 + (end_time.tv_usec - start_time.tv_usec)*1.0/ 1000.0<<" ms"<<endl;
	cout << "Success Rate " << success << "/" << success+fail << endl;

	gettimeofday(&start_time, NULL);
	for(vector<ID>::iterator x=src.begin(), y=dest.begin() ,l = labels.begin(); x!=src.end(); x++, y++, l++) {
		ID id1 = *x;
		ID id2 = *y;
		unsigned root_lt = root_located[id1] - 1;
		if(id1 == id2) {
			r = true;
		}
		else if(Lup[id1] <= Lup[id2] || Ldown[id1] >= Ldown[id2])
			r = false;
		else if(root_located[id1] == root_located[id2]){

			if(oldID_mapto_newID[id1] > oldID_mapto_newID[id2]){
				r = false;
				//continue;
			}
			else if(oldID_mapto_newID[id1] <= oldID_mapto_newID[id2] && interval[root_lt][oldID_mapto_newID[id1]] >= interval[root_lt][oldID_mapto_newID[id2]])
				r = true;
			else {
				unsigned i;
				for(i = 0; i < excpID[id1].size(); i ++) {
					if(excpID[id1][i] == id2 || (root_located[excpID[id1][i]] - 1 == root_lt &&
							oldID_mapto_newID[excpID[id1][i]] <= oldID_mapto_newID[id2] &&
									interval[root_lt][oldID_mapto_newID[excpID[id1][i]]] >= interval[root_lt][oldID_mapto_newID[id2]])) {
						r = true;
						break;
					}
				} //for
				if(i == excpID[id1].size())
					r = false;
			}
		}  //if root same
		else {
			if(cross_located[id1] != 0 && cross_located[id2] == 0)
				r = false;
			else if(cross_located[id1] == 0 && cross_located[id2] == 0)
				r = false;
			else {
				unsigned clt = cross_located[id2] - 1;
				//assert(crossid_mapto_newID[clt][0] == id2);
				unsigned rlt = 1;
				while(rlt < crossid_mapto_newID[clt].size() && crossid_mapto_newID[clt][rlt] + 1 < root_located[id1]) rlt += 2;
				if(crossid_mapto_newID[clt][rlt] + 1 == root_located[id1]){
					ID id = crossid_mapto_newID[clt][0];

					if(oldID_mapto_newID[id1] > crossid_mapto_newID[clt][rlt + 1]){
						r = false;
						//continue;
					}
					else if(oldID_mapto_newID[id1] <= crossid_mapto_newID[clt][rlt + 1] &&
							interval[root_lt][oldID_mapto_newID[id1]] >= interval[root_lt][crossid_mapto_newID[clt][rlt + 1]])
						r = true;
					else {

						unsigned i;
						for(i = 0; i < excpID[id1].size(); i ++) {
							if(excpID[id1][i] == id ||
									(oldID_mapto_newID[excpID[id1][i]] <= crossid_mapto_newID[clt][rlt + 1] &&
											interval[root_lt][oldID_mapto_newID[excpID[id1][i]]] >= interval[root_lt][crossid_mapto_newID[clt][rlt + 1]])) {
								r = true;
								break;
							}
						} //for
						if(i == excpID[id1].size())
							r = false;

					}
				}
				else
					r = false;
			}
		}  //else root diff

		if(r == true){
			if(*l == 0) {
				fail ++;
				//cout<<id1<<" "<<id2<<":"<<*l<<endl;
			}
			else success ++;
		}
		else {
			if(*l == 1) {
				fail ++;
				//cout<<id1<<" "<<id2<<":"<<*l<<endl;
			}
			else success ++;
		}
	}
	gettimeofday(&end_time, NULL);
	cout<<"reachability build time elapse:"<<(end_time.tv_sec - start_time.tv_sec) * 1000.0 + (end_time.tv_usec - start_time.tv_usec)*1.0/ 1000.0<<" ms"<<endl;
	cout << "Success Rate " << success << "/" << success+fail << endl;

	remove(fileName1.c_str());
	remove(fileName2.c_str());
	delete cross_vertices;
	delete visited_vertices;

	TempFile *old_mapto_new = new TempFile(path + "/old_mapto_new", 0);
	old_mapto_new->write((maxID + 1) * sizeof(ID), (const char*) oldID_mapto_newID);
	old_mapto_new->close();
	delete old_mapto_new;
	free(oldID_mapto_newID);
	oldID_mapto_newID = NULL;
	cout<<"write oldID_mapto_newID"<<endl;

	TempFile *cross_l = new TempFile(path + "/cross_located", 0);
	cross_l->write((maxID + 1) * sizeof(ID), (const char*) cross_located);
	cross_l->close();
	delete cross_l;
	free(cross_located);
	cross_located = NULL;
	cout<<"write cross_located"<<endl;

	TempFile *root_l = new TempFile(path + "/root_located", 0);
	root_l->write((maxID + 1) * sizeof(ID), (const char*) root_located);
	root_l->close();
	delete root_l;
	free(root_located);
	root_located = NULL;
	cout<<"write root_located"<<endl;

	TempFile *up = new TempFile(path + "/Lup", 0);
	up->write((maxID + 1) * sizeof(ID), (const char*) Lup);
	up->close();
	delete up;
	free(Lup);
	Lup = NULL;
	cout<<"write Lup"<<endl;

	TempFile *down = new TempFile(path + "/Ldown", 0);
	down->write((maxID + 1) * sizeof(ID), (const char*) Ldown);
	down->close();
	delete down;
	free(Ldown);
	Ldown = NULL;
	cout<<"write Ldown"<<endl;

	//write_excp();
	//write_interval();
}

void Graph::construct(string path, bool *&cross_vertices_arr, bool *&visited_arr) {
	//get_forward_root_vertices(maxID);

	MemoryMappedFile *rootMap = new MemoryMappedFile();
	assert(rootMap->open((Dir+"/DAGroot.forward.0").c_str()));
	unsigned rootNum = (rootMap->getEnd() - rootMap->getBegin()) / sizeof(ID);
	ID *root_arr = (ID*) rootMap->getBegin();
	//if (rootNum > 0)cout << "root:" << *(root_arr) << "  rootNum:" << rootNum << endl;

	oldID_mapto_newID = (ID *)calloc(maxID + 1, sizeof(ID));
	deque<ID>* que = new deque<ID>();
	excp_count = 0;
	root_located = (ID *)calloc(maxID + 1, sizeof(ID));
	cross_located = (ID *)calloc(maxID + 1, sizeof(ID));
	interval = vector < vector <ID> >(rootNum);
	//TempFile crossID_mapto_newID(path + "/crossID_mapto_newID");
	excpID_O = vector < vector <ID> >(maxID + 1);

	unsigned *tree_degree = (unsigned *)calloc(maxID + 1, sizeof(unsigned));
	string fileName3 = string(path+"/visit_vertices");
	MMapBuffer *visit_vertices = new MMapBuffer(fileName3.c_str(), (maxID+1) * sizeof(bool));
	bool *visit = (bool *)(visit_vertices->get_address());

	memset(visited_arr, false, sizeof(bool) * (maxID + 1));
	memset(cross_vertices_arr, false, (maxID+1));
	mark_cross_vertices(path, root_arr, rootNum, visited_arr, cross_vertices_arr);

	memset(visited_arr, false, sizeof(bool) * (maxID + 1));
	memset(visit, false, sizeof(bool) * (maxID + 1));
	crossid_mapto_newID = vector < vector <ID> >(cross_id.size());
	if(rootNum == 1 && (!degree[root_arr[0]].outdeg || degree[root_arr[0]].indeg))
		cout<<"don't have any root vertices!"<<endl;
	else
	{
		//cout<<"enter for loop"<<endl;
		for(unsigned index = 0;index < rootNum;index++){
			cal_tree_degree(root_arr[index], tree_degree, visit);
			//cout<<index<<" time loop"<<endl;
			//continue;
			que->push_front(root_arr[index]);

			unsigned idcount = 1;
			while(!que->empty()){
				ID curID = que->front();
				que->pop_front();

				if(cross_located[curID] != 0) {
					unsigned clt = cross_located[curID] - 1;
					if(root_located[curID] - 1 == index)
						crossid_mapto_newID[clt].push_back(curID);
					else{
						crossid_mapto_newID[clt].push_back(index);
						crossid_mapto_newID[clt].push_back(idcount);
						idcount ++;

//						for(unsigned k = 0; k < crossid_adj[clt].size(); k ++){
//							//cout<<crossid_adj[clt][k]<<" deg:"<<tree_degree[crossid_adj[clt][k]]<<endl;
//							if(tree_degree[crossid_adj[clt][k]]) tree_degree[crossid_adj[clt][k]] --;
//							if(tree_degree[crossid_adj[clt][k]]){
//								excpID_O[curID].push_back(crossid_adj[clt][k]);
//								excp_count ++;
//								continue;
//							}
//							que->push_front(crossid_adj[clt][k]);
//						}
						continue;
					}
				}

				//cout<<"curID:"<<curID<<"  "<<idcount<<endl;
				oldID_mapto_newID[curID] = idcount;
				idcount++;
				//root_located[curID] = index + 1;
				//visited_arr[curID] = true;

				ID *addr = (ID*) DAG_getOffset(curID);
				unsigned count = addr[1];
				addr += 2; //skip the id and count

				for (size_t i = 0; i < count; i++) {
					//cout<<addr[i]<<" deg:"<<tree_degree[addr[i]]<<endl;
					if(tree_degree[addr[i]])tree_degree[addr[i]]--;
					//if(visited_arr[addr[i]])continue;
					if(tree_degree[addr[i]]){
						excpID_O[curID].push_back(addr[i]);
						excp_count ++;
						continue;
					}//degree if
					que->push_front(addr[i]);
				}//for
			} //while
			interval[index].resize(idcount);
			pt_tranv_tree(root_arr[index], visited_arr);
		} //for
	} //else

	cout<<"excp_count:"<<excp_count<<endl;
	free(tree_degree);
	remove(fileName3.c_str());
	delete visit_vertices;

	unsigned size = 0;
	for(unsigned i = 0; i < rootNum; i ++)
		size += interval[i].size();
	cout<<"interval size:"<<size<<endl;

	size = 0;
	for(unsigned i = 0; i < crossid_mapto_newID.size(); i ++)
		size += crossid_mapto_newID[i].size();
	cout<<"crossid_mapto_newID size:"<<size<<endl;

//	for(size_t index = 0;index <= maxID;index++){
//		cout<<index<<" cross:"<<cross_located[index]<<"  root:"<<root_located[index]<<endl;
//	}
//
//	for(unsigned i = 0; i < cross_id.size(); i ++) {
//		cout<<cross_id[i]<<":"<<crossid_mapto_newID[i][0]<<endl;
//		for(unsigned j = 1; j < crossid_mapto_newID[i].size(); j ++){
//			cout<<crossid_mapto_newID[i][j]<<" ";
//		}
//		cout<<endl;
//	}
//
//	for(size_t index = 0;index <= maxID;index++){
//
//		cout<<"oldID:"<<index<<" newID:"<<oldID_mapto_newID[index]<<" root_located:"<<root_located[index]<<endl;
//	}

	excpID = vector < vector <ID> >(maxID + 1);
	if(excp_count){
		memset(visited_arr, false, sizeof(bool) * (maxID + 1));
		for(unsigned index = 0;index < rootNum;index++)
			cal_tree_excp(root_arr[index], visited_arr);
	}

	size = 0;
	for(unsigned i = 0; i <= maxID; i ++)
		size += excpID[i].size();
	cout<<"excpID size:"<<size<<endl;

//	for(unsigned index = 0;index < rootNum;index++) {
//		cout<<index<<":"<<endl;
//		for(unsigned i = 1;i < interval[index].size();i++){
//			cout<<i<<":"<<interval[index][i]<<endl;
//		}
//	}
//	for(size_t index = 0;index <= maxID;index++){
//		cout<<index<<" excp:";
//		for(unsigned i = 0; i < excpID[index].size(); i ++) {
//			cout<<excpID[index][i]<<" ";
//		}
//		cout<<endl;
//		cout<<index<<" excp_O:";
//		for(unsigned i = 0; i < excpID_O[index].size(); i ++) {
//			cout<<excpID_O[index][i]<<" ";
//		}
//		cout<<endl;
//	}
	cout<<"cal_tree_excp end"<<endl;

	optimization(root_arr, rootNum, visited_arr);
	cout<<"optimization end"<<endl;

	vector< vector <ID> >().swap(excpID_O);
	delete que;
	que = NULL;
	rootMap->close();
	delete rootMap;
}

void Graph::mark_cross_vertices(string path, ID *root_arr, unsigned rootNum, bool *&visited_arr, bool *&cross_vertices_arr){
	deque<ID>* que = new deque<ID>();
	ID curID, component;
	unsigned count;
	//unsigned cross_idcount = 0;


	string fileName = string(path+"/tmpvalue");
	MMapBuffer *tmpvalue = new MMapBuffer(fileName.c_str(), maxID * sizeof(ID));
	ID *tmpvalue_arr = (ID *)(tmpvalue->get_address());

	for(size_t index = 0;index < rootNum;index++){
		if(visited_arr[root_arr[index]])continue;
		visited_arr[root_arr[index]] = true;
		que->push_back(root_arr[index]);
		component = root_arr[index];
		tmpvalue_arr[root_arr[index]] = component;

		//idcount = 0;
		while(!que->empty()){
			curID = que->front();
			que->pop_front();
			//idcount ++;
			root_located[curID] = index + 1;
			ID *addr = (ID*) DAG_getOffset(curID);
			//if (addr == NULL)continue;
			//assert(curID == addr[0]);
			count = addr[1];
			addr += 2; //skip the id and count

			for (size_t i = 0; i < count; i++) {
				if(visited_arr[addr[i]]){
					if(tmpvalue_arr[addr[i]] != component){
						if(!cross_vertices_arr[addr[i]]) {
							cross_vertices_arr[addr[i]] = true;
							//cross_idcount ++;
							cross_id.push_back(addr[i]);
							//cout<<"crossid:"<<addr[i]<<endl;
						}
					}
					continue;
				}
				visited_arr[addr[i]] = true;
				que->push_back(addr[i]);
				tmpvalue_arr[addr[i]] = component;
			}
		}
		//interval[index].resize(idcount);
	}

	//bool rlt_same;
	memset(visited_arr, false, sizeof(bool) * (maxID + 1));
	crossid_adj = vector < vector <ID> >(cross_id.size());
	sort(cross_id.begin(), cross_id.end());
	TempFile crossadj(path + "/crossadj");
	for(unsigned i = 0; i < cross_id.size(); i ++){
		que->push_back(cross_id[i]);
		visited_arr[cross_id[i]] = true;
		//vector <ID> temp_adj;

		//rlt_same = true;
		while(!que->empty()){
			curID = que->front();
			que->pop_front();
			cross_located[curID] = i + 1;
			//cout<<curID<<"  ";
			ID *addr = (ID*) DAG_getOffset(curID);
			count = addr[1];
			addr += 2; //skip the id and count
			for (size_t j = 0; j < count; j++) {
				if(cross_vertices_arr[addr[j]]){
					//temp_adj.push_back(addr[j]);
					crossadj.writeId(i);
					crossadj.writeId(addr[j]);
				}
				else if(!visited_arr[addr[j]]){
					que->push_back(addr[j]);
					visited_arr[addr[j]] = true;
				}
			}
		}
//		if(!temp_adj.empty()) {
//			sort(temp_adj.begin(), temp_adj.end());
//			crossid_adj[i].push_back(temp_adj[0]);
//			for(unsigned k = 1; k < temp_adj.size(); k ++){
//				if(temp_adj[k] != temp_adj[k - 1])
//					crossid_adj[i].push_back(temp_adj[k]);
//			}
//		}
//		vector <ID>().swap(temp_adj);
	}

	crossadj.flush();

	TempFile* sortFile = new TempFile("sortFile");
	Sorter::sort(crossadj, *sortFile, TempFile::skipIdId, TempFile::compare12);
	TempFile* r_sortFile = new TempFile("r_sortFile");
	Sorter::sort(crossadj, *r_sortFile, TempFile::skipIdId, TempFile::compare21);
	remove(crossadj.getFile().c_str());
	cout<<"sort end"<<endl;

	MemoryMappedFile mappedIn;
	assert(mappedIn.open(sortFile->getFile().c_str()));
	const char* reader = mappedIn.getBegin(), *limit = mappedIn.getEnd();
	ID from, to, lastfrom, lastto;
	bool firstinsert = true;
	lastfrom = *(ID*) reader;
	//lastto = *(ID*) (reader + sizeof(ID));
	while (reader < limit) {
		from = *(ID*) reader;
		reader += sizeof(ID);
		to = *(ID*) reader;
		reader += sizeof(ID);
		if(firstinsert){
			crossid_adj[from].push_back(to);
			lastto = to;
			firstinsert = false;
		}
		else if(lastfrom != from){
			lastfrom = from;
			crossid_adj[from].push_back(to);
			lastto = to;
		}
		else if(lastto != to){
			crossid_adj[from].push_back(to);
			lastto = to;
		}
	}
	count = 0;
	for(unsigned i = 0; i < crossid_adj.size(); i ++){
		count += crossid_adj[i].size();
	}
	cout<<"crossid_adj:"<<count<<endl;

	//cross id reverse adj
	crossid_r_adj = vector < vector <ID> >(cross_id.size());
	unsigned* crossdeg = (unsigned *)calloc(cross_id.size(), sizeof(unsigned));
	assert(mappedIn.open(r_sortFile->getFile().c_str()));
	reader = mappedIn.getBegin(), limit = mappedIn.getEnd();
	firstinsert = true;
	lastfrom = *(ID*) reader;
	while (reader < limit) {
		from = *(ID*) reader;
		reader += sizeof(ID);
		to = *(ID*) reader;
		to = cross_located[to] - 1;
		reader += sizeof(ID);
		if(firstinsert){
			crossdeg[from] ++;
			crossid_r_adj[to].push_back(from);
			lastfrom = from;
			firstinsert = false;
		} else if(lastto != to){
			crossdeg[from] ++;
			lastto = to;
			crossid_r_adj[to].push_back(from);
			lastfrom = from;
		}
		else if(lastfrom != from){
			crossdeg[from] ++;
			crossid_r_adj[to].push_back(from);
			lastfrom = from;
		}
	}

	unsigned deg = 0;
	cross_located_arr = vector < vector <ID> >(cross_id.size());
	bool* visited = (bool *)calloc(cross_id.size(), sizeof(bool));
	memset(visited, false, sizeof(bool) * cross_id.size());
	//sort(crossdeg, crossdeg + cross_id.size());
	idcount = 1;
	DAG_maxID = 0;
	TempFile crosslocatedarr(path + "/crossadj");
	cout<<"cal_cross_located_arr"<<endl;
	while(deg < cross_id.size()){
		//cout<<"deg:"<<deg<<endl;

		if(crossdeg[deg] != 0) {
			deg ++;
			continue;
		} else {
			if(crossid_r_adj[deg].size() == 0){
				visited[deg] = true;
				deg ++;
				//idcount ++;
				//cout<<idcount<<endl;
				continue;
			}
			cal_cross_located_arr(deg, visited, crosslocatedarr);
			deg ++;
		}
	}
	crosslocatedarr.flush();
	cout<<"end"<<endl;

	TempFile* crosslocatedarr_sortFile = new TempFile("crosslocatedarr_sortFile");
	Sorter::sort(crosslocatedarr, *crosslocatedarr_sortFile, TempFile::skipIdId, TempFile::compare12);
	remove(crosslocatedarr.getFile().c_str());
	cout<<"sort end"<<endl;

	count = 0;
	for(unsigned i = 0; i < crossid_adj.size(); i ++){
		count += cross_located_arr[i].size();
	}
	cout<<"cross_located_arr:"<<count<<endl;
	cout<<"DAG_maxID:"<<DAG_maxID<<endl;
	mappedIn.close();
	r_sortFile->discard();
	delete r_sortFile;
	sortFile->discard();
	delete sortFile;

	free(visited);
	visited = NULL;

	cout<<"cross_idcount"<<cross_id.size()<<endl;
	remove(fileName.c_str());
	delete tmpvalue;
	delete que;
	que = NULL;
}

void Graph::cal_cross_located_arr(unsigned clt, bool *&visited_arr, TempFile &crosslocatedarr){
	//cout<<"clt:"<<clt<<endl;
	//idcount ++;
	//cout<<idcount<<endl;
	visited_arr[clt] = true;
	if(crossid_r_adj[clt].size() == 0)return;
	vector <ID> temp;
	bool* visited = (bool *)calloc(cross_id.size(), sizeof(bool));
	memset(visited, false, sizeof(bool) * cross_id.size());
	for(unsigned i = 0; i < crossid_r_adj[clt].size(); i ++){
		if(!visited_arr[crossid_r_adj[clt][i]]){
			cal_cross_located_arr(crossid_r_adj[clt][i], visited_arr, crosslocatedarr);
		}
		for(unsigned k = 0; k < cross_located_arr[crossid_r_adj[clt][i]].size(); k ++){
			if(!visited[cross_located_arr[crossid_r_adj[clt][i]][k]]){
				//cross_located_arr[clt].push_back(cross_located_arr[crossid_r_adj[clt][i]][k]);
				crosslocatedarr.writeId(clt);
				crosslocatedarr.writeId(cross_located_arr[crossid_r_adj[clt][i]][k]);
				visited[cross_located_arr[crossid_r_adj[clt][i]][k]] = true;
			}
		}
		if(!visited[crossid_r_adj[clt][i]]){
			//cross_located_arr[clt].push_back(crossid_r_adj[clt][i]);
			crosslocatedarr.writeId(clt);
			crosslocatedarr.writeId(crossid_r_adj[clt][i]);
			visited[crossid_r_adj[clt][i]] = true;
		}
	}
//	if(cross_located_arr[clt].size() > 0){
//		sort(cross_located_arr[clt].begin(), cross_located_arr[clt].end());
//		if(cross_located_arr[clt].size() > DAG_maxID)
//			DAG_maxID = cross_located_arr[clt].size();
//
////		cross_located_arr[clt].push_back(temp[0]);
////		for(unsigned i = 1; i < temp.size(); i ++){
////			if(temp[i] != temp[i - 1])
////				cross_located_arr[clt].push_back(temp[i]);
////		}
//	}
	free(visited);
	visited = NULL;
}

void Graph::cal_tree_degree(ID root, unsigned *&tree_degree, bool *&visit) {
	//memset(visit, false, sizeof(bool) * (maxID + 1));
	deque<ID>* que = new deque<ID>();
	que->push_back(root);
	visit[root] = true;
	while(!que->empty()){
		ID curID = que->front();
		que->pop_front();

//		if(root_located[curID] != root_located[root]){
//			unsigned clt = cross_located[curID] - 1;
//			for (size_t i = 0; i < crossid_adj[clt].size(); i++) {
//				tree_degree[crossid_adj[clt][i]] ++;
//				if(!visit[crossid_adj[clt][i]]){
//					que->push_back(crossid_adj[clt][i]);
//					visit[crossid_adj[clt][i]] = true;
//				}
//			}
//			continue;
//		}
		ID *addr = (ID*) DAG_getOffset(curID);
		unsigned count = addr[1];
		addr += 2; //skip the id and count

		for (size_t i = 0; i < count; i++) {
			tree_degree[addr[i]] ++;
			if(visit[addr[i]]){
				continue;
			}
			que->push_back(addr[i]);
			visit[addr[i]] = true;
		}
	}
	delete que;
	que=NULL;
}

ID Graph::pt_tranv_tree(ID id, bool *&visited_arr){
	ID *addr = (ID*) DAG_getOffset(id);
	unsigned count = addr[1];
	addr += 2; //skip the id and count

	ID max = oldID_mapto_newID[id];
	unsigned rlt_id = root_located[id] - 1;
	for (unsigned i = 0; i < count; i++){
		if(!excpID_O[id].size() || !is_excp(id, addr[i])) {
			//cout<<"id:"<<id<<" "<<addr[i]<<endl;
			if(root_located[addr[i]] - 1 == rlt_id){
				if(! visited_arr[addr[i]]){
					interval[rlt_id][oldID_mapto_newID[addr[i]]] = pt_tranv_tree(addr[i], visited_arr);
					visited_arr[addr[i]] = true;
				}
				if(max < interval[rlt_id][oldID_mapto_newID[addr[i]]])
					max = interval[rlt_id][oldID_mapto_newID[addr[i]]];
			}
			else {
				unsigned clt = cross_located[addr[i]] - 1;
//				if(crossid_mapto_newID[clt][0] != addr[i]){
//					cout<<"clt:"<<clt<<" cross_tag:"<<cross_tag[clt]<<endl;
//					cout<<"crossid_mapto_newID:"<<crossid_mapto_newID[clt][0] <<" "<< addr[i]<<endl;
//				}
				assert(crossid_mapto_newID[clt][0] == addr[i]);
				unsigned len = crossid_mapto_newID[clt].size() - 1;
				assert(crossid_mapto_newID[clt][len - 1] == rlt_id);
				interval[rlt_id][crossid_mapto_newID[clt][len]] = crossid_mapto_newID[clt][len];
				if(max < crossid_mapto_newID[clt][len])
					max = crossid_mapto_newID[clt][len];
			}
		}
	}
	return max;
}

void Graph::deal_with_crossid_interval() {
	for(unsigned i = 0; i < cross_id.size(); i ++){
		ID *addr = (ID*) DAG_reverse_getOffset(crossid_mapto_newID[i][0]);
		assert(addr[0] == crossid_mapto_newID[i][0]);
		unsigned count = addr[1];
		addr += 2;
		unsigned rlt_c;
		for(unsigned k = 0; k < count; k ++){
			if(!excpID_O[addr[k]].size() || !is_excp(addr[k], crossid_mapto_newID[i][0])) {
				unsigned rlt = root_located[addr[k]];
				if(k > 0 && rlt == root_located[addr[k - 1]]){
					interval[rlt - 1][oldID_mapto_newID[addr[k]]] = std::max(crossid_mapto_newID[i][rlt_c - 1], interval[rlt - 1][oldID_mapto_newID[addr[k]]]);
					continue;
				}
				rlt_c = 2;
				while(crossid_mapto_newID[i][rlt_c] + 1 != rlt) rlt_c += 2;
				interval[rlt - 1][crossid_mapto_newID[i][rlt_c - 1]] = crossid_mapto_newID[i][rlt_c - 1];
				interval[rlt - 1][oldID_mapto_newID[addr[k]]] = std::max(crossid_mapto_newID[i][rlt_c - 1], interval[rlt - 1][oldID_mapto_newID[addr[k]]]);
			}
		}
	}
}

void Graph::cal_tree_interval(ID *root_arr, unsigned rootNum, bool *&visited_arr) {
	//deal_with_crossid_interval();
	//cout<<"deal_with_crossid_interval end"<<endl;
	unsigned *cross_tag = (unsigned *)calloc(crossid_mapto_newID.size(), sizeof(unsigned));
	for(unsigned i = 0; i < crossid_mapto_newID.size(); i ++)
		cross_tag[i] = 1;
	//cout<<"cross_tag size:"<<crossid_mapto_newID.size()<<endl;
	//cout<<"begin:"<<endl;
	for(size_t index = 0;index < rootNum;index++){
		memset(visited_arr, false, sizeof(bool) * (maxID + 1));
		interval[index][oldID_mapto_newID[root_arr[index]]] = pt_tranv_tree(root_arr[index], visited_arr, cross_tag);
		//cout<<root_arr[index]<<"根标签："<<interval[index][oldID_mapto_newID[root_arr[index]]]<<endl;
		visited_arr[root_arr[index]] = true;
	}
	free(cross_tag);
}

ID Graph::pt_tranv_tree(ID id, bool *&visited_arr, unsigned *&cross_tag) {
	ID *addr = (ID*) DAG_getOffset(id);
	//if (addr[1] == 0) return max;
	unsigned count = addr[1];
	addr += 2; //skip the id and count
	//unsigned rlt_id = root_located[id];

	//ID max = std::max(oldID_mapto_newID[id], interval[rlt_id - 1][oldID_mapto_newID[id]]);
	ID max = oldID_mapto_newID[id];
	for (size_t i = 0; i < count; i++){
		//if(root_located[addr[i]] != rlt_id) continue;
		if(!excpID_O[id].size() || !is_excp(id, addr[i])) {
			//cout<<"id:"<<id<<" "<<addr[i]<<endl;
			unsigned rlt_id = root_located[id] - 1;
			if(root_located[addr[i]] - 1 == rlt_id){
				if(! visited_arr[addr[i]]){
					interval[rlt_id][oldID_mapto_newID[addr[i]]] = pt_tranv_tree(addr[i], visited_arr, cross_tag);
					visited_arr[addr[i]] = true;
				}
				if(max < interval[rlt_id][oldID_mapto_newID[addr[i]]])
					max = interval[rlt_id][oldID_mapto_newID[addr[i]]];
			}
			else {
				unsigned clt = cross_located[addr[i]] - 1;
//				if(crossid_mapto_newID[clt][0] != addr[i]){
//					cout<<"clt:"<<clt<<" cross_tag:"<<cross_tag[clt]<<endl;
//					cout<<"crossid_mapto_newID:"<<crossid_mapto_newID[clt][0] <<" "<< addr[i]<<endl;
//				}
				assert(crossid_mapto_newID[clt][0] == addr[i]);
				if(crossid_mapto_newID[clt][cross_tag[clt]] != rlt_id) cross_tag[clt] += 2;
//				if(crossid_mapto_newID[clt][cross_tag[clt]] != rlt_id) {
//					cout<<cross_tag[clt]<<endl;
//					cout<<"root_Lo:"<<crossid_mapto_newID[clt][cross_tag[clt]]<<"  "<<rlt_id<<endl;
//				}
				assert(crossid_mapto_newID[clt][cross_tag[clt]] == rlt_id);
				//interval[rlt_id][crossid_mapto_newID[clt][cross_tag[clt] + 1]] = crossid_mapto_newID[clt][cross_tag[clt] + 1];
				if(! visited_arr[addr[i]]){
					interval[rlt_id][crossid_mapto_newID[clt][cross_tag[clt] + 1]] = pt_tran_cross_id(addr[i],  visited_arr, cross_tag);
					visited_arr[addr[i]] = true;
				}
				if(max < interval[rlt_id][crossid_mapto_newID[clt][cross_tag[clt] + 1]])
					max = interval[rlt_id][crossid_mapto_newID[clt][cross_tag[clt] + 1]];
			}
		}
	}

	return max;
}

ID Graph::pt_tran_cross_id(ID id, bool *&visited_arr, unsigned *&cross_tag) {
	unsigned clt_id = cross_located[id] - 1;
	ID max = crossid_mapto_newID[clt_id][cross_tag[clt_id] + 1];
	unsigned rlt = crossid_mapto_newID[clt_id][cross_tag[clt_id]];

	for(unsigned i = 0; i < crossid_adj[clt_id].size(); i ++) {
		unsigned clt = cross_located[crossid_adj[clt_id][i]] - 1;
//		if(crossid_mapto_newID[clt][0] != crossid_adj[clt_id][i]){
//			cout<<"clt:"<<clt<<" cross_tag:"<<cross_tag[clt]<<endl;
//			cout<<"crossid_mapto_newID:"<<crossid_mapto_newID[clt][0] <<" "<< crossid_adj[clt_id][i]<<endl;
//		}
		assert(crossid_mapto_newID[clt][0] == crossid_adj[clt_id][i]);
		if(crossid_mapto_newID[clt][cross_tag[clt]] != rlt) cross_tag[clt] += 2;
//		if(crossid_mapto_newID[clt][cross_tag[clt]] != rlt) {
//			cout<<cross_tag[clt]<<endl;
//			cout<<"root_Lo:"<<crossid_mapto_newID[clt][cross_tag[clt]]<<"  "<<rlt<<endl;
//		}
		assert(crossid_mapto_newID[clt][cross_tag[clt]] == rlt);

		if(! visited_arr[crossid_adj[clt_id][i]]){
			interval[rlt][crossid_mapto_newID[clt][cross_tag[clt] + 1]] = pt_tran_cross_id(crossid_adj[clt_id][i],  visited_arr, cross_tag);
			visited_arr[crossid_adj[clt_id][i]] = true;
		}
		if(max < interval[rlt][crossid_mapto_newID[clt][cross_tag[clt] + 1]])
			max = interval[rlt][crossid_mapto_newID[clt][cross_tag[clt] + 1]];
	}

	return max;
}

bool Graph::is_excp(ID id1, ID id2) {
	unsigned i = 0;
	while(i < excpID_O[id1].size() && excpID_O[id1][i] < id2) i++;
	if(i < excpID_O[id1].size() && id2 == excpID_O[id1][i]) return true;
	return false;
}

void Graph::cal_tree_excp(ID id, bool *&visited_arr) {
	ID *addr = (ID*) DAG_getOffset(id);
	unsigned count = addr[1];
	addr += 2; //skip the id and count
	unsigned root_lt = root_located[id];
	for(unsigned i = 0; i < count; i ++){
		if(root_located[addr[i]] == root_lt) {
			if(!visited_arr[addr[i]])
				cal_tree_excp(addr[i], visited_arr);
			for(unsigned j = 0; j < excpID[addr[i]].size(); j ++)
				excpID_O[id].push_back(excpID[addr[i]][j]);
		}
	}
	visited_arr[id] = true;

	if(excpID_O[id].size() > 0){
		sort(excpID_O[id].begin(), excpID_O[id].end());
		ID lastid = excpID_O[id][0];
		if(oldID_mapto_newID[id] > oldID_mapto_newID[lastid] || interval[root_lt - 1][oldID_mapto_newID[id]] < interval[root_lt - 1][oldID_mapto_newID[lastid]])
			excpID[id].push_back(lastid);
		for(unsigned i = 1; i < excpID_O[id].size(); i ++){
			if(lastid == excpID_O[id][i]) continue;
			if(oldID_mapto_newID[id] <= oldID_mapto_newID[excpID_O[id][i]] && interval[root_lt - 1][oldID_mapto_newID[id]] >= interval[root_lt - 1][oldID_mapto_newID[excpID_O[id][i]]]){
				lastid = excpID_O[id][i];
				continue;
			}
			excpID[id].push_back(excpID_O[id][i]);
			lastid = excpID_O[id][i];
		}
	}
}

void Graph::print_adj() {
	cout<<"pring_adj:"<<endl;
	unsigned count;
	unsigned ct = 0;
	cout<<"maxFromID:"<<maxFromID<<" start_ID[0]:"<<start_ID[0]<<endl;
	for(ID i = start_ID[0]; i <= maxID; i ++) {
		ID *addr = (ID*) getOffset(i);
		cout<<i<<" adj:";
		ct ++;
		if (addr == NULL){
			cout<<"null"<<endl;
			continue ;
		}
		//cout<<"curID:"<<i<<" "<<addr[0]<<endl;
		assert(i == addr[0]);
		count = addr[1];
		addr += 2; //skip the id and count

		for(size_t i = 0; i < count; i++) {
			cout<<addr[i]<<" ";
		}
		cout<<"number:"<<count<<endl;

	}
	cout<<ct<<endl;
}

void Graph::print_reverse_adj() {
	cout<<"print_reverse_adj"<<endl;
	unsigned count;
	unsigned ct = 0;
	cout<<"maxFromID:"<<maxFromID<<" reverse_start_ID[0]:"<<reverse_start_ID[0]<<endl;
	for(ID i = reverse_start_ID[0]; i <= maxID; i ++) {
		ID *addr = (ID*) reverse_getOffset(i);
		cout<<i<<" reverse adj:";
		ct ++;
		if (addr == NULL){
			cout<<"null"<<endl;
			continue ;
		}
		//cout<<"curID:"<<curID<<" "<<addr[0]<<endl;
		assert(i == addr[0]);
		count = addr[1];
		addr += 2; //skip the id and count

		for(size_t i = 0; i < count; i++) {
			cout<<addr[i]<<" ";
		}
		cout<<"number:"<<count<<endl;

	}
	cout<<ct<<endl;
}

void Graph::print_DAG() {
	cout<<"print_DAG:"<<endl;
	for(ID i = DAG_start_ID[0]; i <= maxID; i ++) {
		//cout<<"for"<<endl;
		ID *addr = (ID*) DAG_getOffset(i);
		cout<<i<<" adj:";
		//cout<<"off"<<endl;
		if (addr == NULL){
			cout<<"null"<<endl;
			continue ;
		}
		//cout<<"curID"<<i<<" addr[0]"<<addr[0]<<endl;
		//cout<<"curID:"<<i+1<<"addr1:"<<addr1[0]<<endl;
		assert(i == addr[0]);
//		if (addr == NULL){
//			cout<<"null"<<endl;
//			continue ;
//		}
		//cout<<"here"<<endl;
		unsigned count = addr[1];
		addr += 2; //skip the id and count
		for(size_t k = 0; k < count; k++) {
			cout<<addr[k]<<" ";
		}
		//cout<<endl;
		//cout<<"嘿"<<endl;
		cout<<endl;
		ID *addr1 = (ID*) DAG_reverse_getOffset(i);
		cout<<"  reverse adj: ";
		assert(i == addr1[0]);
		count = addr1[1];
		addr1 += 2;
		for(size_t k = 0; k < count; k ++) {
			cout<<addr1[k]<<" ";
		}

		cout<<endl;
	}

}

void Graph::print_DAG_degree() {
	for(ID i = DAG_start_ID[0]; i <= maxID; i ++) {
		if(scc_located[i] != 0)
			cout<<i<<" in:"<<degree[i].indeg<<" out:"<<degree[i].outdeg<<endl;
	}
}

void Graph::print_scc() {
	cout<<"print_scc:"<<endl;
	for(ID i = 0; i < scc_num; i ++) {
		ID *addr = (ID*) scc_getOffset(scc_start_ID[i]);
		cout<<scc_start_ID[i]<<" : ";
		if (addr == NULL){
			cout<<"null"<<endl;
		}
		//cout<<"curID:"<<curID<<" "<<addr[0]<<endl;
		assert(scc_start_ID[i] == addr[0]);
		unsigned count = addr[1];
		addr += 2; //skip the id and count

		for(size_t i = 0; i < count; i++) {
			cout<<addr[i]<<" ";
		}
		cout<<"num:"<<count<<endl;
	}
}

void Graph::print_excp() {
	cout<<"print_excp:"<<endl;
	unsigned count;
	//unsigned ct = 0;
	//cout<<"excp_start_ID[0]:"<<excp_start_ID[0]<<" idcount:"<<idcount<<" DAG_maxID:"<<DAG_maxID<<endl;
	for(unsigned i = 0; i < excp_idcount; i ++) {
		ID *addr = (ID*) excp_getOffset(i);
		cout<<i<<" :";
		if (addr == NULL){
			cout<<"null"<<endl;
			continue ;
		}
		//cout<<"curID:"<<i<<" "<<addr[0]<<endl;
		assert(i == addr[0]);
		count = addr[1];
		addr += 2; //skip the id and count

		cout<<addr[0]<<"异常点：";
		for(size_t j = 1; j < count; j++) {
			cout<<addr[j]<<" ";
		}
		cout<<"个数："<<count - 1<<endl;

	}
	//cout<<ct<<endl;
}
void Graph::print_newGraph() {
	for(unsigned i = 0; i < newGraph_idcount; i ++) {
		cout<<i<<": "<<newGraph_id[i]<<" ";
	}
	cout<<endl;

	cout<<"print_newGraph:"<<endl;
	unsigned count = newGraph_idcount;
	ID *id = (ID *)calloc(newGraph_idcount + 2, sizeof(ID));
	//if(newGraph_idcount > 20)  count = 20;
	//else count = newGraph_idcount;
	for(unsigned i = 0; i < count; i ++) {
		ID *addr = (ID*) newGraph_getOffset(newGraph_id[i]);
		cout<<newID_mapto_oldID[newGraph_id[i]]<<" newGraph：";
		//ct ++;
		if (addr == NULL){
			cout<<"null"<<endl;
			continue ;
		}
		//cout<<"curID:"<<newGraph_id[i]<<" "<<addr[0]<<endl;
		assert(newGraph_id[i] == addr[0]);
		unsigned count = addr[1];
		addr += 2; //skip the id and count

		for(size_t j = 0; j < count; j++) {
			cout<<newID_mapto_oldID[addr[j]]<<" ";
		}
		cout<<"个数："<<count<<endl;
	}
	//新点
	cout<<"newID:"<<endl;
	for(size_t i = 0; i < count; i ++) {

		ID *addr = (ID*) newGraph_getOffset(newGraph_id[i]);
		cout<<newGraph_id[i]<<" newGraph：";
		//ct ++;
		if (addr == NULL){
			cout<<"null"<<endl;
			continue ;
		}
		//cout<<"curID:"<<i<<" "<<addr[0]<<endl;
		assert(newGraph_id[i] == addr[0]);
		unsigned count = addr[1];
		addr += 2; //skip the id and count

		for(size_t j = 0; j < count; j++) {
			cout<<addr[j]<<" ";
		}
		cout<<"个数："<<count<<endl;

		addr = (ID*) reverse_newGraph_getOffset(newGraph_id[i]);
		cout<<newGraph_id[i]<<" 逆：";
		//ct ++;
		if (addr == NULL){
			cout<<"null"<<endl;
			continue ;
		}
		cout<<"curID:"<<newGraph_id[i]<<" "<<addr[0]<<endl;
		assert(newGraph_id[i] == addr[0]);
		count = addr[1];
		addr += 2; //skip the id and count

		for(size_t j = 0; j < count; j++) {
			cout<<addr[j]<<" ";
		}
		cout<<"个数："<<count<<endl;
	}
}

void Graph::print_newID_newGraph() {
	cout<<"print_newID_newGraph:"<<endl;
	unsigned count = newGraph_idcount;
	ID *id = (ID *)calloc(newGraph_idcount + 2, sizeof(ID));
	unsigned idc = 0;
	//if(newGraph_idcount > 20)  count = 20;
	//else count = newGraph_idcount;
	for(size_t i = 0; i < count; i ++) {
		id[idc ++] = newGraph_id[i];
		ID *addr = (ID*) newGraph_getOffset(newGraph_id[i]);
		cout<<newGraph_id[i]<<" newGraph：";
		//ct ++;
		if (addr == NULL){
			cout<<"null"<<endl;
			continue ;
		}
		//cout<<"curID:"<<i<<" "<<addr[0]<<endl;
		assert(newGraph_id[i] == addr[0]);
		unsigned count = addr[1];
		addr += 2; //skip the id and count

		for(size_t j = 0; j < count; j++) {
			cout<<addr[j]<<" ";
		}
		cout<<"个数："<<count<<endl;
	}

	qsort(id, idc, sizeof(ID), qcompare);
	ID d = id[0];
	for(size_t i = 1; i < idc; i ++) {
		if(d == id[i]) {
			assert(0 == 1);
		}
		d = id[i];
	}
}

void Graph::write_interval() {
	unsigned num = interval.size();

	bool firstTime = true;
	int fileindex = -1;
	unsigned fileOff = 0;
	EntityIDBuffer* tempEnt = new EntityIDBuffer();
	vector<TempFile*> adjFile;
	size_t pageSize = num * sizeof(ID) / (MemoryBuffer::pagesize) + (((num * sizeof(ID)) % (MemoryBuffer::pagesize)) ? 1 : 0);
	char adj_indexPath[150];
	sprintf(adj_indexPath, "%s/interval_adj_index.0", Dir.c_str());
	MMapBuffer *global_adj_indexFile = new MMapBuffer(adj_indexPath, 2 * pageSize * MemoryBuffer::pagesize);
	unsigned *global_adj_index = (unsigned *)global_adj_indexFile->get_address();

	for(unsigned i = 0; i < num; i ++) {
		tempEnt->insertID(i);
		for(unsigned j = 1; j < interval[i].size(); j ++)
			tempEnt->insertID(interval[i][j]);

		unsigned num = tempEnt->getSize() - 2;
		tempEnt->getBuffer()[1] = num; //update num
		unsigned size = tempEnt->getSize() * sizeof(ID);
		if (!firstTime && fileOff + size < (unsigned) (-1) / 8) {
			adjFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
			global_adj_index[2 * i] = fileOff;
			global_adj_index[2 * i + 1] = fileindex;
			fileOff += size;
		} else {
			if (fileindex >= 0) {
				adjFile[fileindex]->close();
			}

			fileindex++;
			fileOff = 0;
			adjFile.push_back(new TempFile(Dir + "/interval_adj", fileindex));
			adjFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
			global_adj_index[2 * i] = 1;//actually is '0', but we set '1' here, for '0' has been used to mark the unexist 'to data'
			global_adj_index[2 * i + 1] = fileindex;
			fileOff += size;
			firstTime = false;

			cout << "fileIndex: " << fileindex << "i:" << i << endl;
		}
	}

	adjFile[fileindex]->close();
	global_adj_indexFile->flush();
	cout<<"success write interval list"<<endl;

	for(int i = 0;i < adjFile.size();i++){
		delete adjFile[i];
		adjFile[i] = NULL;
	}
	adjFile.clear();
	delete global_adj_indexFile;
	delete tempEnt;
}
void Graph::write_excp() {
	unsigned num = excpID.size();

	bool firstTime = true;
	int fileindex = -1;
	unsigned fileOff = 0;
	EntityIDBuffer* tempEnt = new EntityIDBuffer();
	vector<TempFile*> adjFile;
	size_t pageSize = num * sizeof(ID) / (MemoryBuffer::pagesize) + (((num * sizeof(ID)) % (MemoryBuffer::pagesize)) ? 1 : 0);
	char adj_indexPath[150];
	sprintf(adj_indexPath, "%s/excpID_adj_index.0", Dir.c_str());
	MMapBuffer *global_adj_indexFile = new MMapBuffer(adj_indexPath, 2 * pageSize * MemoryBuffer::pagesize);
	unsigned *global_adj_index = (unsigned *)global_adj_indexFile->get_address();

	for(unsigned i = 0; i < num; i ++) {
		tempEnt->insertID(i);
		for(unsigned j = 0; j < excpID[i].size(); j ++)
			tempEnt->insertID(excpID[i][j]);

		unsigned num = tempEnt->getSize() - 2;
		tempEnt->getBuffer()[1] = num; //update num
		unsigned size = tempEnt->getSize() * sizeof(ID);
		if (!firstTime && fileOff + size < (unsigned) (-1) / 8) {
			adjFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
			global_adj_index[2 * i] = fileOff;
			global_adj_index[2 * i + 1] = fileindex;
			fileOff += size;
		} else {
			if (fileindex >= 0) {
				adjFile[fileindex]->close();
			}

			fileindex++;
			fileOff = 0;
			adjFile.push_back(new TempFile(Dir + "/excpID_adj", fileindex));
			adjFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
			global_adj_index[2 * i] = 1;//actually is '0', but we set '1' here, for '0' has been used to mark the unexist 'to data'
			global_adj_index[2 * i + 1] = fileindex;
			fileOff += size;
			firstTime = false;

			cout << "fileIndex: " << fileindex << "i:" << i << endl;
		}
	}

	adjFile[fileindex]->close();
	global_adj_indexFile->flush();
	cout<<"success write excpID list"<<endl;

	for(int i = 0;i < adjFile.size();i++){
		delete adjFile[i];
		adjFile[i] = NULL;
	}
	adjFile.clear();
	delete global_adj_indexFile;
	delete tempEnt;
}

