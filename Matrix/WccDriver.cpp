/*
 * WccDriver.cpp
 *
 *  Created on: Mar 18, 2014
 *      Author: root
 */

#include "WccDriver.h"
#include "OSFile.h"
#include "HashID.h"

#define WCC_DEBUG 1

WccDriver::WccDriver() {
	// TODO Auto-generated constructor stub
	matrixMap = NULL;
	memset(start_ID,0,sizeof(ID)*100);
	maxID = 0;
	bucSize = 0;
	idValue = NULL;
	isVisited = NULL;
	isParallel = true;
	pthread_mutex_init(&print_mutex, NULL);
}

WccDriver::~WccDriver() {
	// TODO Auto-generated destructor stub
	if (matrixMap)delete matrixMap;
	matrixMap = NULL;

	if(isParallel){
		for (int i = 0; i < bucSize/BATCH_PARTITION; i++) {
			if(boundary_messages1[i])delete boundary_messages1[i];
			boundary_messages1[i] = NULL;
			if(boundary_messages2[i])delete boundary_messages2[i];
			boundary_messages2[i] = NULL;
			pthread_mutex_destroy(&message_mutex[i]);
			if(cc_map[i])delete cc_map[i];
			cc_map[i] = NULL;
		}
		boundary_messages1.clear();
		boundary_messages2.clear();
		cc_map.clear();
		hasTask.clear();
	}else{
		if(isVisited)free(isVisited);
		isVisited = NULL;
		pthread_mutex_destroy(&cc_mutex);
	}

	pthread_mutex_destroy(&print_mutex);
	if(idValue)free(idValue);
	idValue = NULL;
}

Status WccDriver::init(const char* Dir, const char* outName, bool isMemory, bool isParallel){
	matrixMap = Matrixmap::load(Dir, outName, isMemory);
	unsigned size = 0;
	const char* startIDPath = string((string(Dir) + "/" + string(outName) + "/startID.0")).c_str();
	if (OSFile::FileExists(startIDPath)) {
		char* buf = NULL;
		size = HashID::loadFileinMemory(startIDPath, buf);
		memcpy(start_ID, buf, size);
		free(buf);
	} else {
		cerr << "startID file not exit!" << startIDPath << endl;
		assert(false);
	}
	size = size / sizeof(ID) - 1;
	bucSize = size;
	maxID = start_ID[size] - 1;
	this->isParallel = isParallel;
	cout<<"bucSize: "<<bucSize<<endl;
	//for(size_t i = 0;i <= size;i++)cout<<"start_ID["<<i<<"]"<<start_ID[i]<<endl;

	if (isMemory && isParallel) {
		for (size_t i = 0; i < bucSize/BATCH_PARTITION; i++) {
			boundary_messages1.push_back(new EntityIDBuffer());
			boundary_messages1[i]->setIDCount(2);
			boundary_messages1[i]->setSortKey(10);
			boundary_messages2.push_back(new EntityIDBuffer());
			boundary_messages2[i]->setIDCount(2);
			boundary_messages2[i]->setSortKey(10);
			pthread_mutex_init(&message_mutex[i], NULL);
			cc_map.push_back(new EntityIDBuffer());
			hasTask.push_back(false);
		}
	}

	if(!isParallel){
		isVisited = (char *)calloc(maxID + 1, sizeof(char));
		memset(isVisited, 0, (maxID + 1) * sizeof(char));
		pthread_mutex_init(&cc_mutex,NULL);
	}

	idValue = (ID *)calloc(maxID + 1, sizeof(ID));
	for(ID i = 0;i <= maxID;i++)idValue[i] = i;

	cout << "init complete!" << endl;
	return OK;
}

int WccDriver::getPartitionID(ID id){
	if (id > maxID || id < start_ID[0])
		return -1;

	unsigned index = 0;
	int begin = 0, mid = 0, end = bucSize - 1;
	while (begin < end) {
		mid = (begin + end) / 2;
		if (start_ID[mid] == id) {
			begin = mid;
			break;
		} else if (start_ID[mid] < id)
			begin = mid + 1;
		else
			end = mid;
	}

	if (start_ID[begin] > id)
		index = begin - 1;
	else
		index = begin;
	return index;
}

