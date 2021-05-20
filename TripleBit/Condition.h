#ifndef _CONDITION_H_
#define _CONDITION_H_

#include "TripleBit.h"
#include "MutexLock.h"

class Condition
{
#ifdef TRIPLEBIT_WINDOWS
	CONDITION_VARIABLE cond;
#else
	pthread_cond_t cond;
#endif

public:
	Condition(void);
	~Condition(void);
	void wait(MutexLock& lock);
	void notifyAll();
};
#endif
