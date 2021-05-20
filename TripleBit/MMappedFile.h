#ifndef _MMAPPED_FILE_H_
#define _MMAPPED_FILE_H_

#include "OSFile.h"
#include <sys/mman.h>

class MMappedFile : public OSFile
{
protected:
#ifdef TRIPLEBIT_WINDOWS
	void*			md;
#endif
	size_t			init_size;
	size_t			mmap_size;
	char *			mmap_addr;

public:
	MMappedFile(const char* name, size_t init_size);
	char*	get_address() {return mmap_addr;}

	virtual ~MMappedFile(void);
	Status	flush();
	Status	open(AccessMode mode, int flags = 0);
	Status	close();
	Status	get_size(size_t& size) const;
	Status	set_size(size_t new_size);
};

#endif