void WccDriver::exchange(){
	//cout<<"swap boundary_messages1 and boundary_messages2"<<endl;
	boundary_messages1.swap(boundary_messages2);
}

void WccDriver::wcc_in_parallel(unsigned partitionID, unsigned itera){
	//if(partitionID == 0)start_ID[partitionID] = 0;
	int len = start_ID[partitionID + BATCH_PARTITION] - start_ID[partitionID];
	char *visited = (char *)malloc(sizeof(char) * len);
	memset(visited, 0, len * sizeof(char));
	ID firstId = start_ID[partitionID];
	deque<ID>* que = new deque<ID>();
	ID component = 0;
	unsigned vecIndex = partitionID / BATCH_PARTITION;
	hasTask[vecIndex] = false;
	EntityIDBuffer* ent = new EntityIDBuffer();
	ent->setIDCount(1);

	if(itera == 0){
		cc_map[vecIndex]->setIDCount(2);
		cc_map[vecIndex]->setSortKey(10);//unsorted
		for (ID id = start_ID[partitionID];id < start_ID[partitionID + BATCH_PARTITION];id++) {
			if(visited[id-firstId])continue;
			visited[id-firstId] = 1;
			que->push_back(id);
			component = id;
			while(!que->empty()){
				ID curID = que->front();
				que->pop_front();

				matrixMap->getAllY(curID, ent);
				size_t entsize = ent->getSize();
				ID *p = ent->getBuffer();
				for (size_t i = 0; i < entsize; i++) {
					component = std::min(component, p[i]);
					if(p[i] < start_ID[partitionID] || p[i] >= start_ID[partitionID+BATCH_PARTITION])continue;
					if(visited[p[i]-firstId])continue;
					visited[p[i]-firstId] = 1;
					que->push_back(p[i]);
				}
			}

			cc_map[vecIndex]->insertID(id);
			cc_map[vecIndex]->insertID(component);
		}

		size_t cc_size = cc_map[vecIndex]->getSize();
		pthread_mutex_lock(&print_mutex);
		cout<<"cc_size = "<<cc_size<<endl;
		pthread_mutex_unlock(&print_mutex);
		hasTask[vecIndex] = true;
	}else if(itera == 1){
		ID *ptr = cc_map[vecIndex]->getBuffer();
		size_t cc_size = cc_map[vecIndex]->getSize();
		for(size_t index = 0;index < 2 * cc_size;index+=2){
		//int index = 1;
		//for (ID id = start_ID[partitionID];id < start_ID[partitionID + BATCH_PARTITION];id++) {
			//if(visited[id-firstId])continue;
			visited[ptr[index]-firstId] = 1;
			que->push_back(ptr[index]);
			component = ptr[index+1];
			idValue[ptr[index]] = component;
			//index += 2;
			//cout<<"iter "<<itera<<" , component : "<<component<<endl;

			while(!que->empty()){
				ID curID = que->front();
				que->pop_front();

				matrixMap->getAllY(curID, ent);
				size_t entsize = ent->getSize();
				ID *p = ent->getBuffer();
				for (size_t i = 0; i < entsize; i++) {
					if(p[i] < start_ID[partitionID] || p[i] >= start_ID[partitionID+BATCH_PARTITION]){
						//'p[i]' is a boundary vertex, find the partition that 'p[i]' belongs to
						if(idValue[p[i]] > component){
							unsigned parID = getPartitionID(p[i])/BATCH_PARTITION;
							//need locked
							pthread_mutex_lock(&message_mutex[parID]);
							boundary_messages1[parID]->insertID(p[i]);
							boundary_messages1[parID]->insertID(component);
							pthread_mutex_unlock(&message_mutex[parID]);
						}
						continue;
					}
					if(visited[p[i]-firstId])continue;
					visited[p[i]-firstId] = 1;
					que->push_back(p[i]);
					idValue[p[i]] = component;
				}
			}
		}
		hasTask[vecIndex] = true;
	}else{
		size_t msize = boundary_messages1[vecIndex]->getSize();
		/*pthread_mutex_lock(&print_mutex);
		cout<<"vecIndex = "<<vecIndex<<" , msize = "<<msize<<endl;
		pthread_mutex_unlock(&print_mutex);*/
		ID lastComponent = ID(-1);
		vector<ID> startID;

		if(msize){
			cc_map[vecIndex]->sort(2);
			ID *ptr = boundary_messages1[vecIndex]->getBuffer();
			map<ID,ID> scatterVertices;
			for(size_t j = 0;j < 2 * msize;j+=2){
				component = idValue[ptr[j]];
				if(ptr[j+1] < component){
					if(component == lastComponent){
						size_t vecSize = startID.size();
						for(size_t k = 0;k < vecSize;k++){
							scatterVertices[startID[k]] = std::min(scatterVertices[startID[k]], ptr[j+1]);
						}
					}
					else{
						cc_map[vecIndex]->findValuesByKey(component, startID);
						//cout<<"id: "<<ptr[j]<<" , component0: "<<ptr[j+1]<<" , startID: "<<startID<<", component: "<<component<<endl;
						size_t vecSize = startID.size();
						//cout<<"itera = "<<itera<<", vecSize = "<<vecSize<<endl;
						for(size_t k = 0;k < vecSize;k++){
							//assert(startID != ID(-1));
							if(scatterVertices.find(startID[k]) == scatterVertices.end()){
								scatterVertices[startID[k]] = ptr[j+1];
							}else{
								scatterVertices[startID[k]] = std::min(scatterVertices[startID[k]],ptr[j+1]);
							}
						}
						lastComponent = component;
					}
				}
			}
			boundary_messages1[vecIndex]->empty();
			cc_map[vecIndex]->sort(1);

			//cout<<"end of scatterVertices, scatterVertices.size() = "<<scatterVertices.size()<<endl;

			for(map<ID, ID>::iterator iter = scatterVertices.begin(), limit = scatterVertices.end();iter != limit;iter++){
				//if(iter->first < start_ID[partitionID] || iter->first >= start_ID[partitionID+BATCH_PARTITION])continue;
				assert(iter->first >= start_ID[partitionID] && iter->first < start_ID[partitionID+BATCH_PARTITION]);
				cc_map[vecIndex]->modifyValueByKey(iter->first, iter->second);
				visited[iter->first -firstId] = 1;
				que->push_back(iter->first);
				component = iter->second;
				//cout<<"iter "<<itera<<" , id : "<<iter->first<<" , component : "<<component<<endl;
				idValue[iter->first] = component;
				while(!que->empty()){
					ID curID = que->front();
					que->pop_front();

					matrixMap->getAllY(curID, ent);
					size_t entsize = ent->getSize();
					ID *p = ent->getBuffer();
					for (size_t i = 0; i < entsize; i++) {
						if(p[i] < start_ID[partitionID] || p[i] >= start_ID[partitionID+BATCH_PARTITION]){
							//'p[i]' is a boundary vertex, find the partition that 'p[i]' belongs to
							if(idValue[p[i]] > component){
								unsigned parID = getPartitionID(p[i])/BATCH_PARTITION;
								//need locked
								pthread_mutex_lock(&message_mutex[parID]);
								boundary_messages2[parID]->insertID(p[i]);
								boundary_messages2[parID]->insertID(component);
								pthread_mutex_unlock(&message_mutex[parID]);
							}
							continue;
						}
						if(visited[p[i]-firstId])continue;
						visited[p[i]-firstId] = 1;
						que->push_back(p[i]);
						idValue[p[i]] = component;
					}
				}
			}
			hasTask[vecIndex] = true;
		}
	}

	delete ent;
	delete que;
	free(visited);
}

