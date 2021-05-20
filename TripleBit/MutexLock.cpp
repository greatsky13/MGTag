#include "MutexLock.h"

MutexLock::MutexLock(void) 
{
}


void MutexLock::lock()
{
#ifdef TRIPLEBIT_WINDOWS
	EnterCriticalSection(&mutex);
#else
	pthread_mutex_lock(&mutex);
#endif
}

bool MutexLock::tryLock()
{
#ifdef TRIPLEBIT_WINDOWS
	return TryEnterCriticalSection(&mutex);
#else
	return pthread_mutex_trylock(&mutex) ==0 ;
#endif
}

void MutexLock::unLock()
{
#ifdef TRIPLEBIT_WINDOWS
	LeaveCriticalSection(&mutex);
#else
	pthread_mutex_unlock(&mutex);
#endif
}
