/*
 * SynchronousBuffer.h
 *
 *  Created on: Apr 26, 2010
 *      Author: root
 *
 * 	Be used to in the pipeline which need to control the memory writing and reading
 */

#ifndef SYNCHRONOUSBUFFER_H_
#define SYNCHRONOUSBUFFER_H_

#define SYNCHRONOUSBUFFER_DEBUG 1

class MemoryBuffer;

#include "TripleBit.h"
#include "MutexLock.h"
#include "Condition.h"

class SynchronousBuffer {
private:
	MemoryBuffer* buffer;

	//for synchronous access the buffer
	//pthread_mutex_t bufferLock;
	//pthread_cond_t  bufferNotEmpty;
	//pthread_cond_t  bufferFull;
	//pthread_cond_t  bufferNotFull;
	MutexLock bufferLock;
	Condition bufferNotEmpty;
	Condition bufferFull;
	Condition bufferNotFull;

	char* base;
	unsigned int readPos;
	unsigned int writePos;

	unsigned int pageSize;
	unsigned int usedSize;
	unsigned int remainderSize;

	bool finish; //used to identify whether writing is finished;
public:
	SynchronousBuffer();
	Status 	MemoryCopy(void* src, size_t length);
	Status 	MemoryGet(void* dest, size_t length);
	void	SetFinish() { finish = true; }
	virtual ~SynchronousBuffer();

private:
	bool 	IsBufferFull(size_t length) { return (writePos + length) % pageSize == readPos; }
	bool 	IsBufferEmpty() { return (readPos == writePos); }
};

#endif /* SYNCHRONOUSBUFFER_H_ */
