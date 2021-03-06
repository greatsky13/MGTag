/*
 * MMapBuffer.cpp
 *
 *  Created on: Oct 6, 2010
 *      Author: root
 */

#include "MMapBuffer.h"
#include "MMappedFile.h"

MMapBuffer::MMapBuffer(const char* _filename, size_t initSize) {
	mfile = new MMappedFile(_filename, initSize);

	if(mfile->open(OSFile::F_READWRITE, OSFile::FO_CREATE | OSFile::FO_RANDOM) == OK ) {
        address = mfile->get_address();
		mfile->get_size(length);
	} else {
		address = 0;
		length = 0;
	}
}

MMapBuffer::~MMapBuffer() {
	// TODO Auto-generated destructor stub
	mfile->close();
    delete mfile;
	address = 0;
	length = 0;
}

Status MMapBuffer::flush()
{
	return mfile->flush();
}

Status MMapBuffer::close()
{
	return mfile->close();
}
const char* MMapBuffer::getName(){
    return mfile->get_name();
}

Status MMapBuffer::resize(size_t new_size, bool clear)
{
	Status ret = mfile->set_size(new_size);
	if(ret == OK) {
		address = mfile->get_address();
		mfile->get_size(length);
	} 

	return ret;
}

void MMapBuffer::memset(char value)
{
	::memset((char*)address, value, length);
}

MMapBuffer* MMapBuffer::create(const char* filename, size_t initSize)
{
	MMapBuffer* buffer = new MMapBuffer(filename, initSize);
	return buffer;
}

Status MMapBuffer::remove(){
	mfile->close();
	return mfile->remove();
}

char* MMapBuffer::getBuffer() {
        return (char*)address;
}

char* MMapBuffer::getBuffer(size_t pos) {
        return (char*)address + pos;
}

