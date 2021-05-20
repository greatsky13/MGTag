/*
 * OSFile.h
 *
 *  Created on: Oct 8, 2010
 *      Author: root
 */

#ifndef OSFILE_H_
#define OSFILE_H_

#include "TripleBit.h"

class OSFile {
public:
	enum AccessMode {F_READ = 0, F_WRITE = 1, F_READWRITE = 2};
	enum OpenMode {
		FO_TRUNCATE = 0X01,
		FO_CREATE	= 0X02,
		FO_SYNC		= 0X04,
		FO_RANDOM	= 0X08
	};
	OSFile(const char * filename = 0);
	~OSFile();

	void    get_error_text(Status code, char* buf, size_t buf_size) const;

	Status  read(size_t pos, void* buf, size_t size);
	Status  write(size_t pos, void const* buf, size_t size);

	Status  set_position(size_t pos);
	Status  get_position(size_t& pos);

	Status  read(void* buf, size_t size);
	Status  write(void const* buf, size_t size);

	Status  flush();

	Status  open(AccessMode mode, int flags=0);
	Status  close();
	Status  remove();

	char const* get_name() const;
	Status  set_name(char const* new_name);

	Status  get_size(size_t & size) const;
	Status  set_size(size_t new_size);

	static bool FileExists(const char * path);
	static bool DirectoryExists(const char * path);
	static bool MkDir(const char * path);
	static bool FileDelete(const char * path);
	static size_t FileSize(const char * path);

protected:
	static Status get_system_error();

protected:
	AccessMode		mode;
	int				flags;
	bool			opened;
	char			name[256];

#ifdef TRIPLEBIT_UNIX
	int				fd;
#else
	void*			fd;
#endif
};

#endif /* OSFILE_H_ */
