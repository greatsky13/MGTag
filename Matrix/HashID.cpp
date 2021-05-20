/*
 * HashID.cpp
 *
 *  Created on: 2013-6-4
 *      Author: yaya
 */

#include "HashID.h"
#include "TempFile.h"
#include "OSFile.h"
#include "Sorter.h"
#include "TSorter.h"
#include <list>

//#define FLAT_ADJLIST 1

string HashID::Dir = "";
Degree *globalDegree;

HashID::HashID(unsigned maxID) {
	// TODO Auto-generated constructor stub
}

HashID::HashID(string dir) {
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

HashID::~HashID() {
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

void HashID::FIXLINE(char * s) {
	int len = (int) strlen(s) - 1;
	if (s[len] == '\n')
		s[len] = 0;
}

bool HashID::isNewIDExist(ID id, ID& newoff) {
	ID temp = *(oldID_mapto_newID + id);
	if (temp) {
		newoff = temp;
		return true;
	} else {
		return false;
	}
}

bool HashID::setNew(ID pos, ID oldID) {
	*(newID_mapto_oldID +pos) = oldID;
	 return true;
}

bool HashID::setOld(ID pos, ID newID) {
	*(oldID_mapto_newID + pos) = newID;
	return true;
}

void HashID::init(bool forward_or_backward) {
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

void HashID::init_scc() {
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

void HashID::init_DAG() {
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

void HashID::init_reverse_DAG() {
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

void HashID::init_excp() {
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

void HashID::init_newGraph() {
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

status HashID::convert_edge(string inputfile, unsigned lineCount) {
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

status HashID::convert_edge_to_reverse_adj(string inputfile, unsigned lineCount) {
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

status HashID::convert_adj(string inputfile, unsigned lineCount) {
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

status HashID::convert_adj_nonum(string inputfile, unsigned lineCount) {
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

status HashID::convert_adj_to_reverse_adj(string inputfile, unsigned lineCount) {
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

char* HashID::getOffset(ID id) {
	if (id > maxFromID || id < start_ID[0])return NULL;

	unsigned fileindex = global_adj_index[2 * id + 1];
	unsigned adjOff = global_adj_index[2 * id];
	if(adjOff == 0)return NULL; //'0' indicate unexist 'from or to data'
	if(adjOff == 1)adjOff = 0; //we modify the '1' back to '0' here
	char *temp = (adj[fileindex] + adjOff);
	return temp;
}

char* HashID::reverse_getOffset(ID id) {
	if (id > maxFromID || id < reverse_start_ID[0])return NULL;

	unsigned fileindex = reverse_global_adj_index[2 * id + 1];
	unsigned adjOff = reverse_global_adj_index[2 * id];
	if(adjOff == 0)return NULL; //'0' indicate unexist 'from or to data'
	if(adjOff == 1)adjOff = 0; //we modify the '1' back to '0' here
	char *temp = (reverse_adj[fileindex] + adjOff);
	return temp;
}

char* HashID::scc_getOffset(ID id){
	//if (id < scc_start_ID[0])return NULL;

	unsigned i = 0;
	for(; i < scc_num; i ++) {
		if(scc_start_ID[i] == id)
			break;
	}
	i ++;

	unsigned fileindex = global_scc_index[2 * i + 1];
	unsigned adjOff = global_scc_index[2 * i];
	if(adjOff == 0)return NULL; //'0' indicate unexist 'from or to data'
	if(adjOff == 1)adjOff = 0; //we modify the '1' back to '0' here
	char *temp = (scc[fileindex] + adjOff);
	return temp;
}

char* HashID::DAG_getOffset(ID id){
	if (id < DAG_start_ID[0])return NULL;

	unsigned fileindex = global_DAG_index[2 * id + 1];
	unsigned adjOff = global_DAG_index[2 * id];
	if(adjOff == 0)return NULL; //'0' indicate unexist 'from or to data'
	if(adjOff == 1)adjOff = 0; //we modify the '1' back to '0' here
	char *temp = (DAG[fileindex] + adjOff);
	return temp;
}

char* HashID::DAG_reverse_getOffset(ID id) {
	if (id < DAG_start_ID[0])return NULL;

	unsigned fileindex = DAG_reverse_global_adj_index[2 * id + 1];
	unsigned adjOff = DAG_reverse_global_adj_index[2 * id];
	if(adjOff == 0)return NULL; //'0' indicate unexist 'from or to data'
	if(adjOff == 1)adjOff = 0; //we modify the '1' back to '0' here
	char *temp = (DAG_reverse_adj[fileindex] + adjOff);
	return temp;
}

char* HashID::excp_getOffset(unsigned count) {
	if (count < 0 || count >= excp_idcount)return NULL;

	//count --;
	unsigned fileindex = global_exceptions_index[2 * count + 1];
	unsigned adjOff = global_exceptions_index[2 * count];
	if(adjOff == 0)return NULL; //'0' indicate unexist 'from or to data'
	if(adjOff == 1)adjOff = 0; //we modify the '1' back to '0' here
	char *temp = (exceptions[fileindex] + adjOff);
	return temp;
}

unsigned HashID::find_idcount(ID id) {
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

char* HashID::newGraph_getOffset(ID id) {
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

bool HashID::zeroDegree(ID id) {
	if (!degree[id].indeg && !degree[id].outdeg)
		return true;
	else
		return false;
}

bool HashID::both_in_and_out_vertex(ID id) {
	if (degree[id].indeg && degree[id].outdeg)
		return true;
	else
		return false;
}

void HashID::get_root_vertices(ID maxID) {
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

void HashID::sort_degree(unsigned fileindex) {
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

size_t HashID::loadFileinMemory(const char* filePath, char*& buf) {
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

void HashID::parallel_load_task(char *&buf, MemoryMappedFile *&temp,
		unsigned long cur_pos, size_t copy_size) {
	memcpy(buf + cur_pos, temp->getBegin() + cur_pos, copy_size);
}

void HashID::parallel_load_inmemory(const char* filePath, char*& buf) {
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
				boost::bind(&HashID::parallel_load_task, buf, temp, cur_pos, MEMCPY_SIZE));
		cur_pos += MEMCPY_SIZE;
	}
	if (left)
		CThreadPool::getInstance().AddTask(
				boost::bind(&HashID::parallel_load_task, buf, temp, cur_pos, left));
	CThreadPool::getInstance().Wait();

	temp->close();
	delete temp;
}

unsigned HashID::DFS_V(ID v) {
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

unsigned HashID::DFS_V(ID v, bool* &neighbors_visited, ID &newRootID) {
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

void HashID::DFS(bool forward_or_backward) {
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

ID HashID::convertToRaw(TempFile* rawFile, bool forward_or_backward) {
	if (degree) {
		free(degree);
		degree = NULL;
	}
	degree = (Degree*) calloc(maxID + 1, sizeof(Degree));

	for (unsigned fileindex = 0; fileindex < adjMap.size(); fileindex++) {
		char* begin = adjMap[fileindex]->get_address();
		char* limit = begin + adjMap[fileindex]->get_length();
		ID from = 0, to = 0;
		ID newFrom = 0, newTo = 0;
		unsigned count = 0;
		while (begin < limit) {
			from = *(ID*) begin;
			newFrom = oldID_mapto_newID[from];
			begin += sizeof(ID);
			count = *(unsigned*) begin;
			begin += sizeof(unsigned);
			if(forward_or_backward == true)degree[newFrom].outdeg += count;
			else degree[newFrom].indeg += count;

			for (unsigned i = 0; i < count; i++) {
				to = *(ID*) begin;
				newTo = oldID_mapto_newID[to];
				if(forward_or_backward == true)degree[newTo].indeg++;
				else degree[newTo].outdeg++;
				rawFile->writeId(newFrom);
				rawFile->writeId(newTo);
				begin += sizeof(ID);
			}
		}
	}

	rawFile->close();

	TempFile *degreeFile;
	if(forward_or_backward == true)degreeFile = new TempFile(Dir + "/degFile.forward", 0);
	else degreeFile = new TempFile(Dir + "/degFile.backward", 0);
	degreeFile->write(((size_t) maxID + 1) * sizeof(Degree),(const char*) degree);
	degreeFile->close();
	delete degreeFile;
	degreeFile = NULL;
	free(degree);
	degree = NULL;

	free(oldID_mapto_newID);
	oldID_mapto_newID = NULL;
	return maxID;
}

int compquick(const void *a, const void *b) {
	return *(int *) a - *(int *) b;
}

//------------------------------------------------------------------------------------

unsigned HashID::bfs_tree_builder(ID rootID, bool *&visited, hash_map<ID, size_t> &id_offset){
	queue<ID> Q;
	Q.push(rootID);
	ID tmp_id;
	ID* addr = NULL;
	unsigned count = 0, num = 0, unique_vertices_num = 0;
	EntityIDBuffer *buffer = new EntityIDBuffer();
	buffer->setIDCount(1);
	size_t offset = 0;
	TempFile *tree_file = new TempFile(Dir + "/tree", 0);

	while(!Q.empty()){
		tmp_id = Q.front();
		Q.pop();
		if(!visited[tmp_id]){
			visited[tmp_id] = true;
			unique_vertices_num++;
			buffer->empty();
			buffer->insertID(tmp_id);
			buffer->insertID(0);
			num = 0 ;
			addr = (ID*) getOffset(tmp_id);
			if (addr == NULL)continue;
			if(tmp_id != addr[0]){
				cout<<tmp_id<<" "<<addr[0]<<endl;
				continue;
			}
			//assert(tmp_id == *(addr));
			count = *(addr + 1);
			addr += 2; //skip the id and count

			for (int i = 0; i < count; i++) {
				ID temp = *(addr + i);
				if(!visited[temp]){
					buffer->insertID(temp);
					Q.push(temp);
					num++;
				}
			}

			buffer->getBuffer()[1] = num;
			unsigned size = buffer->getSize();
			tree_file->write(size * sizeof(ID), (const char*) buffer->getBuffer());
			id_offset.insert(pair<ID, size_t>(tmp_id, offset));
			offset += size;
		}
	}

	tree_file->flush();
	delete tree_file;
	delete buffer;
	return unique_vertices_num;
}

void HashID::dfs_forward_encode(ID rootID, bool *&visited, hash_map<ID, size_t> id_offset, ID &newRootID){
	stack<ID> S;
	S.push(rootID);
	ID tmp_id;
	MemoryMappedFile *tree_map = new MemoryMappedFile();
	assert(tree_map->open((Dir+"/tree.0").c_str()));
	ID *tree = (ID*) tree_map->getBegin();
	size_t offset = 0;
	unsigned count = 0, off = 0;

	while(!S.empty()){
		tmp_id = S.top();
		S.pop();
		if(!visited[tmp_id]){
			visited[tmp_id] = true;
			setOld(tmp_id, idcount);
			setNew(idcount, tmp_id);
			idcount++;
			offset = id_offset[tmp_id];
			if(tmp_id != tree[offset])continue;
			//assert(tmp_id == tree[offset]);
			count = tree[offset+1];
			if(count == 0)continue;
			offset += 2;
			for(int i = count - 1;i >= 0;i--){
				if(!visited[tree[offset+i]])S.push(tree[offset+i]);
			}
		}
	}

	newRootID = oldID_mapto_newID[rootID];
	tree_map->close();
	delete tree_map;
}

void HashID::dfs_backward_encode(ID rootID, bool *&visited, hash_map<ID, size_t> id_offset, ID &newRootID, unsigned unique_vertices_num){
	stack<ID> S;
	S.push(rootID);
	ID tmp_id;
	MemoryMappedFile *tree_map = new MemoryMappedFile();
	tree_map->open((Dir+"/tree.0").c_str());
	if(!(tree_map->getEnd() - tree_map->getBegin())){
		//zero size
		delete tree_map;
		return;
	}
	ID *tree = (ID*) tree_map->getBegin();
	size_t offset = 0;
	unsigned count = 0, off = 0;
	idcount += unique_vertices_num;
	ID cur_id = idcount;

	while(!S.empty()){
		tmp_id = S.top();
		S.pop();
		if(!visited[tmp_id]){
			visited[tmp_id] = true;
			setOld(tmp_id, cur_id);
			setNew(cur_id, tmp_id);
			cur_id--;
			offset = id_offset[tmp_id];
			if(tmp_id != tree[offset])continue;
			//assert(tmp_id == tree[offset]);
			count = tree[offset+1];
			if(count == 0)continue;
			offset += 2;
			for(int i = count - 1;i >= 0;i--){
				S.push(tree[offset+i]);
			}
		}
	}

	newRootID = oldID_mapto_newID[rootID];
	tree_map->close();
	delete tree_map;
}

//forward_or_backward == true: means forward, otherwise means backward
void HashID::encode(bool forward_or_backward){
	if(forward_or_backward == true)idcount = 1;//id from small to large
	else idcount = 0;//id from large to small
	newID_mapto_oldID = (ID *)calloc(maxID + 1, sizeof(ID));
	oldID_mapto_newID = (ID *)calloc(maxID + 1, sizeof(ID));

	if(forward_or_backward == true)get_root_vertices(maxID);
	MemoryMappedFile *rootMap = new MemoryMappedFile();
	if(forward_or_backward == true)assert(rootMap->open((Dir+"/root.forward.0").c_str()));
	else assert(rootMap->open((Dir+"/root.backward.0").c_str()));
	unsigned rootNum = (rootMap->getEnd() - rootMap->getBegin()) / sizeof(ID);
	ID *root = (ID*) rootMap->getBegin();
	if (rootNum > 0)cout << "rootNum: " << rootNum << endl;

	unsigned off = 0;
	ID newRootID = 0;
	TempFile *newRoot;
	if(forward_or_backward == true)newRoot= new TempFile(Dir + "/new_root.forward", 0);
	else newRoot= new TempFile(Dir + "/new_root.backward", 0);

	bool *visited1 = (bool *) calloc((maxID + 1), sizeof(bool));
	bool *visited2 = (bool *) calloc((maxID + 1), sizeof(bool));
	hash_map<ID, size_t> id_offset;

	//encode the root vertices based subgraph
	if(forward_or_backward == true){
		//encode so (i.e., forward tree), start vertex is a forward_root_vertex
		unsigned unique_vertices_num = 0;
		for (int i = 0; i < rootNum; i++) {
			unique_vertices_num = bfs_tree_builder(root[i], visited1, id_offset);
			if(!unique_vertices_num)continue;
			dfs_forward_encode(root[i], visited2, id_offset, newRootID);
			newRoot->writeId(newRootID);
			hash_map<ID, size_t>().swap(id_offset);
		}
	}else{
		//encode os (i.e., backward tree), start vertex is a backward_root_vertex
		unsigned unique_vertices_num = 0;
		for (int i = 0; i < rootNum; i++) {
			unique_vertices_num = bfs_tree_builder(root[i], visited1, id_offset);
			if(!unique_vertices_num)continue;
			dfs_backward_encode(root[i], visited2, id_offset, newRootID, unique_vertices_num);
			newRoot->writeId(newRootID);
			hash_map<ID, size_t>().swap(id_offset);
		}
	}
	rootMap->close();
	delete rootMap;
	cout << "in root vertices tree based encode, idcount: " << idcount << ", maxID: " << maxID << endl;

	if (idcount < maxID) {
		//encode the loop back subgraph, which means don't have root vertices
		assert(degree);
		size_t isolated_vertices = 0;
		unsigned unique_vertices_num = 0;
		if(forward_or_backward == true){
			//forward encode
			for(ID i = start_ID[0]; i <= maxID; i++) {
				if (!isNewIDExist(i, off) && both_in_and_out_vertex(i)) {
					//encode so, loop back tree based, start vertex is a vertex in the circle
					unique_vertices_num = bfs_tree_builder(i, visited1, id_offset);
					if(!unique_vertices_num)continue;
					dfs_forward_encode(i, visited2, id_offset, newRootID);
					newRoot->writeId(newRootID);
					hash_map<ID, size_t>().swap(id_offset);
				}else if(zeroDegree(i)){
					isolated_vertices++;
				}
			}
		}else{
			//backward encode
			for(ID i = start_ID[0]; i <= maxID; i++) {
				if (!isNewIDExist(i, off) && both_in_and_out_vertex(i)) {
					//encode os, loop back tree based, start vertex is a vertex in the circle
					unique_vertices_num = bfs_tree_builder(i, visited1, id_offset);
					if(!unique_vertices_num)continue;
					dfs_backward_encode(i, visited2, id_offset, newRootID, unique_vertices_num);
					newRoot->writeId(newRootID);
					hash_map<ID, size_t>().swap(id_offset);
				}else if(zeroDegree(i)){
					isolated_vertices++;
				}
			}
		}
		cout<<"in loop back tree based, idcount: "<<idcount<<", maxID: "<<maxID
				<<", isolated_vertices: "<<isolated_vertices<<endl;
	}

	if(OSFile::FileExists((Dir+"/tree.0").c_str()))OSFile::FileDelete((Dir+"/tree.0").c_str());
	free(visited1);
	free(visited2);

	if (idcount < maxID) {
		//vertices that haven't encode yet
		for(ID i = start_ID[0]; i <= maxID; i++){
			if (!isNewIDExist(i, off) && !zeroDegree(i)){
				setOld(i, idcount);
				setNew(idcount, i);
				newRoot->writeId(idcount);
				idcount++;
			}
		}
		cout<<"in total, idcount: "<<idcount<<", maxID: "<<maxID<<endl;
	}

	newRoot->flush();
	delete newRoot;
	save_encoded_ids(forward_or_backward);
}

void HashID::save_encoded_ids(bool forward_or_backward){
	TempFile *old_mapto_new, *new_mapto_old;
	if(forward_or_backward == true){
		remove((Dir+"/root.forward.0").c_str());
		old_mapto_new = new TempFile(Dir + "/old_mapto_new.forward", 0);
		new_mapto_old = new TempFile(Dir + "/new_mapto_old.forward", 0);
	}
	else{
		remove((Dir+"/root.backward.0").c_str());
		maxID = idcount;
		old_mapto_new = new TempFile(Dir + "/old_mapto_new.backward", 0);
		new_mapto_old = new TempFile(Dir + "/new_mapto_old.backward", 0);
	}

	old_mapto_new->write((maxID + 1) * sizeof(ID), (const char*) oldID_mapto_newID);
	old_mapto_new->close();
	delete old_mapto_new;

	new_mapto_old->write((maxID + 1) * sizeof(ID), (const char*) newID_mapto_oldID);
	new_mapto_old->close();
	delete new_mapto_old;

	free(newID_mapto_oldID);
	newID_mapto_oldID = NULL;
	cout << "end write file old_mapto_new and new_mapto_old, size: " << (maxID + 1) * sizeof(ID) << endl;
}

//--------------------------------------------following is topology encode----------------------------------------------------
void HashID::get_forward_root_vertices(ID maxID) {
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

ID *HashID::mark_cross_vertices(string path, ID *root_arr, unsigned rootNum, bool *&visited_arr, bool *&cross_vertices_arr, unsigned &cross_idcount){
	deque<ID>* que = new deque<ID>();
	deque<ID>* que_cross = new deque<ID>();
	ID curID, component;
	unsigned count;
	
	//unsigned idcount = 0;

	string fileName = string(path+"/tmpvalue");
	MMapBuffer *tmpvalue = new MMapBuffer(fileName.c_str(), maxID * sizeof(ID));
	ID *tmpvalue_arr = (ID *)(tmpvalue->get_address());
	
	for(size_t index = 0;index < rootNum;index++){
		if(visited_arr[root_arr[index]])continue;
		visited_arr[root_arr[index]] = true;
		que->push_back(root_arr[index]);
		component = root_arr[index];
		tmpvalue_arr[root_arr[index]] = component;
		newGraph_idcount ++;

		//idcount ++;

		while(!que->empty()){
			curID = que->front();
			que->pop_front();
			
			ID *addr = (ID*) DAG_getOffset(curID);
			if (addr == NULL)continue;
			assert(curID == addr[0]);
			count = addr[1];
			addr += 2; //skip the id and count
			
			for (size_t i = 0; i < count; i++) {
				if(visited_arr[addr[i]]){
					if(tmpvalue_arr[addr[i]] != component){
						if(!cross_vertices_arr[addr[i]]) {
//							ID *addr_addr = (ID *) DAG_getOffset(addr[i]);
//							if(addr_addr[1] != 0) {
//								cross_vertices_arr[addr[i]] = true;
//								que_cross->push_back(addr[i]);
//								//cross_count ++;
//								cross_idcount ++;
//								//cout<<"crossid:"<<addr[i]<<endl;
//								newGraph_idcount ++;
//							}
							cross_vertices_arr[addr[i]] = true;
							que_cross->push_back(addr[i]);
							//cross_count ++;
							cross_idcount ++;
							//cout<<"crossid:"<<addr[i]<<endl;
							newGraph_idcount ++;
						}
					}
					continue;
				}
				visited_arr[addr[i]] = true;
				que->push_back(addr[i]);
				tmpvalue_arr[addr[i]] = component;

				//idcount ++;
			}
		}
	}
	
	ID *cross_id = (ID *)calloc(cross_idcount, sizeof(ID));
	for(unsigned i = 0; i < cross_idcount; i ++) {
		cross_id[i] = que_cross->front();
		que_cross->pop_front();
	}

	//cout<<"cross_count："<<cross_count<<endl;
	remove(fileName.c_str());
	delete tmpvalue;
	delete que, que_cross;
	return cross_id;
}
	
void HashID::deal_with_cross_vertex(bool *&visited_arr, bool *cross_vertices_arr, ID crossid, TempFile &excp_id){
	deque<ID>* que = new deque<ID>();
	ID curID;
	unsigned count;

	visited_arr[crossid] = true;
	que->push_front(crossid);
	EntityIDBuffer* tempEnt = new EntityIDBuffer();
	EntityIDBuffer* NG_tempEnt = new EntityIDBuffer();
	
//	NG_tempEnt->empty();
//	NG_tempEnt->insertID(idcount);
//	NG_tempEnt->insertID(0);

	//newGraph_id[newGraph_idcount] = idcount;
	//newGraph_idcount ++;

	//cout<<"cross:"<<crossid<<" "<<newGraph_idcount<<endl;
	while(!que->empty()){
		curID = que->front();
		que->pop_front();
		DAG_maxID = std::max(curID, DAG_maxID);
		
		/////啊
		visited_arr[curID] = true;

		oldID_mapto_newID[curID] = idcount;
		newID_mapto_oldID[idcount] = curID;
		idcount++;
		/////啊

		ID *addr = (ID*) DAG_getOffset(curID);
		if (addr == NULL){
			cout<<"curID:"<<curID<<" NULL"<<endl;
			continue;
		}
		if(curID != addr[0]) cout<<"curID:"<<curID<<" addr[0]"<<addr[0]<<endl;
		assert(curID == addr[0]);
		count = addr[1];
		addr += 2; //skip the id and count

//		tempEnt->empty();
//		tempEnt->insertID(oldID_mapto_newID[curID]);
//		tempEnt->insertID(0);

		for (size_t i = 0; i < count; i++) {
			DAG_maxID = std::max(addr[i], DAG_maxID);
			if(degree[addr[i]].indeg)degree[addr[i]].indeg--;
			if(cross_vertices_arr[addr[i]]){
				//tempEnt->insertID(addr[i]);
				//NG_tempEnt->insert_uniqueID(addr[i]);
				//new_graph.writeId(crossid);
				//new_graph.writeId(addr[i]);
				continue;
			}
			if(visited_arr[addr[i]])continue;
			if(degree[addr[i]].indeg){
				//tempEnt->insertID(addr[i]);
				excp_count ++;
				excp_id.writeId(oldID_mapto_newID[curID]);
				excp_id.writeId(addr[i]);
				continue;
			}
				
			que->push_front(addr[i]);
		}

		//存异常
//		unsigned num = tempEnt->getSize() - 2;
//		tempEnt->getBuffer()[1] = num; //update num
//		unsigned size = tempEnt->getSize() * sizeof(ID);
//		if (fileOff + size < (unsigned) (-1) / 8) {
//			excpFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
//			global_excp_index[2 * oldID_mapto_newID[curID]] = fileOff;
//			global_excp_index[2 * oldID_mapto_newID[curID] + 1] = fileindex;
//			fileOff += size;
//		} else {
//			if (fileindex >= 0) {
//				excpFile[fileindex]->close();
//			}
//
//			fileindex++;
//			fileOff = 0;
//			excpFile.push_back(new TempFile(Dir + "/exceptions", fileindex));
//			excp_start_ID[fileindex] = oldID_mapto_newID[curID];
//			excpFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
//			global_excp_index[2 * oldID_mapto_newID[curID]] = 1;//actually is '0', but we set '1' here, for '0' has been used to mark the unexist 'to data'
//			global_excp_index[2 * oldID_mapto_newID[curID] + 1] = fileindex;
//			fileOff += size;
//		}
	}

	//存newGraph
//	unsigned NG_num = NG_tempEnt->getSize() - 2;
//	NG_tempEnt->getBuffer()[1] = NG_num; //update num
//	unsigned NG_size = NG_tempEnt->getSize() * sizeof(ID);
//	if (NG_fileOff + NG_size < (unsigned) (-1) / 8) {
//		NG_File[NG_fileindex]->write(NG_size,(const char*) NG_tempEnt->getBuffer());
//		global_NG_index[2 * newGraph_idcount] = NG_fileOff;
//		global_NG_index[2 * newGraph_idcount + 1] = NG_fileindex;
//		NG_fileOff += NG_size;
//	} else {
//		if (NG_fileindex >= 0) {
//			NG_File[NG_fileindex]->close();
//		}
//
//		NG_fileindex++;
//		NG_fileOff = 0;
//		NG_File.push_back(new TempFile(Dir + "/newGraph", NG_fileindex));
//		//newGraph_start_ID[NG_fileindex] = oldID_mapto_newID[crossid];
//		NG_File[NG_fileindex]->write(NG_size,(const char*) NG_tempEnt->getBuffer());
//		global_NG_index[2 * newGraph_idcount] = 1;//actually is '0', but we set '1' here, for '0' has been used to mark the unexist 'to data'
//		global_NG_index[2 * newGraph_idcount + 1] = NG_fileindex;
//		NG_fileOff += NG_size;
//	}
	//newGraph_idcount ++;

	delete que;
}

void HashID::cross_collector(ID *cross_id, unsigned cross_idcount, bool *cross_vertices_arr, bool *&visited_arr, TempFile &excp_id, TempFile &new_graph_root) {
//	ID last_crossid, crossid, last_rootid, rootid=0;
//	MemoryMappedFile mappedIn;
//	assert(mappedIn.open(sorted_cross_distribution.getFile().c_str()));
//	const char* reader = mappedIn.getBegin(), *limit = mappedIn.getEnd();
//	unsigned crossid_count = 0;
//
//	TripleBitBuilder::loadData(reader, crossid);
//	last_crossid = crossid, last_rootid = rootid;
//	reader = TempFile::skipId(reader);
//	cout<<"crossid:"<<crossid<<" ";
//	deal_with_cross_vertex(visited_arr, cross_vertices_arr, crossid, fileindex, fileOff, excpFile, global_excp_index, NG_fileindex, NG_fileOff, NG_File, global_NG_index);
//	crossid_count++;
//
//	while (reader < limit) {
//		TripleBitBuilder::loadData(reader, crossid);
//		if(crossid == last_crossid) {
//			reader = TempFile::skipId(reader);
//			continue;
//		}
//
//		if ( crossid != last_crossid ) {
//			cout<<"crossid:"<<crossid<<" ";
//			deal_with_cross_vertex(visited_arr, cross_vertices_arr, crossid, fileindex, fileOff, excpFile, global_excp_index, NG_fileindex, NG_fileOff, NG_File, global_NG_index);
//			crossid_count++;
//			last_crossid = crossid;
//
//		}
//
//		reader = TempFile::skipId(reader);
//	}
//
//	mappedIn.close();
//	cout<<"the number of crossid is: "<<crossid_count<<endl;

	ID last_crossid = cross_id[0];
	//ID crossid = cross_id[0];
	unsigned count = 0;
	new_graph_root.writeId(cross_id[0]);
	deal_with_cross_vertex(visited_arr, cross_vertices_arr, cross_id[0], excp_id);
	count ++;
	for(unsigned i = 1; i < cross_idcount; i ++) {
		//crossid = cross_id[i];
		if(cross_id[i] != last_crossid) {
			new_graph_root.writeId(cross_id[i]);
			deal_with_cross_vertex(visited_arr, cross_vertices_arr, cross_id[i], excp_id);
			count ++;
			last_crossid = cross_id[i];
		}
	}
	cout<<"the number of cross id is:"<<count<<endl;
}

void HashID::construct_range_graph(TempFile &sorted_range, TempFile &range_graph, size_t &adjsize){
	ID last_rootid, rootid;
	unsigned last_startid, startid, last_endid, endid;
	vector<unsigned> subtree_data;
	size_t range_size = 0;
	
	MemoryMappedFile mappedIn;
	assert(mappedIn.open(sorted_range.getFile().c_str()));
	const char* reader = mappedIn.getBegin(), *limit = mappedIn.getEnd();

	TripleBitBuilder::loadTriple(reader, rootid, startid, endid);
	subtree_data.push_back(startid);
	subtree_data.push_back(endid);
	
	last_rootid = rootid, last_startid = startid, last_endid = endid;
	reader = TripleBitBuilder::skipIdIdId(reader);

	while (reader < limit) {
		TripleBitBuilder::loadTriple(reader, rootid, startid, endid);
		if(rootid == last_rootid && startid == last_startid && endid == last_endid){
			reader = TripleBitBuilder::skipIdIdId(reader);
			continue;
		}
		
		if(rootid == last_rootid){
			subtree_data.push_back(startid);
			subtree_data.push_back(endid);
			last_startid = startid, last_endid = endid;
			reader = TripleBitBuilder::skipIdIdId(reader);
			continue;
		}
		
		range_size = subtree_data.size() / 2;
		for(size_t i = 0;i < range_size;i++){
			range_graph.writeId(subtree_data[2*i]);
			range_graph.writeId(subtree_data[2*i]);
			range_graph.writeId(subtree_data[2*i+1]);
			for(size_t j = 0;j < range_size;j++){
				if(j != i){
					range_graph.writeId(subtree_data[2*i]);
					range_graph.writeId(subtree_data[2*j]);
					range_graph.writeId(subtree_data[2*j+1]);
				}
			}
		}
		
		adjsize += range_size;
		subtree_data.clear();
		subtree_data.push_back(startid);
		subtree_data.push_back(endid);
		last_rootid = rootid, last_startid = startid, last_endid = endid;
		reader = TripleBitBuilder::skipIdIdId(reader);
	}
	
	range_size = subtree_data.size() / 2;
	for(size_t i = 0;i < range_size;i++){
		range_graph.writeId(subtree_data[2*i]);
		range_graph.writeId(subtree_data[2*i]);
		range_graph.writeId(subtree_data[2*i+1]);
		for(size_t j = 0;j < range_size;j++){
			if(j != i){
				range_graph.writeId(subtree_data[2*i]);
				range_graph.writeId(subtree_data[2*j]);
				range_graph.writeId(subtree_data[2*j+1]);
			}
		}
	}
	
	adjsize += range_size;
	range_graph.flush();
	mappedIn.close();
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

void HashID::convert_range_graph_into_adjlist(string path, TempFile &sorted_range_graph, size_t adjsize){
	startid_offset *offset_arr = (startid_offset *)calloc(adjsize+1, sizeof(startid_offset));
	size_t offset_ptr = 0;
	
	ID last_srcid, srcid;
	unsigned last_startid, startid, last_endid, endid;
	char *tmpbuf = (char *)malloc(TMP_BUF_SIZE*sizeof(char));
	size_t tmpbuf_index = 0;
	bool flag = true;
	
	TempFile *adjfile = new TempFile(path+"/range_adjlist", 0);
	size_t offset = 0;
	FILE *flat_adjlist = fopen((path+"/flat_adjlist").c_str(), "w");
	FILE *vertex_pair = fopen((path+"/vertex_pair").c_str(), "w");
	
	startid_offset_index *offset_index_arr = (startid_offset_index *)calloc(adjsize/RANGE_BINARY_SIZE+2, sizeof(startid_offset_index));
	unsigned offset_index_ptr = 0;
	
	vector<unsigned> tmp_range;
	
	MemoryMappedFile mappedIn;
	assert(mappedIn.open(sorted_range_graph.getFile().c_str()));
	const char *reader = mappedIn.getBegin(), *limit = mappedIn.getEnd();

	TripleBitBuilder::loadTriple(reader, srcid, startid, endid);
	offset_arr[offset_ptr].startid = srcid;
	offset_arr[offset_ptr].offset = offset;
	offset_index_arr[offset_index_ptr].startid = srcid;
	offset_index_arr[offset_index_ptr].len = offset_ptr;
	offset_ptr++;
	offset_index_ptr++;
	assert(startid<=endid);
	tmp_range.push_back(startid);
	tmp_range.push_back(endid);
	
	last_srcid = srcid, last_startid = startid, last_endid = endid;
	reader = TripleBitBuilder::skipIdIdId(reader);

	while (reader < limit) {
		TripleBitBuilder::loadTriple(reader, srcid, startid, endid);
		if(srcid == last_srcid && startid == last_startid && endid == last_endid){
			reader = TripleBitBuilder::skipIdIdId(reader);
			continue;
		}
		
		if(srcid == last_srcid){
			assert(startid <= endid);
			tmp_range.push_back(startid);
			tmp_range.push_back(endid);
			last_startid = startid, last_endid = endid;
			reader = TripleBitBuilder::skipIdIdId(reader);
			continue;
		}
		
		batch_write(flat_adjlist, vertex_pair, adjfile, tmp_range, last_srcid, tmpbuf, tmpbuf_index, flag, offset);
		
		offset_arr[offset_ptr].startid = srcid;
		offset_arr[offset_ptr].offset = offset;
		offset_ptr++;
		assert(startid <= endid);
		tmp_range.clear();
		tmp_range.push_back(startid);
		tmp_range.push_back(endid);
		
		if(!(offset_ptr % RANGE_BINARY_SIZE)){
			offset_index_arr[offset_index_ptr].startid = srcid;
			offset_index_arr[offset_index_ptr].len = offset_ptr;
			offset_index_ptr++;
		}
		
		last_srcid = srcid, last_startid = startid, last_endid = endid;
		reader = TripleBitBuilder::skipIdIdId(reader);
	}
	mappedIn.close();
	
	batch_write(flat_adjlist, vertex_pair, adjfile, tmp_range, last_srcid, tmpbuf, tmpbuf_index, flag, offset);
	
	offset_arr[offset_ptr].startid = endid+1;
	offset_arr[offset_ptr].offset = offset;
	offset_ptr++;
	offset_index_arr[offset_index_ptr].startid = endid+1;
	offset_index_arr[offset_index_ptr].len = offset_ptr;
	offset_index_ptr++;
	
	free(tmpbuf);
	final_flush(adjfile, tmpbuf, tmpbuf_index);
	delete adjfile;
	fclose(flat_adjlist);
	fclose(vertex_pair);
	
	TempFile *adjoffset = new TempFile(path+"/range_adjoffset", 0);
	adjoffset->write(offset_ptr*sizeof(startid_offset), (const char*)offset_arr);
	adjoffset->flush();
	free(offset_arr);
	delete adjoffset;
	
	TempFile *adjoffset_index = new TempFile(path+"/range_adjoffset_index", 0);
	adjoffset_index->write(offset_index_ptr*sizeof(startid_offset_index), (const char*)offset_index_arr);
	adjoffset_index->flush();
	free(offset_index_arr);
	delete adjoffset_index;
}

///啊
//struct Abnormal *HashID::find_abnormal(ID id){
//	struct Abnormal *abn = first_abnormal;
//	while(abn != NULL){
//		if(abn->parent == id)
//			return abn;
//		abn = abn->next;
//	}
//	return abn;
//}

bool HashID::find_equal(ID root_id, ID child_id, bool *cross_vertices_arr){
	//ID newID = oldID_mapto_newID[root_id];
	if(cross_vertices_arr[newID_mapto_oldID[child_id]])
		return true;

	struct Abnormal *abn = first_abnormal;
	while(abn != NULL){
		if(abn->parent == root_id)
			break;
		abn = abn->next;
	}

	//bool tag = false;
	if(abn == NULL) {
		return false;
	}
	else{
		struct Child *ch = abn->first_child;
		while(ch != NULL){
			if(ch->id == child_id)
				return true;
			ch = ch->next;
		}
	}

	//free(ch);
	//ch = NULL;
	return false;
}

bool HashID::is_exceptions(ID id1, ID id2) {
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
ID HashID::posttranv_tree(ID id, unsigned count, bool *cross_vertices_arr){
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

void HashID::mark_vertices_tag(TempFile &new_graph_root, bool *cross_vertices_arr){
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

void HashID::abnormal_mapto_newID(struct Abnormal *abn){
	struct Child *ch;
	while(abn !=NULL){
		ch = abn->first_child;
		while(ch != NULL){
			ID temp = ch->id;
			ch->id = oldID_mapto_newID[temp];
			ch = ch->next;
		}
		abn = abn->next;
	}
}

void HashID::subtree_collector(string path, ID *root_arr, unsigned rootNum, bool *&visited_arr, bool *cross_vertices_arr){
	ID idcount = 1;
	newID_mapto_oldID = (ID *)calloc(maxID + 2, sizeof(ID));
	oldID_mapto_newID = (ID *)calloc(maxID + 1, sizeof(ID));

	/////啊
	//ID maxEdge = 6636600779;
	idtag = (ID *)calloc(maxID + 2, sizeof(ID));

	struct Abnormal *abnormal, *last_abn;
	first_abnormal = abnormal = (struct Abnormal *)calloc(1, sizeof(struct Abnormal));
	struct Child *ab_ch, *last_ab_ch;
	ab_ch = abnormal->first_child = (struct Child *)calloc(1, sizeof(struct Child));
	abnormal->child_num = 0;
	abnormal_count = 0;

	TempFile new_graph(path+"/new_graph");
	/////啊

	TempFile range(path+"/range");
	TempFile cross_distribution(path+"/cross_distribution");
	
	deque<ID>* que = new deque<ID>();
	ID curID;
	unsigned startid, endid, count;
	
	for(size_t index = 0;index < rootNum;index++){
		startid = idcount;
		if(visited_arr[root_arr[index]])continue;
		//visited_arr[root_arr[index]] = true;
		que->push_front(root_arr[index]);
		///啊
		//oldID_mapto_newID[root_arr[index]] = idcount;
		//newID_mapto_oldID[idcount] = root_arr[index];
		//idcount++;

		cout<<root_arr[index]<<" ";
		new_graph.writeId(idcount);
		/////啊

		while(!que->empty()){
			curID = que->front();
			que->pop_front();
			
			///啊
			visited_arr[curID] = true;
			oldID_mapto_newID[curID] = idcount;
			newID_mapto_oldID[idcount] = curID;
			idcount++;
			/////啊

			ID *addr = (ID*) getOffset(curID);
			if (addr == NULL)continue;
			assert(curID == addr[0]);
			count = addr[1];
			addr += 2; //skip the id and count
			

			if(curID == root_arr[index])
				cout<<"邻接点个数："<<count<<endl;

			//////啊

			for(size_t i = 0; i < count; i ++){
				cout<<addr[i]<<" ";
			}
			cout<<endl;

			bool tag = false;
			/////

			for (size_t i = 0; i < count; i++) {
				//if(bool *is_scc_vertices_arr[addr[i]])find_cycles(addr[i], visited_arr  );
				if(degree[addr[i]].indeg)degree[addr[i]].indeg--;
				if(cross_vertices_arr[addr[i]]){
					cross_distribution.writeId(addr[i]);
					cross_distribution.writeId(oldID_mapto_newID[root_arr[index]]);

					/////啊
					//new_graph.writeId(addr[i]);
//					abnormal->parent = oldID_mapto_newID[curID];
//					if(abnormal->child_num == 0){
//						abnormal->first_child->id = addr[i];
//						ab_ch = (struct Child *)calloc(1, sizeof(struct Child));
//						abnormal->first_child->next = ab_ch;
//
//						last_ab_ch = abnormal->first_child;
//						//ab_ch = next_ab_ch;
//						abnormal->child_num ++;
//					}
//					else{
//
//					//cout<<abnormal->parent<<" :"<<abnormal->children->id<<endl;
//						ab_ch->id = addr[i];
//						struct Child *next_ab_ch = (struct Child *)calloc(1, sizeof(struct Child));
//						ab_ch->next = next_ab_ch;
//						last_ab_ch = ab_ch;
//						ab_ch = next_ab_ch;
//						abnormal->child_num ++;
//					}
//
//					tag = true;

					/////啊

					//cout<<"cross here "<<addr[i]<<endl;
					continue;
				}
				if(visited_arr[addr[i]])continue;
				if(degree[addr[i]].indeg){
					//周双
					abnormal->parent = oldID_mapto_newID[curID];
					if(abnormal->child_num == 0){
						abnormal->first_child->id = addr[i];
						ab_ch = (struct Child *)calloc(1, sizeof(struct Child));
						abnormal->first_child->next = ab_ch;

						last_ab_ch = abnormal->first_child;
						//ab_ch = next_ab_ch;
						abnormal->child_num ++;
					}
					else{

					//cout<<abnormal->parent<<" :"<<abnormal->children->id<<endl;
						ab_ch->id = addr[i];
						struct Child *next_ab_ch = (struct Child *)calloc(1, sizeof(struct Child));
						ab_ch->next = next_ab_ch;
						last_ab_ch = ab_ch;
						ab_ch = next_ab_ch;
						abnormal->child_num ++;
					}

					tag = true;
					//周双
					cout<<"degree here "<<addr[i]<<endl;
					continue;
				}
				////啊
				//visited_arr[addr[i]] = true;
				que->push_front(addr[i]);
				//oldID_mapto_newID[addr[i]] = idcount;
				//newID_mapto_oldID[idcount] = addr[i];
				//idcount++;
				/////啊
			}
			if(tag == true){
				last_ab_ch->next = NULL;
				ab_ch = NULL;
				struct Abnormal *nextAbn = (struct Abnormal *)calloc(1, sizeof(struct Abnormal));
				abnormal->next = nextAbn;
				last_abn = abnormal;
				abnormal = nextAbn;
				abnormal->first_child = (struct Child *)calloc(1, sizeof(struct Child));
				abnormal->child_num = 0;

				abnormal_count ++;
			}
		}

		endid = idcount - 1;
		
		range.writeId(oldID_mapto_newID[root_arr[index]]);
		range.writeId(startid);
		range.writeId(endid);

	}
	
	for(size_t index = 1;index < idcount;index++){

		cout<<"oldID:"<<newID_mapto_oldID[index]<<" newID:"<<oldID_mapto_newID[newID_mapto_oldID[index]]<<endl;
	}

	cross_distribution.flush();
	range.flush();
	new_graph.flush();
	delete que;
	
	TempFile sorted_cross_distribution(path+"/sorted_cross_distribution");
	Sorter::sort(cross_distribution, sorted_cross_distribution, TempFile::skipIdId, TempFile::compare12);
	//cross_collector(sorted_cross_distribution, range, new_graph, idcount, cross_vertices_arr, visited_arr, abnormal, last_abn);
	range.flush();
	new_graph.flush();
	cout<<"in subtree_collector(), idcount = "<<idcount<<endl;
	
	for(size_t index = 1;index < idcount;index++){

		cout<<"oldID:"<<newID_mapto_oldID[index]<<" newID:"<<oldID_mapto_newID[newID_mapto_oldID[index]]<<endl;
	}

	cout<<endl;
	/////以下注意是孤立点，范围的存取
	ID newoff;
	for(ID id = 1; id <= maxID; id++){
		if (!isNewIDExist(id, newoff)){
			oldID_mapto_newID[id] = idcount;
			newID_mapto_oldID[idcount] = id;
			new_graph.writeId(idcount);
			idcount++;
		}
	}


	new_graph.flush();
	///////啊
//	for(size_t index = 1;index < 30;index++){
//
//		cout<<"oldID:"<<newID_mapto_oldID[index]<<" newID:"<<oldID_mapto_newID[newID_mapto_oldID[index]]<<endl;
//	}
	/////啊
	last_abn->next = NULL;
	abnormal = NULL;

	abnormal_mapto_newID(first_abnormal);

	abnormal = first_abnormal;
	while(abnormal != NULL){
		cout<<abnormal->parent<<"异常：";
		struct Child *ch = abnormal->first_child;
		while(ch != NULL){
			cout<<ch->id<<" ";
			ch = ch->next;
		}
		cout<<endl;
		abnormal = abnormal->next;
	}

	for(size_t i = 0; i < 6; i ++){
		cout<<"oldID:"<<i<<" newID:"<<oldID_mapto_newID[i]<<endl;
	}

	//mark_vertices_tag(new_graph, cross_vertices_arr);

	/////、啊
	for(size_t i = 1; i <= maxID + 1; i ++){
		if(i % 5 == 0)
			cout<<endl;
		cout<<i<<"标签："<<idtag[i]<<" ";
	}
	/////啊
	cout<<endl;

	cout<<"in subtree_collector(), after traverse, idcount = "<<idcount<<endl;
	
	TempFile *old_mapto_new = new TempFile(path + "/old_mapto_new", 0);
	TempFile *new_mapto_old = new TempFile(path + "/new_mapto_old", 0);
	old_mapto_new->write((maxID + 1) * sizeof(ID), (const char*) oldID_mapto_newID);
	old_mapto_new->close();
	delete old_mapto_new;
	new_mapto_old->write((maxID + 1) * sizeof(ID), (const char*) newID_mapto_oldID);
	new_mapto_old->close();
	delete new_mapto_old;
	free(oldID_mapto_newID);
	oldID_mapto_newID = NULL;
	free(newID_mapto_oldID);
	newID_mapto_oldID = NULL;
	
	/////啊
	TempFile *id_tag = new TempFile(path + "/id_tag",0);
	TempFile *abnormal_id = new TempFile(path + "/abnormal_id",0);
	cout<<"write idtag!"<<endl;
	id_tag->write((maxID + 1) * sizeof(ID), (const char*)idtag);
	cout<<"write idtag over!"<<endl;
	//abnormal_id->write((maxID + 1) * sizeof(Abnormal), (const char*)abnormal);
	//cout<<"write abnormal!"<<endl;
	//////a

	TempFile sorted_range(path+"/sorted_range");
	TSorter::sort(range, sorted_range, TripleBitBuilder::skipIdIdId, TripleBitBuilder::compare123);
	TempFile range_graph(path+"/range_graph");
	size_t adjsize = 0;
	construct_range_graph(sorted_range, range_graph, adjsize);
	
	TempFile sorted_range_graph(path+"/sorted_range_graph");
	TSorter::sort(range_graph, sorted_range_graph, TripleBitBuilder::skipIdIdId, TripleBitBuilder::compare123);
	convert_range_graph_into_adjlist(path, sorted_range_graph, adjsize);
	
	remove(range.getFile().c_str());
	remove(sorted_range.getFile().c_str());
	remove(cross_distribution.getFile().c_str());
	remove(sorted_cross_distribution.getFile().c_str());
	remove(range_graph.getFile().c_str());
	remove(sorted_range_graph.getFile().c_str());
}

void HashID::topology_encode(string path){
	get_forward_root_vertices(maxID);
	
	MemoryMappedFile *rootFile = new MemoryMappedFile();
	assert(rootFile->open((string(path)+"/root.forward.0").c_str()));
	unsigned rootNum = (rootFile->getEnd() - rootFile->getBegin()) / sizeof(ID);
	cout<<"tree number: "<<rootNum<<endl;
	ID *root_arr = (ID*) rootFile->getBegin();
	
	string fileName1 = string(path+"/cross_vertices");
	MMapBuffer *cross_vertices = new MMapBuffer(fileName1.c_str(), (maxID+1) * sizeof(bool));
	bool *cross_vertices_arr = (bool *)(cross_vertices->get_address());
	memset(cross_vertices_arr, false, (maxID+1));
	
	string fileName2 = string(path+"/visited");
	MMapBuffer *visited = new MMapBuffer(fileName2.c_str(), (maxID+1) * sizeof(bool));
	bool *visited_arr = (bool *)(visited->get_address());
	memset(visited_arr, false, (maxID+1));
	
	//mark_cross_vertices(path, root_arr, rootNum, visited_arr, cross_vertices_arr);
	
	cout<<"make_cross_vertices end"<<endl;

	//for(size_t i = 0; i < rootNum; i ++)
		//cout<<root_arr[i];

	memset(visited_arr, false, (maxID+1));
	//subtree_collector(path, root_arr, rootNum, visited_arr, cross_vertices_arr);
	
	rootFile->close();
	remove(fileName1.c_str());
	remove(fileName2.c_str());
	delete rootFile;
	delete cross_vertices;
	delete visited;
}

//----------------------------Following is the NL code by 双双-----------------------------------------------
void HashID::get_old_root_vertices(ID maxID) {
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

void HashID::get_backward_root_vertices(ID maxID) {
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

ID *HashID::get_unvisited_neighbors(ID curID, bool* visited_arr, ID *&tmpvalue_arr, ID component) {
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

ID *HashID::get_unvisited_neighbors(ID curID, bool* visited_arr) {
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

void HashID::DFS_V(ID* root_arr, unsigned rootNum, ID *&scc_tag, bool* &visited_arr){
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

void HashID::DFS_V(ID id, ID *&scc_tag, bool *&visited_arr){
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

ID* HashID::find_max_scc_tag(ID *scc_tag, bool* visited_arr) {
	for(unsigned i = ID_count; i > 0; i --) {
		if(!visited_arr[scc_tag[i]]){
			//cout<<"find_max:"<<scc_tag[i]<<":"<<i<<endl;
			return &scc_tag[i];
		}
	}
	return NULL;
}

deque<ID>* HashID::reverse_DFS_V(ID id, bool* &visited_arr) {
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
		scc_num ++;
		return que;
	}
	else return NULL;
	//delete que;
}

void HashID::print_adj() {
	cout<<"pring_adj:"<<endl;
	unsigned count;
	unsigned ct = 0;
	cout<<"maxFromID:"<<maxFromID<<" start_ID[0]:"<<start_ID[0]<<endl;
	for(ID i = start_ID[0]; i <= maxID; i ++) {
		ID *addr = (ID*) getOffset(i);
		cout<<i<<"邻接点：";
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
		cout<<"个数："<<count<<endl;

	}
	cout<<ct<<endl;
}

void HashID::print_reverse_adj() {
	cout<<"print_reverse_adj"<<endl;
	unsigned count;
	unsigned ct = 0;
	cout<<"maxFromID:"<<maxFromID<<" reverse_start_ID[0]:"<<reverse_start_ID[0]<<endl;
	for(ID i = reverse_start_ID[0]; i <= maxID; i ++) {
		ID *addr = (ID*) reverse_getOffset(i);
		cout<<i<<"逆邻接点：";
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
		cout<<"个数："<<count<<endl;

	}
	cout<<ct<<endl;
}

void HashID::print_DAG() {
	cout<<"print_DAG:"<<endl;
	for(ID i = DAG_start_ID[0]; i <= maxID; i ++) {
		//cout<<"for"<<endl;
		ID *addr = (ID*) DAG_getOffset(i);
		cout<<i<<"邻接点：";
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

		ID *addr1 = (ID*) DAG_reverse_getOffset(i);
		cout<<"  逆邻接点： ";
		assert(i == addr1[0]);
		count = addr1[1];
		addr1 += 2;
		for(size_t k = 0; k < count; k ++) {
			cout<<addr1[k]<<" ";
		}

		cout<<endl;
	}

}

void HashID::print_DAG_degree() {
	for(ID i = DAG_start_ID[0]; i <= maxID; i ++) {
		if(scc_located[i] != 0)
			cout<<i<<" in:"<<degree[i].indeg<<" out:"<<degree[i].outdeg<<endl;
	}
}

void HashID::print_scc() {
	cout<<"print_scc:"<<endl;
	for(ID i = 0; i < scc_num; i ++) {
		ID *addr = (ID*) scc_getOffset(scc_start_ID[i]);
		cout<<scc_start_ID[i]<<"： ";
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
		cout<<"个数："<<count<<endl;
	}
}

void HashID::print_excp() {
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
void HashID::print_newGraph() {
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
		cout<<newID_mapto_oldID[newGraph_id[i]]<<"newGraph：";
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
}

void HashID::print_newID_newGraph() {
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

bool HashID::find_cycles(ID id, bool *&visited_arr, bool *cross_vertices_arr) {
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

void HashID::DFS_Sec(string path, ID *&scc_tag, bool *&visited_arr){
	//print_adj();
	//print_reverse_adj();

	bool firstTime = true;
	int fileindex = -1;
	unsigned fileOff = 0;
	EntityIDBuffer* tempEnt = new EntityIDBuffer();
	vector<TempFile*> sccFile;
	size_t pageSize = ID_count * sizeof(ID) / (MemoryBuffer::pagesize) + (((ID_count * sizeof(ID)) % (MemoryBuffer::pagesize)) ? 1 : 0);
	char scc_indexPath[150];
	sprintf(scc_indexPath, "%s/global_scc_index.0", Dir.c_str());
	MMapBuffer *global_scc_indexFile = new MMapBuffer(scc_indexPath, 2 * pageSize * MemoryBuffer::pagesize);
	unsigned *global_scc_index = (unsigned *)global_scc_indexFile->get_address();

	scc_num = 0;
	scc_start_ID = (ID *)calloc(ID_count + 1, sizeof(ID));

//	cout<<"ID_count:"<<ID_count<<endl;
//	for(size_t i = 1; i <= ID_count; i++){
//		cout<<scc_tag[i]<<":"<<i<<" ";
//	}
//	cout<<endl;
	ID *id = find_max_scc_tag(scc_tag, visited_arr);
	deque<ID>* que  = new deque<ID>();
	bool have_scc = false;
	//unsigned sccC = 0;
	//return ;
	while(id != NULL){
		//cout<<"find maxid "<<*id<<endl;
		que = reverse_DFS_V(*id, visited_arr);
		//cout<<"scc_num:"<<scc_num<<endl;

		if(que != NULL){
			//存-------------------scc----------------------------
			have_scc = true;
			ID first_id = que->front();
			if(first_id == 0) first_id = que->back();
			tempEnt->empty();
			tempEnt->insertID(first_id);
			tempEnt->insertID(0); // for num
			scc_start_ID[scc_num - 1] = first_id;
			while(!que->empty()){
				ID id = que->front();
			//	is_scc_vertices_arr[id] = true;
				que->pop_front();
				tempEnt->insertID(id);
				scc_located[id] = first_id;
				degree[id].indeg = 0;
				globalDegree[id].indeg = 0;
			}

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
		}
		id = find_max_scc_tag(scc_tag, visited_arr);
	}

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

void HashID::DFS_Fst(string path, ID *&scc_tag, bool *&visited_arr) {
	get_old_root_vertices(maxID);
	MemoryMappedFile *rootMap = new MemoryMappedFile();
	assert(rootMap->open((Dir+"/root.forward.0").c_str()));
	unsigned rootNum = (rootMap->getEnd() - rootMap->getBegin()) / sizeof(ID);
	ID *root = (ID*) rootMap->getBegin();
	if (rootNum > 0)cout << "root:" << *(root) << "  rootNum:" << rootNum << endl;

	ID_count = 1;

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
	cout<<"maxID:"<<maxID<<"  idcount:"<<ID_count<<endl;
//	for(size_t i = 1; i <= ID_count; i++){
//		cout<<scc_tag[i]<<":"<<i<<" ";
//	}
//	cout<<endl;

	rootMap->close();
	delete rootMap;
	rootMap = NULL;
}

bool HashID::is_new_root(ID root_id, ID *oldroot_arr, unsigned oldrootNum) {
	unsigned i = 0;
	while(root_id > oldroot_arr[i]) {
		i ++;
	}
	if(i >= oldrootNum) return false;
	else if(root_id == oldroot_arr[i])
		return true;
	else return false;
}

void HashID::construct_excp_id(TempFile &excp_id) {
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

void HashID::excp_collect(ID *root_arr, unsigned rootNum, bool *&visited_arr) {
	bool firstTime = true;
	int fileindex = -1;
	unsigned fileOff = 0;
	EntityIDBuffer* tempEnt = new EntityIDBuffer();
	vector<TempFile*> excpFile;
	size_t pageSize = excp_idcount * sizeof(ID) / (MemoryBuffer::pagesize) + (((excp_idcount * sizeof(ID)) % (MemoryBuffer::pagesize)) ? 1 : 0);
	char excp_indexPath[150];
	sprintf(excp_indexPath, "%s/total_excp_index.0", Dir.c_str());
	MMapBuffer *global_excp_indexFile = new MMapBuffer(excp_indexPath, 2 * pageSize * MemoryBuffer::pagesize);
	unsigned *global_excp_index = (unsigned *)global_excp_indexFile->get_address();

	deque<ID>* que = new deque<ID>();
	for(size_t i = 0;i < rootNum; i++){
		if(excp_located[oldID_mapto_newID[root_arr[i]]])
			que->push_back(oldID_mapto_newID[root_arr[i]]);
	}
}

void HashID::deal_with_excp(TempFile &new_graph_root, bool *&cross_vertices_arr) {
	for(unsigned i = 0; i < excp_idcount; i ++) {
		ID *addr = (ID *)excp_getOffset(i);
		assert(i == addr[0]);
		unsigned count = addr[1];
		addr += 2;
		for(size_t k = 1; k < count; k ++) {
			if((idtag[addr[0]] < idtag[addr[k]] || addr[0] > addr[k]) && ! cross_vertices_arr[newID_mapto_oldID[addr[k]]]) {
				cross_vertices_arr[newID_mapto_oldID[addr[k]]] = true;
				new_graph_root.writeId(newID_mapto_oldID[addr[k]]);
				newGraph_idcount ++;
			}
		}
	}
}

void HashID::construct_newGraph(ID *root_arr, unsigned rootNum, TempFile &new_graph, TempFile &new_graph_root, bool *cross_vertices_arr, bool *visited_arr) {
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
	if(have_cross_vertices == true) {
		while (reader < limit){
			ID id = *(ID*) reader;
			reader += sizeof(ID);
			//cout<<id<<": "<<oldID_mapto_newID[id]<<endl;

			if(! visited_arr[id]) {
				que->push_front(id);
				//cout<<"("<<id<<") ";
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
							new_graph.writeId(id);
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
			} //if crossid
		} //while reader
	} //have cross
	idcount --;

	mappedIn.close();
	delete que;
}

void HashID::construct_newGraph_adj(TempFile &new_graph) {
	ID from, to, lastfrom;
	unsigned count = 0;
	MemoryMappedFile mappedIn;
	assert(mappedIn.open(new_graph.getFile().c_str()));
	const char* reader = mappedIn.getBegin(), *limit = mappedIn.getEnd();

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

	TripleBitBuilder::loadTwo(reader, from, to);
	lastfrom = oldID_mapto_newID[from];

	for(unsigned i = 0; i < id_belong_tree[lastfrom]; i++) {
		tempEnt->empty();
		tempEnt->insertID(newGraph_id[i]);
		tempEnt->insertID(0); // for num

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
			excpFile.push_back(new TempFile(Dir + "/newGraph", fileindex));
			//reverse_start_ID[fileindex] = lastto;
			excpFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
			global_excp_index[2 * count] = 1;//actually is '0', but we set '1' here, for '0' has been used to mark the unexist 'to data'
			global_excp_index[2 * count + 1] = fileindex;
			fileOff += size;
			firstTime = false;
		}
		count ++;
	}
	tempEnt->empty();

	while (reader < limit) {
		TripleBitBuilder::loadTwo(reader, from, to);
		from = oldID_mapto_newID[from];
		to = oldID_mapto_newID[to];
		reader = TempFile::skipIdId(reader);

		if (firstInsert) {
			tempEnt->insertID(from);
			tempEnt->insertID(0); // for num
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
				excpFile.push_back(new TempFile(Dir + "/newGraph", fileindex));
				//reverse_start_ID[fileindex] = lastto;
				excpFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
				global_excp_index[2 * count] = 1;//actually is '0', but we set '1' here, for '0' has been used to mark the unexist 'to data'
				global_excp_index[2 * count + 1] = fileindex;
				fileOff += size;
				firstTime = false;
			}
			count ++;

			if(newGraph_id[count] != from){
				for(; count < id_belong_tree[from]; count ++) {
					tempEnt->empty();
					tempEnt->insertID(newGraph_id[count]);
					tempEnt->insertID(0); // for num

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
						excpFile.push_back(new TempFile(Dir + "/newGraph", fileindex));
						//reverse_start_ID[fileindex] = lastto;
						excpFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
						global_excp_index[2 * count] = 1;//actually is '0', but we set '1' here, for '0' has been used to mark the unexist 'to data'
						global_excp_index[2 * count + 1] = fileindex;
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
			tempEnt->insert_uniqueID(to);
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
		excpFile.push_back(new TempFile(Dir + "/newGraph", fileindex));
		//reverse_start_ID[fileindex] = lastto;
		excpFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
		global_excp_index[2 * count] = 1;//actually is '0', but we set '1' here, for '0' has been used to mark the unexist 'to data'
		global_excp_index[2 * count + 1] = fileindex;
		fileOff += size;
		firstTime = false;
	}
	count ++;

	if(count != newGraph_idcount){
		for(; count < newGraph_idcount; count ++) {
			tempEnt->empty();
			tempEnt->insertID(newGraph_id[count]);
			tempEnt->insertID(0); // for num

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
				excpFile.push_back(new TempFile(Dir + "/newGraph", fileindex));
				//reverse_start_ID[fileindex] = lastto;
				excpFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
				global_excp_index[2 * count] = 1;//actually is '0', but we set '1' here, for '0' has been used to mark the unexist 'to data'
				global_excp_index[2 * count + 1] = fileindex;
				fileOff += size;
				firstTime = false;
			}
		}
	}
	cout<<"count:"<<count<<" newGraph_idcount:"<<newGraph_idcount<<endl;

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
}

ID HashID::posttranv_tree(ID id, bool *cross_vertices_arr) {
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

void HashID::construct_newGraph_idtag(bool *cross_vertices_arr) {
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

void HashID::dfs_code(string path, bool *cross_vertices_arr, bool *&visited_arr) {
	get_forward_root_vertices(maxID);

	MemoryMappedFile *rootMap = new MemoryMappedFile();
	assert(rootMap->open((Dir+"/DAGroot.forward.0").c_str()));
	unsigned rootNum = (rootMap->getEnd() - rootMap->getBegin()) / sizeof(ID);
	ID *root_arr = (ID*) rootMap->getBegin();
	if (rootNum > 0)cout << "root:" << *(root_arr) << "  rootNum:" << rootNum << endl;

	newGraph_idcount = 0;
	unsigned cross_idcount = 0;
	ID *cross_id = mark_cross_vertices(path, root_arr, rootNum, visited_arr, cross_vertices_arr, cross_idcount);
	memset(visited_arr, false, (maxID + 1));
	cout<<"cross_idcount:"<<cross_idcount<<endl;

	//cross_idcount = 0;

//	//exceptions
//	bool firstTime = true;
//	int fileindex = -1;
//	unsigned fileOff = 0;
//	EntityIDBuffer* tempEnt = new EntityIDBuffer();
//	vector<TempFile*> excpFile;
//	size_t pageSize = DAG_idcount * sizeof(ID) / (MemoryBuffer::pagesize) + (((DAG_idcount * sizeof(ID)) % (MemoryBuffer::pagesize)) ? 1 : 0);
//	char excp_indexPath[150];
//	sprintf(excp_indexPath, "%s/global_excp_index.0", Dir.c_str());
//	MMapBuffer *global_excp_indexFile = new MMapBuffer(excp_indexPath, 2 * pageSize * MemoryBuffer::pagesize);
//	unsigned *global_excp_index = (unsigned *)global_excp_indexFile->get_address();
//
//	//new graph
//	bool NG_firstTime = true;
//	int NG_fileindex = -1;
//	unsigned NG_fileOff = 0;
//	EntityIDBuffer* NG_tempEnt = new EntityIDBuffer();
//	vector<TempFile*> NG_File;
//	size_t NG_pageSize = (newGraph_idcount + 1) * sizeof(ID) / (MemoryBuffer::pagesize) + ((((newGraph_idcount + 1) * sizeof(ID)) % (MemoryBuffer::pagesize)) ? 1 : 0);
//	char NG_indexPath[150];
//	sprintf(NG_indexPath, "%s/global_newGraph_index.0", Dir.c_str());
//	MMapBuffer *global_NG_indexFile = new MMapBuffer(NG_indexPath, 2 * pageSize * MemoryBuffer::pagesize);
//	unsigned *global_NG_index = (unsigned *)global_NG_indexFile->get_address();

	cout<<"newGraph_idcount:"<<newGraph_idcount<<endl;
	//newGraph_id = (ID *)calloc(newGraph_idcount, sizeof(ID));
	idcount = 1;
	//newGraph_idcount = 0;
	newID_mapto_oldID = (ID *)calloc(maxID + 2, sizeof(ID));
	oldID_mapto_newID = (ID *)calloc(maxID + 1, sizeof(ID));
	excp_located = (ID *)calloc(maxID + 1, sizeof(ID));
	memset(excp_located, 0, (maxID + 1));

	//TempFile cross_distribution(path+"/cross_distribution");
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
			//newGraph_id[newGraph_idcount] = idcount;
			//newGraph_idcount ++;
			new_graph_root.writeId(root_arr[index]);

			//cout<<"root:"<<root_arr[index]<<" "<<newGraph_idcount<<endl;
//			NG_tempEnt->empty();
//			NG_tempEnt->insertID(idcount);
//			NG_tempEnt->insertID(0);

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
//				if(curID != addr[0]){
//					cout<<"curID:"<<curID<<" addr[0]:"<<addr[0]<<" scc:"<<scc_located[curID]<<endl;
//				}
				assert(curID == addr[0]);
				count = addr[1];
				addr += 2; //skip the id and count

				//tempEnt->empty();
				//tempEnt->insertID(oldID_mapto_newID[curID]);
				//tempEnt->insertID(0);
				have_excp_id = false;

				for (size_t i = 0; i < count; i++) {
					DAG_maxID = std::max(addr[i], DAG_maxID);

					if(degree[addr[i]].indeg)degree[addr[i]].indeg--;
					if(cross_vertices_arr[addr[i]]){
						have_cross_vertices = true;
						//cross_distribution.writeId(addr[i]);

						//new_graph.writeId(root_arr[index]);
						//new_graph.writeId(addr[i]);
						//NG_tempEnt->insert_uniqueID(addr[i]);
						continue;
					}
					if(visited_arr[addr[i]])continue;
					if(degree[addr[i]].indeg){
						//tempEnt->insertID(addr[i]);
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

				//存异常
//				unsigned num = tempEnt->getSize() - 2;
//				tempEnt->getBuffer()[1] = num; //update num
//				unsigned size = tempEnt->getSize() * sizeof(ID);
//				if (!firstTime && fileOff + size < (unsigned) (-1) / 8) {
//					excpFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
//					global_excp_index[2 * oldID_mapto_newID[curID]] = fileOff;
//					global_excp_index[2 * oldID_mapto_newID[curID] + 1] = fileindex;
//					fileOff += size;
//				} else {
//					if (fileindex >= 0) {
//						excpFile[fileindex]->close();
//					}
//
//					fileindex++;
//					fileOff = 0;
//					excpFile.push_back(new TempFile(Dir + "/exceptions", fileindex));
//					excp_start_ID[fileindex] = oldID_mapto_newID[curID];
//					excpFile[fileindex]->write(size,(const char*) tempEnt->getBuffer());
//					global_excp_index[2 * oldID_mapto_newID[curID]] = 1;//actually is '0', but we set '1' here, for '0' has been used to mark the unexist 'to data'
//					global_excp_index[2 * oldID_mapto_newID[curID] + 1] = fileindex;
//					fileOff += size;
//					firstTime = false;
//				} //else

			} //while

			//存newGraph
//			unsigned NG_num = NG_tempEnt->getSize() - 2;
//			NG_tempEnt->getBuffer()[1] = NG_num; //update num
//			unsigned NG_size = NG_tempEnt->getSize() * sizeof(ID);
//			if (!NG_firstTime && NG_fileOff + NG_size < (unsigned) (-1) / 8) {
//				NG_File[NG_fileindex]->write(NG_size,(const char*) NG_tempEnt->getBuffer());
//				global_NG_index[2 * newGraph_idcount] = NG_fileOff;
//				global_NG_index[2 * newGraph_idcount + 1] = NG_fileindex;
//				NG_fileOff += NG_size;
//			} else {
//				if (NG_fileindex >= 0) {
//					NG_File[NG_fileindex]->close();
//				}
//
//				NG_fileindex++;
//				NG_fileOff = 0;
//				NG_File.push_back(new TempFile(Dir + "/newGraph", NG_fileindex));
//				//newGraph_start_ID[NG_fileindex] = oldID_mapto_newID[root_arr[index]];
//				NG_File[NG_fileindex]->write(NG_size,(const char*) NG_tempEnt->getBuffer());
//				global_NG_index[2 * newGraph_idcount] = 1;//actually is '0', but we set '1' here, for '0' has been used to mark the unexist 'to data'
//				global_NG_index[2 * newGraph_idcount + 1] = NG_fileindex;
//				NG_fileOff += NG_size;
//				NG_firstTime = false;
//			}

			//newGraph_idcount ++;
		} //for
	} //else

	//global_excp_indexFile->flush();
	//global_NG_indexFile->flush();

//	for(size_t index = 1;index < idcount;index++){
//
//		cout<<"oldID:"<<newID_mapto_oldID[index]<<" newID:"<<oldID_mapto_newID[newID_mapto_oldID[index]]<<endl;
//	}

	//cross_distribution.flush();
	excp_id.flush();
	//new_graph.flush();
	new_graph_root.flush();

	cout<<"root code finish"<<endl;

	//TempFile sorted_cross_distribution(path+"/sorted_cross_distribution");
	if(have_cross_vertices == true) {
		qsort(cross_id, cross_idcount, sizeof(ID), qcompare);
		//Sorter::sort(cross_distribution, sorted_cross_distribution, TempFile::skipId, TempFile::compare);
		cross_collector(cross_id, cross_idcount, cross_vertices_arr, visited_arr, excp_id, new_graph_root);
	}
	idcount --;

	//excpFile[fileindex]->close();
	//global_excp_indexFile->flush();

	//NG_File[NG_fileindex]->close();
	//global_NG_indexFile->flush();

	//init_newGraph();
	//init_excp();

	new_graph_root.flush();
	excp_id.flush();
	//new_graph.flush();
	construct_excp_id(excp_id);
	//memset(visited_arr, false, (maxID + 1));
	//excp_collect(root_arr, rootNum, visited_arr);

	cout<<"in subtree_collector(), idcount = "<<idcount<<" DAG_maxID:"<<DAG_maxID<<endl;
	//cout<<"newGraph_start_ID"<<newGraph_start_ID[0]<<endl;
	cout<<"cross code finish!"<<endl;

	cout<<"scc_num:"<<scc_num<<endl;
	//cout<<"newGraph_idcount:"<<newGraph_idcount<<endl;
	cout<<"excp_count:"<<excp_count<<endl;

	//assert(false);

//	for(size_t index = 1;index <= idcount;index++){
//
//		cout<<"oldID:"<<newID_mapto_oldID[index]<<" newID:"<<oldID_mapto_newID[newID_mapto_oldID[index]]<<endl;
//	}
//
//	cout<<endl;

	cout<<"maxID:"<<maxID<<endl;
	//assert(false);
	//print_excp();
	//print_newGraph();
	//print_newID_newGraph();

	idtag = (ID *)calloc(idcount + 1, sizeof(ID));
	cout<<"mark tag begin:"<<endl;
	mark_vertices_tag(new_graph_root, cross_vertices_arr);
	cout<<"mark tag end:"<<endl;
//	for(size_t index = 1;index <= idcount;index++) {
//		cout<<"id:"<<index<<": "<<idtag[index]<<" ";
//	}
//	cout<<endl;

	deal_with_excp(new_graph_root, cross_vertices_arr);
	new_graph_root.flush();
	cout<<"deal_with_excp end"<<endl;
	memset(visited_arr, false, (maxID + 1));
	memset(oldID_mapto_newID, 0, (maxID + 1));
	memset(newID_mapto_oldID, 0, (maxID + 2));
	id_belong_tree = (unsigned *)calloc(idcount + 1, sizeof(unsigned));
	construct_newGraph(root_arr, rootNum, new_graph, new_graph_root, cross_vertices_arr, visited_arr);
	cout<<"construct_newGraph end"<<endl;
//	for(size_t index = 1;index <= idcount;index++){
//
//		cout<<"oldID:"<<newID_mapto_oldID[index]<<" newID:"<<oldID_mapto_newID[newID_mapto_oldID[index]]<<endl;
//	}
//
//	cout<<endl;

	new_graph.flush();
	construct_newGraph_adj(new_graph);
	cout<<"construct_newGraph_adj"<<endl;
	memset(idtag, 0, (idcount + 1));
	construct_newGraph_idtag(cross_vertices_arr);
	cout<<"construct_newGraph_idtag end"<<endl;
	cout<<"idcount:"<<idcount<<endl;
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
	//free(oldID_mapto_newID);
	//oldID_mapto_newID = NULL;
	//free(newID_mapto_oldID);
	//newID_mapto_oldID = NULL;

	//qsort(newGraph_id, newGraph_idcount, sizeof(ID), qcompareNewGraphID);
	//print_newGraph();
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

//	for(int i = 0;i < excpFile.size();i++){
//		delete excpFile[i];
//		excpFile[i] = NULL;
//	}
//	excpFile.clear();
//	for(int i = 0;i < NG_File.size();i++){
//		delete NG_File[i];
//		NG_File[i] = NULL;
//	}
//	NG_File.clear();
//	delete global_excp_indexFile;
//	delete global_NG_indexFile;
//
//	delete tempEnt;
//	delete NG_tempEnt;
	delete que;
	rootMap->close();
	delete rootMap;

	remove(new_graph.getFile().c_str());
	remove(new_graph_root.getFile().c_str());
	remove(excp_id.getFile().c_str());

//	remove(cross_distribution.getFile().c_str());
//	remove(sorted_cross_distribution.getFile().c_str());
}

bool HashID::find_visited(ID id, ID *addr, unsigned k) {
	for(size_t i = 0; i < k; i ++){
		if(scc_located[id] == addr[i])
			return true;
	}
	return false;
}

bool HashID::is_equal(deque<ID>* que, ID curID) {
	for(size_t i = 0; i < que->size(); i ++){
		if(que->at(i) == curID){
			return true;
		}
	}

	return false;
}

void HashID::deal_with_scc(ID id, unsigned &ct, ID *&scc_visited_firstid, EntityIDBuffer *&tempEnt, TempFile &DAGfile) {
	deque<ID>* que = new deque<ID>();
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
				if(tempEnt->is_existed(addr[k])) {
					degree[addr[k]].indeg --;
					globalDegree[addr[k]].indeg --;
				}
				else {
					tempEnt->insertID(addr[k]);
					DAGfile.writeId(id);
					DAGfile.writeId(addr[k]);
				}
				//else que->push_back(addr[k]);
			}
			else if(scc_located[addr[k]] != 0 && scc_located[addr[k]] != scc_located[id] && ! tempEnt->is_existed(scc_located[addr[k]])){
				//que->push_back(scc_located[addr[k]]);
				tempEnt->insertID(scc_located[addr[k]]);
				degree[scc_located[addr[k]]].indeg ++;
				globalDegree[scc_located[addr[k]]].indeg ++;

				DAGfile.writeId(id);
				DAGfile.writeId(scc_located[addr[k]]);
			}
		}
	}

//	while(! que->empty()){
//		ID curID = que->front();
//		que->pop_front();
//		tempEnt->insertID(curID);
//	}

	scc_visited_firstid[ct] = scc_located[id];
	ct ++;

	delete que;
}

status HashID::generate_DAG_reverse_adj(TempFile &DAGfile) {
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
			adjFile.push_back(new TempFile(Dir + "/DAG_reverse_adj", fileindex));
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

void HashID::generate_DAG(string path) {
	bool firstInsert = true, firstTime = true;
	int fileindex = -1;
	unsigned fileOff = 0;
	EntityIDBuffer* tempEnt = new EntityIDBuffer();
	vector<TempFile*> DAGFile;
	size_t pageSize = (maxID + 1) * sizeof(ID) / (MemoryBuffer::pagesize) + ((((maxID + 1) * sizeof(ID)) % (MemoryBuffer::pagesize)) ? 1 : 0);
	char DAG_indexPath[150];
	sprintf(DAG_indexPath, "%s/global_DAG_index.0", Dir.c_str());
	MMapBuffer *global_DAG_indexFile = new MMapBuffer(DAG_indexPath, 2 * pageSize * MemoryBuffer::pagesize);
	unsigned *global_DAG_index = (unsigned *)global_DAG_indexFile->get_address();

	//bool *scc_visited = (bool *)calloc(scc_num, sizeof(bool));
	//memset(scc_visited, false, (scc_num + 1));
	ID *scc_visited_firstid = (ID *)calloc(scc_num, sizeof(ID));

	DAG_idcount = 0;
	TempFile DAGfile(path + "/DAGfile");
	//cout<<"startID:"<<start_ID[0]<<" maxID:"<<maxID<<endl;
	for (ID i = start_ID[0]; i <= maxID; i ++) {

		ID *addr;
		addr = (ID*) getOffset(i);
		//cout<<i<<"邻接点：";
		if (addr == NULL){
			cout<<"id:"<<i<<" null"<<endl;
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

			for(unsigned k = 0; k < count; k++) {
				//cout<<addr[k]<<" ";

				if(scc_located[i] == 0){
					if(scc_located[addr[k]] != 0){
//						ID *sccaddr = (ID*) scc_getOffset(scc_located[addr[k]]);
//						assert(scc_located[addr[k]] == sccaddr[0]);
//						if(find_visited(addr[k], addr, k))
//							continue;
//
//						if(scc_located[addr[k]] != addr[k])
//							addr[k] = scc_located[addr[k]];
						if(!tempEnt->is_existed(scc_located[addr[k]])){
							tempEnt->insertID(scc_located[addr[k]]);
							degree[scc_located[addr[k]]].indeg ++;
							globalDegree[scc_located[addr[k]]].indeg ++;
							DAGfile.writeId(i);
							DAGfile.writeId(scc_located[addr[k]]);
						}
					}
					else{
						tempEnt->insertID(addr[k]);
						DAGfile.writeId(i);
						DAGfile.writeId(addr[k]);
					}
				} //if scc
				else if(scc_located[i] == i) {
					//if(scc_located[i] == scc_located[addr[k]]) continue;
					bool tag = false;
					for(size_t n = 0; n < ct; n ++) {
						if(scc_located[i] == scc_visited_firstid[n])
							tag = true;
					}
					//cout<<"i"<<i<<endl;
					if(tag == false)
						deal_with_scc(i, ct, scc_visited_firstid, tempEnt, DAGfile);
				}

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
			DAGFile.push_back(new TempFile(Dir + "/DAG", fileindex));
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
	//print_DAG();

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

unsigned HashID::belong_tree(ID id) {
	unsigned i = 0;
	while(i < newGraph_idcount && id >= newGraph_id[i])
		i ++;
	return i - 1;
}

bool HashID::tree_reach(ID x, ID y) {
	if(x <= y && idtag[x] >= idtag[y])
		return true;
//	else {
//		deque<ID>* que  = new deque<ID>();
//		que->push_back(x);
//		while(!que->empty()) {
//			ID curID = que->front();
//			que->pop_front();
//
//			if(curID <= y && idtag[curID] >= idtag[y])
//				return true;
//
//			if(excp_located[curID]) {
//				ID *addr = (ID *)excp_getOffset(excp_located[curID]);
//				assert(addr[0] == excp_located[curID] - 1);
//				unsigned count = addr[1];
//				addr += 2;
//				assert(curID == addr[0]);
//				for(size_t i = 1; i < count; i ++) {
//					if(y == addr[i])
//						return true;
//					unsigned addr_belong = id_belong_tree[addr[i]];
//					if(addr_belong == x_belong)
//						que->push_back(addr[i]);
//				} //for
//			}
//		} //while
//	} //else
	return false;
}

bool HashID::new_tree_reach(ID x, ID y, unsigned x_belong) {
	if(x <= y && idtag[x] >= idtag[y])
		return true;
//	else {
//		deque<ID>* que  = new deque<ID>();
//		que->push_back(x);
//		while(!que->empty()) {
//			ID curID = que->front();
//			que->pop_front();
//
//			if(curID <= y && idtag[curID] >= idtag[y])
//				return true;
//
//			if(excp_located[curID]) {
//				ID *addr = (ID *)excp_getOffset(excp_located[curID]);
//				assert(addr[0] == excp_located[curID] - 1);
//				unsigned count = addr[1];
//				addr += 2;
//				assert(curID == addr[0]);
//				for(size_t i = 1; i < count; i ++) {
//					if(y == addr[i])
//						return true;
//					unsigned addr_belong = id_belong_tree[addr[i]];
//					if(addr_belong == x_belong)
//						que->push_back(addr[i]);
//				} //for
//			}
//		}
//	}
	return false;
}

bool HashID::newGraph_reachable(ID id1, ID id2) {
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
//			if(! oldID_mapto_newID[addr[i]]){
//				cout<<"oldid:"<<addr[i]<<" newid:"<<oldID_mapto_newID[addr[i]]<<" ";
//				if(zeroDegree(addr[i])) cout<<"zeroDegree ";
//				if(scc_located[addr[i]]) cout<<"scc ";
//				if(cross_vertices_arr[addr[i]]) cout<<"cross   ";
//			}
		}
	}
	return false;
}

bool HashID::find_reach_path(ID x, ID y) {
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

//void HashID::vertices_reachable(ID x, ID y, bool &reachable) {
//	bool tag = false;
//	unsigned x_belong = id_belong_tree[x];
//	//cout<<random_x[i]<<" belong: "<<x_belong<<" "<<newGraph_id[x_belong]<<endl;
//	ID x_next_belong;
//	if(x_belong == newGraph_idcount - 1) x_next_belong = idcount;
//	else x_next_belong = newGraph_id[x_belong + 1];
//	if(id_belong_tree[y] == x_belong) {
//		reachable = tree_reach(x, y);
//	}
//	else {
//		unsigned y_belong = id_belong_tree[y];
//		//cout<<random_y[i]<<" belong: "<<y_belong<<" "<<newGraph_id[y_belong]<<endl;
//		if(!newGraph_reachable(newGraph_id[x_belong], newGraph_id[y_belong])) {
//			reachable = false;
//		} else {
//			unsigned id_num = 0;
//			ID *id = find_reach_path(newGraph_id[x_belong], newGraph_id[y_belong], id_num);
//
//			//cout<<"reach path:";
//			for(size_t j = 0; j < id_num; j++) {
//				//cout<<id[j]<<" ";
//				ID *addr = (ID *)reverse_getOffset(newID_mapto_oldID[id[j]]);
//				assert(addr[0] == newID_mapto_oldID[id[j]]);
//				unsigned count = addr[1];
//				addr += 2;
//				for(size_t k = 0; k < count; k ++) {
//					if(!scc_located[addr[k]]) {
//						if(oldID_mapto_newID[addr[k]] >= newGraph_id[x_belong] && oldID_mapto_newID[addr[k]] < x_next_belong)
//							tag = tree_reach(x, oldID_mapto_newID[addr[k]]);
//					} else {
//						ID scc_id = scc_located[addr[k]];
//						if(oldID_mapto_newID[scc_id] >= newGraph_id[x_belong] && oldID_mapto_newID[scc_id] < x_next_belong)
//							tag = tree_reach(x, oldID_mapto_newID[scc_id]);
//					}
//					if(tag == true) {
//						//reachable[i] = true;
//						break;
//					}
//				} //for k
//				if(tag == true) {
//					reachable = true;
//					break;
//				}
//			} //for j
//			//cout<<endl;
//		}
//	}
//}

void HashID::vertices_reachability(struct timeval &start_time, struct timeval &end_time1) {
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

//		for (size_t i = 0; i < num; i++) {
//			cout<<"("<<random_x[i]<<","<<random_y[i]<<"):";
//		}
//		cout<<endl;
	//qsort(random_x, num, sizeof(ID), qcompare);
	//qsort(random_y, num, sizeof(ID), qcompare);
//	 random_x[0] = 6;
//	 random_y[0] = 18;
//	 random_x[1] = 1;
//	 random_y[1] = 18;
//	 random_x[2] = 1;
//	 random_y[2] = 19;
//	 random_x[3] = 98010;
//	 random_y[3] = 107579;
//	 cout<<"newGraph_id[98011]:"<<belong_tree(98010)<<" "<<newGraph_id[belong_tree(98010)]<<endl;
//	 ID *addr = (ID *) newGraph_getOffset(newGraph_id[belong_tree(98010)]);
//	 cout<<"addr[1]:"<<addr[1]<<" addr[2]:"<<addr[2]<<endl;

	cout<<"reach query begin:"<<endl;
	gettimeofday(&start_time, NULL);
//	for(size_t i = 0; i < num; i++) {
//		bool tag = false;
//		unsigned x_belong = id_belong_tree[random_x[i]];
//		//cout<<random_x[i]<<" belong: "<<x_belong<<" "<<newGraph_id[x_belong]<<endl;
//		ID x_next_belong;
//		if(x_belong == newGraph_idcount - 1) x_next_belong = idcount;
//		else x_next_belong = newGraph_id[x_belong + 1];
//		if(id_belong_tree[random_y[i]] == x_belong ) {
//			r = tree_reach(random_x[i], random_y[i], x_belong);
//		}
//		else {
////			if(excp_located[random_x[i]]) {
////				ID *excp_addr = (ID *) excp_getOffset(excp_located[random_x[i]]);
////				assert(excp_addr[0] == excp_located[random_x[i]] - 1);
////				unsigned excp_count = excp_addr[1];
////				excp_addr += 2;
////				assert(excp_addr[0] == random_x[i]);
////				for(size_t n = 1; n < excp_count; n ++) {
////					if(random_y[i] == excp_addr[n]){
////						reachable[i] = true;
////						break;
////					}
////				}
////			}
//
//			unsigned y_belong = id_belong_tree[random_y[i]];
//			//cout<<random_y[i]<<" belong: "<<y_belong<<" "<<newGraph_id[y_belong]<<endl;
//			unsigned id_num = 0;
//			ID *id = find_reach_path(newGraph_id[x_belong], newGraph_id[y_belong], id_num);
//
//			if(id_num == 0)
//				r = false;
//			else {
//				for(size_t j = 0; j < id_num; j++) {
//					//cout<<id[j]<<" ";
//					ID *addr = (ID *)DAG_reverse_getOffset(newID_mapto_oldID[id[j]]);
//					assert(addr[0] == newID_mapto_oldID[id[j]]);
//					unsigned count = addr[1];
//					addr += 2;
//					for(size_t k = 0; k < count; k ++) {
//						if(id_belong_tree[oldID_mapto_newID[addr[k]]] == x_belong)
//							r = tree_reach(random_x[i], oldID_mapto_newID[addr[k]], x_belong);
//						if(r) {
//							//reachable[i] = true;
//							break;
//						}
//					} //for k
//					if(r) {
//						break;
//					}
//				} //for j
//			} //else
//		} //else
//	} //for i

	for(vector<ID>::iterator x=src.begin(), y=dest.begin(); x!=src.end(); x++, y++) {
		//bool tag = false;
		//unsigned x_belong = id_belong_tree[*x];
		//cout<<random_x[i]<<" belong: "<<x_belong<<" "<<newGraph_id[x_belong]<<endl;
		//ID x_next_belong;
		//if(x_belong == newGraph_idcount - 1) x_next_belong = idcount;
		//else x_next_belong = newGraph_id[x_belong + 1];
		if(id_belong_tree[*y] == id_belong_tree[*x] ) {
			r = tree_reach(*x, *y);
		}
		else {
//			if(excp_located[random_x[i]]) {
//				ID *excp_addr = (ID *) excp_getOffset(excp_located[random_x[i]]);
//				assert(excp_addr[0] == excp_located[random_x[i]] - 1);
//				unsigned excp_count = excp_addr[1];
//				excp_addr += 2;
//				assert(excp_addr[0] == random_x[i]);
//				for(size_t n = 1; n < excp_count; n ++) {
//					if(random_y[i] == excp_addr[n]){
//						reachable[i] = true;
//						break;
//					}
//				}
//			}

			//unsigned y_belong = id_belong_tree[*y];
			//cout<<random_y[i]<<" belong: "<<y_belong<<" "<<newGraph_id[y_belong]<<endl;
			//unsigned id_num = 0;
			r = find_reach_path(*x, *y);

//			if(id_num == 0)
//				r = false;
//			else {
//				for(size_t j = 0; j < id_num; j++) {
//					//cout<<id[j]<<" ";
//					ID *addr = (ID *)DAG_reverse_getOffset(newID_mapto_oldID[id[j]]);
//					assert(addr[0] == newID_mapto_oldID[id[j]]);
//					unsigned count = addr[1];
//					addr += 2;
//					for(size_t k = 0; k < count; k ++) {
//						if(id_belong_tree[oldID_mapto_newID[addr[k]]] == x_belong)
//							r = tree_reach(*x, oldID_mapto_newID[addr[k]]);
//						if(r) {
//							//reachable[i] = true;
//							break;
//						}
//					} //for k
//					if(r) {
//						break;
//					}
//				} //for j
//			} //else
		} //else
	} //for i
	gettimeofday(&end_time1, NULL);
	cout<<"reachability build time elapse:"<<(end_time1.tv_sec - start_time.tv_sec) * 1000.0 + (end_time1.tv_usec - start_time.tv_usec)*1.0/ 1000.0<<" ms"<<endl;
//	for (size_t i = 0; i < num; i++) {
//		cout<<"("<<random_x[i]<<","<<random_y[i]<<"):";
//		if(reachable[i]) cout<<"yes!";
//		else cout<<"no!";
//		if(i % 5 == 0)cout<<endl;
//	}
//	cout<<endl;
}

void HashID::convert_DAG_adj() {
	init_DAG();
	init_reverse_DAG();
}

void HashID::pre_code(string path, unsigned is_first, struct timeval &start_time, struct timeval &end_time, struct timeval &end_time1) {
	cout << "in DFS Encode, maxID: " << maxID << endl;

	string fileName1 = string(path+"/visited_vertices");
	MMapBuffer *visited_vertices = new MMapBuffer(fileName1.c_str(), (maxID+1) * sizeof(bool));
	bool *visited_arr = (bool *)(visited_vertices->get_address());
	memset(visited_arr, false, (maxID+1));

	string fileName2 = string(path+"/cross_vertices");
	MMapBuffer *cross_vertices = new MMapBuffer(fileName2.c_str(), (maxID+1) * sizeof(bool));
	bool *cross_vertices_arr = (bool *)(cross_vertices->get_address());
	//memset(cross_vertices_arr, false, (maxID+1));

	//print_adj();
	//print_reverse_adj();
	if(is_first == 1) {
		ID *scc_tag = (ID *)calloc(maxID + 1, sizeof(ID));
		memset(scc_tag, 0, (maxID + 1));
		DFS_Fst(path, scc_tag, visited_arr);
		cout<<"DFS_Fst finish!"<<endl;

		scc_located = (ID *)calloc(maxID + 1, sizeof(ID));
		memset(scc_located, 0, (maxID + 1));

		//init(false);
		//cout<<"init"<<endl;
		//memset(scc_tag, 0, (maxID + 1));
		cout<<"dfs_s"<<endl;
		memset(visited_arr, false, (maxID + 1));
		DFS_Sec(path, scc_tag, visited_arr);
		cout<<"DFS_Sec finish!"<<endl;
		//print_adj();

		cout<<"generate_DAG begin:"<<endl;
		memset(visited_arr, false, (maxID + 1));
		//generate_DAG(path, cross_vertices_arr, visited_arr);
		generate_DAG(path);
		cout<<"generate_DAG finish!"<<endl;
		//print_DAG();
		//print_DAG_degree();
		gettimeofday(&end_time, NULL);
	} else {
		convert_DAG_adj();
	}

	memset(visited_arr, false, (maxID + 1));
	memset(cross_vertices_arr, false, (maxID+1));
	cout<<"dfs_code begin:"<<endl;
	dfs_code(path, cross_vertices_arr, visited_arr);
	cout<<"dfs_code finish!"<<endl;
	//print_newGraph();
	//print_newID_newGraph();
	//gettimeofday(&start_time, NULL);

//	for(size_t i = 0; i <= maxID; i ++) {
//		if(cross_vertices_arr[i])
//			cout<<"cross:"<<i<<" ";
//	}

	vertices_reachability(start_time, end_time1);
	//gettimeofday(&end_time, NULL);

	remove(fileName1.c_str());
	remove(fileName2.c_str());
	delete cross_vertices;
	delete visited_vertices;
}
//------------------------------------------------------------------------------------------------------------------------//

template <typename T>
static bool find_pos(T *t, ID vertex, int low, int high, int &mid, int &low_bound, int &high_bound){
	bool hit = false;
	mid = 0, low_bound = 0, high_bound = 0;
	
	while(low <= high){
		mid = (low+high)/2;
		if(vertex > t[mid].startid)low = mid+1;
		else if(vertex < t[mid].startid)high = mid-1;
		else {
			hit = true;
			break;
		}
	}
	//cout<<"mid: "<<mid<<endl;
	//cout<<t[mid].startid<<" "<<vertex<<endl;
	int tmp_mid = mid;
	
	if(!hit){
		if(vertex > t[tmp_mid].startid){
			low_bound = tmp_mid;
			if(tmp_mid < high){
				tmp_mid++;
				while(tmp_mid < high && vertex > t[tmp_mid].startid){
					low_bound = tmp_mid;
					tmp_mid++;
				}
			}
			high_bound = tmp_mid;
		}else if(vertex < t[tmp_mid].startid){
			high_bound = tmp_mid;
			if(tmp_mid > 0){
				tmp_mid--;
				while(tmp_mid > 0 && vertex < t[tmp_mid].startid){
					high_bound = tmp_mid;
					tmp_mid--;
				}
			}
			low_bound = tmp_mid;
		}
		
		assert(low_bound <= high_bound);
	}
	
	return hit;
}

static void decode(vector<unsigned> &data, const char *reader, const char *limit){
	register unsigned shift = 0;
	ID id = 0;
	register unsigned int c;
	int count = 0;
	
	while (reader <= limit) {
		id = 0;
		shift = 0;
		c = *reader;
		if (!(c & 128)) {
			//head = 0
			id |= c << shift;
			shift += 7;
			reader++;
			if (reader > limit)break;
			
			c = *reader;
			while (!(c & 128)) {
				id |= c << shift;
				shift += 7;
				reader++;
				if (reader > limit)break;
				c = *reader;
			}
			
			count++;
			if(count % 2){
				if(count == 1 || count == 3){
					data.push_back(id);
				}
				else{
					data.push_back(id+data.back());
				}
			}
			else{
				data.push_back(id+data.back());
			}
		} else {
			//head = 1
			id |= (c & 0x7F) << shift;
			shift += 7;
			reader++;
			if (reader > limit)break;
			
			c = *reader;
			while (c & 128) {
				id |= (c & 0x7F) << shift;
				shift += 7;
				reader++;
				if (reader > limit)break;
				c = *reader;
			}
			
			count++;
			if(count % 2){
				if(count == 1 || count == 3){
					data.push_back(id);
				}
				else{
					data.push_back(id+data.back());
				}
			}
			else{
				data.push_back(id+data.back());
			}
		}
	}
}

static bool find_vertex(vector<unsigned> data, ID vertex){
	//cout<<"first id range: ["<<data[0]<<", "<<data[1]<<"]"<<endl;
	size_t size = data.size();
	for(size_t i = 0;i < size/2;i++){
		assert(data[2*i] <= data[2*i+1]);
		//cout<<"["<<data[2*i]<<", "<<data[2*i+1]<<"] ";
	}
	//cout<<endl;
	if(vertex >= data[0] && vertex <= data[1])return true;
	
	int low = 1, high = size/2 -1, mid, len = high;
	while(low <= high){
		mid = (low+high)/2;
		if(vertex > data[2*mid+1])low = mid+1;
		else if(vertex < data[2*mid+1])high = mid-1;
		else {
			//find it
			return true;
		}
	}
	
	if(vertex > data[2*mid+1]){
		if(mid < len){
			mid++;
			while(mid < len && vertex > data[2*mid+1])mid++;
		}
		if(vertex >= data[2*mid] && vertex <= data[2*mid+1])return true;
	}else{
		if(mid > 0){
			mid--;
			while(mid > 0 && vertex < data[2*mid+1])mid--;
		}
		if(mid < len)mid++;
		if(vertex >= data[2*mid] && vertex <= data[2*mid+1])return true;
	}
	
	return false;
}

bool HashID::intersect_or_not(ID vertex1, ID vertex2, startid_offset_index *offset_index_arr, unsigned len, startid_offset *offset_arr, const char *adj_ptr){
	int mid, low_bound, high_bound;
	bool hit = false;
	size_t low_offset = 0, high_offset = 0;
	
	hit = find_pos<startid_offset_index>(offset_index_arr, vertex1, 0, len-1, mid, low_bound, high_bound);
	//cout<<"hit1: "<<hit<<endl;
	//cout<<offset_arr[offset_index_arr[mid].len].startid<<" "<<offset_arr[offset_index_arr[mid+1].len-1].startid<<endl;
	//cout<<offset_arr[offset_index_arr[low_bound].len].startid<<" "<<offset_arr[offset_index_arr[low_bound+1].len-1].startid<<endl;
	//cout<<offset_arr[offset_index_arr[high_bound].len].startid<<" "<<offset_arr[offset_index_arr[high_bound+1].len-1].startid<<endl;
	
	if(hit){
		low_offset = offset_arr[offset_index_arr[mid].len].offset;
		high_offset = offset_arr[offset_index_arr[mid].len+1].offset;
	}else{
		if(low_bound == high_bound){
			high_bound = low_bound + 1;
		}
		hit = find_pos<startid_offset>(offset_arr, vertex1, offset_index_arr[low_bound].len, offset_index_arr[high_bound].len-1, mid, low_bound, high_bound);
		//cout<<"hit2: "<<hit<<endl;
		if(hit){
			low_offset = offset_arr[mid].offset;
			high_offset = offset_arr[mid+1].offset;
		}else{
			low_offset = offset_arr[low_bound].offset;
			high_offset = offset_arr[low_bound+1].offset;
		}
	}
	
	assert(low_offset <= high_offset);
	//cout<<"offset range: ["<<low_offset<<", "<<high_offset<<"]"<<endl;
	
	vector<unsigned> data;
	decode(data, adj_ptr+low_offset, adj_ptr+high_offset);
	hit = find_vertex(data, vertex2);
	return hit;
}