bool WccDriver::taskAvailable(){
	bool taskAvail = false;
	for(unsigned i = 0;i < bucSize/BATCH_PARTITION;i++){
		taskAvail = taskAvail || hasTask[i];
	}

	return taskAvail;
}

void WccDriver::analysis_cc_map(map<ID, unsigned> &cc_info){
	for(unsigned i = 0;i < bucSize/BATCH_PARTITION;i++){
		ID *p = cc_map[i]->getBuffer();
		size_t entSize = cc_map[i]->getSize();
		for(size_t i = 1;i < 2 * entSize;i+=2){
			cc_info[p[i]] = 0;
		}
	}

	cout<<"cc_info.size(): "<<cc_info.size()<<endl;
}

void WccDriver::wcc_in_parallel(unsigned rootID, EntityIDBuffer *&cc_count){
	deque<ID>* que = new deque<ID>();
	EntityIDBuffer* ent = new EntityIDBuffer();
	ent->setIDCount(1);
	isVisited[rootID] = 1;
	que->push_back(rootID);
	ID component = rootID;
	unsigned count = 1;

	while(!que->empty()){
		ID curID = que->front();
		que->pop_front();

		matrixMap->getAllY(curID, ent, false);
		size_t entsize = ent->getSize();
		ID *p = ent->getBuffer();
		for (size_t i = 0; i < entsize; i++) {
			if(isVisited[p[i]])continue;
			isVisited[p[i]] = 1;
			que->push_back(p[i]);
			idValue[p[i]] = component;
			count++;
		}
	}

	pthread_mutex_lock(&cc_mutex);
	//cc_count.push_back(make_pair<ID, unsigned>(rootID, count));
	cc_count->insertID(rootID);
	cc_count->insertID(count);
	pthread_mutex_unlock(&cc_mutex);
	delete ent;
	delete que;
}

