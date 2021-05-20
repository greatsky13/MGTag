#include "ServerInterface.h"
#include "TripleBitRepository.h"

#include <stdio.h>
#include <stdlib.h>

#pragma comment (lib, "WSock32.lib")

int main(int argc, char* argv[])
{
	if(argc != 3) {
		printf("Usage: %s <repo path> <serverport>\n", argv[0]);
		exit(-1);
	}

	short port = atoi(argv[2]);

	TripleBitRepository * repo = TripleBitRepository::create(argv[1]);
	ServerInterface * server = new ServerInterface(repo, port);
	server->run();

	delete server;
}