/*
 * LineHashIndex.h
 *
 *  Created on: Nov 15, 2010
 *      Author: root
 */

#ifndef LINEHASHINDEX_H_
#define LINEHASHINDEX_H_

class MemoryBuffer;
class ChunkManager;
class MMapBuffer;

#include "TripleBit.h"
#include "EntityIDBuffer.h"

class LineHashIndex {
public:
	struct Point{
		ID x;
		ID y;
	};

	enum IndexType { SUBJECT_INDEX, OBJECT_INDEX};
private:
	MemoryBuffer* idTable;
	MemoryBuffer* offsetTable;
	ID* idTableEntries;
	ID* offsetTableEntries;
	ChunkManager& chunkManager;
	IndexType indexType;
	unsigned int tableSize;
	unsigned lineNo;

	//line parameters;
	double upperk[4];
	double upperb[4];
	double lowerk[4];
	double lowerb[4];

	ID startID[4];
private:

	void insertEntries(ID id, size_t offset);
	int searchChunk(ID id);
	bool buildLine(int startEntry, int endEntry, int lineNo);
public:
	static string dir;
	LineHashIndex(ChunkManager& _chunkManager, IndexType type);
	Status buildIndex(unsigned chunkType);
	Status getOffsetByID(ID id, size_t & offset, unsigned typeID);
	Status getFirstOffsetByID(ID id, size_t& offset, unsigned typeID);
	Status getYByID(ID id,EntityIDBuffer* entBuffer,unsigned typeID);
	void save(MMapBuffer*& indexBuffer);
	virtual ~LineHashIndex();
private:
	bool isBufferFull();
public:
	static LineHashIndex* load(ChunkManager& manager, IndexType type, char* buffer, size_t& offset);
	static void unload( char* buffer, size_t& offset);
};

#endif /* LINEHASHINDEX_H_ */
