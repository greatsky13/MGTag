#ifndef _MUTEXLOCK_H_
#define _MUTEXLOCK_H_

#include "TripleBit.h"

#ifdef TRIPLEBIT_WINDOWS
#include <windows.h>
#else
#include <pthread.h>
#endif

//#include <iostream>

class MutexLock
{
#ifdef TRIPLEBIT_WINDOWS
	CRITICAL_SECTION mutex;
#else
	pthread_mutex_t mutex;
#endif
public:
	MutexLock(void);

	~MutexLock() {
//		std::cout<<"delete lock"<<std::endl;		

#ifdef TRIPLEBIT_WINDOWS
		DeleteCriticalSection(&mutex);
#else
		pthread_mutex_destroy(&mutex);
#endif
	}

	void init() {
#ifdef TRIPLEBIT_WINDOWS
		InitializeCriticalSection(&mutex);
#else
		pthread_mutex_init(&mutex,NULL);
#endif
	}

	void lock();

	void unLock();

	bool tryLock();

#ifdef TRIPLEBIT_WINDOWS
	CRITICAL_SECTION& getMutex() {
		return mutex;
	}
#else
	pthread_mutex_t& getMutex() {
		return mutex;
	}

#endif
};

#endif
