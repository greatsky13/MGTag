#ifndef __SERVERINTERFACE_H__
#define __SERVERINTERFACE_H__

class TripleBitRepository;

#include "ThreadPool.h"

class ServerInterface
{
	TripleBitRepository * repo;
	short port;
	char listen_socket_buf[8];
public:
	ServerInterface(TripleBitRepository* repo, short port);
	~ServerInterface();

	void run();

private:
	static THREADRET THREADCALL command_listener(void* args);
};

#endif

