/*
 * ThreadPool.h
 *
 *  Created on: 2010-6-18
 *      Author: liupu
 */

#ifndef THREADPOOL_H_
#define THREADPOOL_H_

#include "TripleBit.h"
#include "MutexLock.h"
#include "Condition.h"

#include <vector>
#include <string>
#include <iostream>
#include <map>
#include <boost/function.hpp>

#if (defined(TRIPLEBIT_WINDOWS))
#include <process.h>
#include <windows.h>
#else
#include <pthread.h>
#endif

using namespace std;

#if (defined(TRIPLEBIT_WINDOWS))
typedef DWORD THREADID;
typedef DWORD THREADRET;
#define THREADCALL WINAPI
#else
typedef pthread_t THREADID;
typedef void* THREADRET;
#define THREADCALL
#endif

class CThreadPool {
public:
	typedef boost::function<void ()> Task;
private:
	vector<Task> m_vecTaskList; //task list
	int m_iThreadNum; //the No of threads
	vector<THREADID> m_vecIdleThread; //idle thread list
	vector<THREADID> m_vecBusyThread; //busy thread list


	MutexLock threadMutex;
	MutexLock threadIdleMutex;
	MutexLock threadBusyMutex;

	Condition threadCond;
	Condition threadEmptyCond;
	Condition threadBusyEmptyCond;

	static CThreadPool* instance;				//the thread pool instance;
protected:
	friend class CTask;
	static THREADRET THREADCALL ThreadFunc(void * threadData); //new thread function
	int MoveToIdle(THREADID tid); //move the idle when the task complete
	int MoveToBusy(THREADID tid); //move the tid to busy list
	int Create(); //create task
public:
	static CThreadPool& getInstance();

	bool shutdown;
	CThreadPool(int threadNum);
	~CThreadPool();
	int AddTask(const Task& task);	// Add the task to List
	int StopAll();
	int Wait();				 //waiting for task complete!
};

struct ThreadPoolArg
{
	CThreadPool* pool;
	vector<CThreadPool::Task>* taskList;
};

#endif /* THREADPOOL_H_ */