void WccDriver::wcc_in_serial(map<ID, pair<ID, unsigned> > &cc_info){
	deque<ID>* que = new deque<ID>();
	ID component = 0;
	unsigned count = 0;
	//map<ID, pair<ID, unsigned> > cc_info;//start_id, component, count
	EntityIDBuffer* ent = new EntityIDBuffer();
	ent->setIDCount(1);

	memset(isVisited, 0, (maxID + 1) * sizeof(char));
	for (ID id = 0; id <= maxID; id++) {
		if(isVisited[id])continue;
		isVisited[id] = 1;
		que->push_back(id);
		component = id;
		count = 1;
		while(!que->empty()){
			ID curID = que->front();
			que->pop_front();

			matrixMap->getAllY(curID, ent, false);
			size_t entsize = ent->getSize();
			ID *p = ent->getBuffer();
			for (size_t i = 0; i < entsize; i++) {
				if(isVisited[p[i]])continue;
				isVisited[p[i]] = 1;
				que->push_back(p[i]);
				component = std::min(component, p[i]);
				count++;
			}
		}
		//cout<<"iter 0 , component : "<<component<<endl;
		cc_info[id] = make_pair<ID, unsigned>(component, count);
	}

	cout<<"cc_info.size() = "<<cc_info.size()<<endl;

	memset(isVisited, 0, (maxID + 1) * sizeof(char));
	for (ID id = 0; id <= maxID; id++) {
		if(isVisited[id])continue;
		isVisited[id] = 1;
		que->push_back(id);
		map<ID,pair<ID, unsigned> >::iterator val = cc_info.find(id);
		component = ((*val).second).first;
		//cout<<"iter 1 , component : "<<component<<endl;
		idValue[id] = component;
		while(!que->empty()){
			ID curID = que->front();
			que->pop_front();

			matrixMap->getAllY(curID, ent, false);
			size_t entsize = ent->getSize();
			ID *p = ent->getBuffer();
			for (size_t i = 0; i < entsize; i++) {
				if(isVisited[p[i]])continue;
				isVisited[p[i]] = 1;
				que->push_back(p[i]);
				idValue[p[i]] = component;
			}
		}
	}

	delete ent;
	delete que;
}
