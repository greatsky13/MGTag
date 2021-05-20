/*
 * PageIndex.h
 *
 *  Created on: 2012-9-17
 *      Author: yaya
 */

#ifndef PAGEINDEX_H_
#define PAGEINDEX_H_

#include "MemoryBuffer.h"
#include "MMapBuffer.h"
#include "Matrixmap.h"
#include "EntityIDBuffer.h"

struct Point {
	ID x;
	size_t y;
};
class PageIndex {
public:

private:
	MMapBuffer* idOffsetTable;
	Point* idTableEntries;
//	char fileName[100];
	unsigned tableSize;
	BucketManager& manager;
public:
	PageIndex(BucketManager&,char *);
	PageIndex(BucketManager&);
	virtual ~PageIndex();
	void insertEntries(ID id, size_t offset);
	void endInsert();
	int searchChunk(ID id);
	int getOffsetByID(ID id, size_t& offset);
	Status getYByID(ID id,EntityIDBuffer* entBuffer);
	static PageIndex* load(BucketManager& manager,char * fileName);
	static PageIndex* loadMemory(BucketManager& manager,char * fileName);
	void flush();
	Point* getidOffsetTable(){return idTableEntries;}
	void setidOffsetTable(Point *entries){ idTableEntries = entries; }
	unsigned getTableSize(){ return tableSize; }
	void setTableSize(unsigned size){ tableSize = size; }
};

#endif /* PAGEINDEX_H_ */
