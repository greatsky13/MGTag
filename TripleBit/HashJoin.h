/*
 * HashJoin.h
 *
 *  Created on: Jul 14, 2010
 *      Author: root
 */

#ifndef HASHJOIN_H_
#define HASHJOIN_H_

#include "TripleBit.h"
#include "ThreadPool.h"

class EntityIDBuffer;
class HashJoinTask;
class BuildHashIndexTask;

struct HashJoinArg;

class HashJoin {
private:
	//CThreadPool* pool;
public:
	friend class HashJoinTask;
	HashJoin();
	virtual ~HashJoin();

	static ID HashFunction(ID id, ID hashKey);
	static ID GetHashKey2(ID size);
	void Join(EntityIDBuffer* entBuffer1, EntityIDBuffer* entBuffer2, int joinKey1, int joinKey2);
	static ID GetHashKey(ID size);
	static void BuildHashIndex(ID* p ,int joinKey, ID hashKey, vector<int>& hashTrack, vector<int>& prefixSum, int size, int IDCount);
	static void HashJoinInit(EntityIDBuffer* buffer,ID& hashKey, vector<vector<int>* >& hashTrack, int joinKey);
	static void SortMergeJoin(ID* buffer1, ID * buffer2, int IDCount1, int IDCount2, int joinKey1, int joinKey2,
			int size1, int size2, char* flagVector1, char* flagVector2);
	static void SortMergeJoin(ID* buffer1, ID * buffer2, int IDCount1, int IDCount2, int joinKey1, int joinKey2,
			int size1, int size2, char* flagVector1);
	static void run(HashJoinArg* arg);
};

struct HashJoinArg
{
	char* flag1;
	char* flag2;
	EntityIDBuffer* buffer1;
	EntityIDBuffer* buffer2;

	//the start pos in the buffer;
	int startPos1;
	int length1;

	int startPos2;
	int length2;

	int joinKey1;
	int joinKey2;
};

#endif /* HASHJOIN_H_ */
