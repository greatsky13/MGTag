/*
 * BufferManager.cpp
 *
 *  Created on: 2011-4-8
 *      Author: liupu
 */

#include "BufferManager.h"
#include "EntityIDBuffer.h"

BufferManager* BufferManager::instance = NULL;

BufferManager::BufferManager()
{
	// TODO Auto-generated constructor stub
	for (int i = 0; i < INIT_BUFFERS; i++) {
		EntityIDBuffer* buffer = new EntityIDBuffer;
		bufferPool.push_back(buffer);
		cleanBuffer.push_back(buffer);
	}
	usedBuffer.clear();
	bufferCnt = INIT_BUFFERS;
}

BufferManager::~BufferManager()
{
	// TODO Auto-generated destructor stub
}

bool BufferManager::expandBuffer()
{
	for (int i = 0; i < INCREASE_BUFFERS; i++) {
		EntityIDBuffer* buffer = new EntityIDBuffer;
		if (buffer == NULL) {
			bufferCnt += i;
			return false;
		}
		bufferPool.push_back(buffer);
		cleanBuffer.push_back(buffer);
	}

	bufferCnt += INCREASE_BUFFERS;
	return true;
}

EntityIDBuffer* BufferManager::getNewBuffer()
{
	if (usedBuffer.size() == bufferPool.size()) {
		if (expandBuffer() == false)
			return NULL;
	}
	EntityIDBuffer* buffer = cleanBuffer.front();
	cleanBuffer.erase(cleanBuffer.begin());;
	usedBuffer.push_back(buffer);

	return buffer;
}

void BufferManager::destroyBuffers()
{
	for (size_t i = 0; i < bufferPool.size(); i++) {
		delete bufferPool[i];
	}

	usedBuffer.clear();
	cleanBuffer.clear();
}

Status BufferManager::freeBuffer(EntityIDBuffer* buffer)
{
	vector<EntityIDBuffer*>::iterator iter;
	iter = find(usedBuffer.begin(), usedBuffer.end(), buffer);
	if (iter != usedBuffer.end()) {
		usedBuffer.erase(iter);
		cleanBuffer.push_back(buffer);
		//(*iter)->empty();
		buffer->empty();
		return OK;
	} else {
		return NOT_FOUND;
	}
}

Status BufferManager::reserveBuffer()
{
	usedBuffer.clear();
	cleanBuffer.clear();
	int i;
	for (i = 0; i < INIT_BUFFERS; i++) {
		bufferPool[i]->empty();
		cleanBuffer.push_back(bufferPool[i]);
	}

	vector<EntityIDBuffer*>::iterator iter = bufferPool.begin() + i;
	for (; iter != bufferPool.end(); iter++) {
		delete *iter;
		bufferPool.erase(iter);
	}

	return OK;
}

BufferManager* BufferManager::getInstance()
{
	if (instance == NULL) {
		instance = new BufferManager;
	}

	return instance;
}
