/*
 * WccDriver.h
 *
 *  Created on: Mar 18, 2014
 *      Author: root
 */

#ifndef WCCDRIVER_H_
#define WCCDRIVER_H_

#include "Matrixmap.h"
#include "BuildAdjGraph.h"

#define BATCH_PARTITION 1

class WccDriver {
public:
	WccDriver();
	virtual ~WccDriver();
	Status init(const char* Dir, const char* outName, bool isMemory, bool isParallel);
	void wcc_in_parallel(unsigned partitionID, unsigned itera);
	void wcc_in_parallel(unsigned rootID, EntityIDBuffer* &cc_count);
	void wcc_in_serial(map<ID, pair<ID, unsigned> > &cc_info);
	unsigned getMaxID(){ return maxID; }
	unsigned getBucNum(){ return bucSize; }
	bool taskAvailable();
	ID* getValue(){ return idValue; }
	void analysis_cc_map(map<ID, unsigned> &cc_count);
	void exchange();
private:
	Matrixmap *matrixMap;
	ID start_ID[100];
	ID *idValue;
	ID maxID;
	unsigned bucSize;
	bool isParallel;
	char *isVisited;
	vector<EntityIDBuffer *> boundary_messages1;
	vector<EntityIDBuffer *> boundary_messages2;
	pthread_mutex_t message_mutex[96/BATCH_PARTITION];
	pthread_mutex_t print_mutex;
	pthread_mutex_t cc_mutex;
	vector<EntityIDBuffer *> cc_map;
	vector<bool> hasTask;

	void sync();
	int getPartitionID(ID id);
};

#endif /* WCCDRIVER_H_ */
