#ifndef __BPLUS_INDEX_H__
#define __BPLUS_INDEX_H__

class MMapBuffer;
class ChunkManager;
class MemoryBuffer;

#include "TripleBit.h"

class BPlusIndex {
public:
	enum IndexType {SUBJECT_INDEX, OBJECT_INDEX};
private:
	ChunkManager& manager;
	MemoryBuffer* indexBuffer;
	size_t indexSize;

	unsigned writerIndex;
	unsigned currentIndex;
	unsigned recordCount;
	unsigned pageCount;

	ID* indexEntry;
	IndexType type;
public:
	BPlusIndex(ChunkManager& _manager, IndexType indexType);
	Status buildIndex(char chunkType);
	Status getOffsetByID(ID id, unsigned &offset, unsigned typeId);
	void save(MMapBuffer* & buffer);
	~BPlusIndex();

private:
	int searchChunk(ID id, unsigned& start, unsigned& end);
	void insertEntry(ID id, unsigned offset);
	void packInnerNode();
public:
	static BPlusIndex* load(ChunkManager& manager, IndexType type, char* buffer, size_t& offset);
};

#endif /* __BPLUS_INDEX_H__ */
