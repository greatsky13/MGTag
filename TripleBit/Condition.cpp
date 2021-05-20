#include "Condition.h"

Condition::Condition(void)
{
#ifdef TRIPLEBIT_WINDOWS
	InitializeConditionVariable(&cond);
#else
	pthread_cond_init(&cond,NULL);
#endif
}


Condition::~Condition(void)
{
#ifdef TRIPLEBIT_WINDOWS
#else
	pthread_cond_destroy(&cond);
#endif
}

void Condition::wait(MutexLock& lock)
{
#ifdef TRIPLEBIT_WINDOWS
	SleepConditionVariableCS(&cond,&lock.getMutex(),INFINITE); 
#else
	pthread_cond_wait(&cond, &lock.getMutex());
#endif
}

void Condition::notifyAll()
{
#ifdef TRIPLEBIT_WINDOWS
	WakeConditionVariable(&cond);
#else
	pthread_cond_broadcast(&cond);
#endif
}
