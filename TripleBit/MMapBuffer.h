/*
 * MMapBuffer.h
 *
 *  Created on: Oct 6, 2010
 *      Author: root
 */

#ifndef MMAPBUFFER_H_
#define MMAPBUFFER_H_

#include "TripleBit.h"
//#include "MMappedFile.h"
class MMappedFile;

class MMapBuffer {
protected:
	MMappedFile* mfile;
	size_t length;
	char * address;
public:
	char * get_address() const { return (char*)address; }
    char* getBuffer();
    char* getBuffer(size_t pos);

	Status resize(size_t new_size,bool clear);
	size_t get_length() {return length;}
	void   memset(char value);
	Status   flush();
	Status close();

	MMapBuffer(const char* filename, size_t initSize);
	virtual ~MMapBuffer();
	Status remove();
    const char* getName();
public:
	static MMapBuffer* create(const char* filename, size_t initSize);
};

#endif /* MMAPBUFFER_H_ */
