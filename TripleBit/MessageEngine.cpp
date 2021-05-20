/*
 * MessageEngine.cpp
 *
 *  Created on: Sep 20, 2010
 *      Author: root
 */

#include "MessageEngine.h"
#include "TripleBit.h"

MessageEngine::MessageEngine() {
	// TODO Auto-generated constructor stub

}

MessageEngine::~MessageEngine() {
	// TODO Auto-generated destructor stub
}

void MessageEngine::showMessage(char* msg, MessageType type /* = DEFAULT*/)
{
	switch(type)
	{
	case INFO:
		fprintf(stderr, "\033[0;32mINFO: %s\033[0m.\n",msg);
		break;
	case WARNING:
		fprintf(stderr, "\033[1;33mWARNING: %s\033[0m.\n",msg);
		break;
	case ERR:
		fprintf(stderr, "\033[0;31mERROR: %s\033[0m.\n",msg);
		break;
	default:
		printf("%s\n", msg);
	}
}
