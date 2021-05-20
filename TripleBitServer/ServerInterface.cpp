#include "ServerInterface.h"
#include "TripleBitRepository.h"
#include "MemoryBuffer.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef TRIPLEBIT_UNIX
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>

struct command_listener_param {
	int fd;
	sockaddr client_addr;
	TripleBitRepository * repo;
};

ServerInterface::ServerInterface(TripleBitRepository* repo, short port) : repo(repo), port(port)
{

}


ServerInterface::~ServerInterface(void)
{
	int & listen_sock = *((int*) listen_socket_buf);
	close(listen_sock);
}


void ServerInterface::run()
{
	int & listen_sock = *((int*) listen_socket_buf);
	listen_sock = socket(AF_INET, SOCK_STREAM, 0);

	unsigned value = 0x01;
	setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, (void *) &value, sizeof(value));

	sockaddr_in server_addr, client_addr;
	socklen_t slen;
	bzero((char *) &server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(port);
	bind(listen_sock, (sockaddr *) &server_addr, sizeof(server_addr));
	listen(listen_sock, 8);
	for (;;) {
		slen = sizeof(client_addr);
		int connfd = accept(listen_sock, (sockaddr *) &client_addr, &slen);
		if (connfd == -1) {
			printf("Error while accepting");
			break;
		}
		char *str = inet_ntoa(client_addr.sin_addr);
		printf("Connecting from %s\n", str);
		command_listener_param * param = new command_listener_param();
		printf("socket fd: %d\n", connfd);
		param->fd = connfd;
		param->repo = repo;
		memcpy(&param->client_addr, &client_addr, slen);
		pthread_t tid;
		if (pthread_create(&tid, NULL, command_listener, param) != 0)
			printf("Create service thread error!\n");
	}
}

static bool repeated_read(int fd,char * buff,int offset,int length){
	int trd = 0;
	while(trd<length){
		ssize_t rd = read(fd,buff+offset+trd,length-trd);
		if(rd<0)
			return false;
		trd+=rd;
	}
	return trd;
}

static bool repeated_write(int fd,char * buff,int offset,int length){
	int trd = 0;
	while(trd<length){
		ssize_t rd = write(fd,buff+offset+trd,length-trd);
		if(rd<0)
			return false;
		trd+=rd;
	}
	return trd;
}

THREADRET THREADCALL ServerInterface::command_listener(void* args)
{
	command_listener_param * param = (command_listener_param *) args;
	printf("listen fd: %d\n", param->fd);
	uint cmdsize;
	MemoryBuffer * cmdbuffer = new MemoryBuffer(4 * 1024);

	for (;;) {
		// receive the query string.
		if (!repeated_read(param->fd, (char *) (&cmdsize), 0, sizeof(uint))) {
			printf("Read command length error.\n");
			break;
		}

		if (cmdbuffer->get_length() < cmdsize)
			cmdbuffer->resize(cmdsize);
		if (!repeated_read(param->fd, cmdbuffer->get_address(), 0, cmdsize)) {
			printf("Read command error, length %d.\n", cmdsize);
		}

		// TODO execute the query
		string query(cmdbuffer->get_address(), cmdsize);
		if(query.compare("exit") == 0)
			break;

		param->repo->execute(query);

		cout<<"execute over"<<endl;

		// TODO send the result;
		TripleBitRepository::result_iterator iter = param->repo->get_result_begin();
		TripleBitRepository::result_iterator limit = param->repo->get_result_end();

		int writepos = 4;
		while(iter != limit) {
			cout<<*iter<<endl;
			if(writepos + iter->length() > cmdbuffer->get_length()) {
				cmdbuffer->resize(1024);
			}
			strcpy(cmdbuffer->get_address() + writepos, iter->c_str());
			writepos += iter->length();
			*(cmdbuffer->get_address() + writepos) = '\0';
			writepos++;

			iter++;
		}
		*((uint*)cmdbuffer->get_address()) = writepos - 4;

		int totallen = writepos;
		if (!repeated_write(param->fd, cmdbuffer->get_address(), 0, totallen)) {
			printf("Written ResultSet error, length %d\n", totallen);
			break;
		}
	}

	printf("Listener thread exited\n");
	close(param->fd);
	delete cmdbuffer;
	delete param;

	return NULL;
}

#else

#include <WinSock.h>
#include <stdio.h>

struct command_listener_param {
	SOCKET client_socket;
	sockaddr client_addr;
	TripleBitRepository * repo;
};

ServerInterface::ServerInterface(TripleBitRepository* repo, short port) : repo(repo), port(port)
{
	
}

ServerInterface::~ServerInterface()
{
	SOCKET & listen_sock = *((SOCKET*) listen_socket_buf);
	closesocket(listen_sock);
}

void ServerInterface::run() 
{
	WSADATA ws;

	if(WSAStartup(MAKEWORD(2, 2), &ws) != 0) {
		printf("Init windows socket failed: %d\n", GetLastError());
		return;
	}
	
	SOCKET & listen_sock = *((SOCKET*) listen_socket_buf);
	listen_sock = socket(AF_INET, SOCK_STREAM, 0);

	unsigned value = 0x01;
	setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, (char*) &value, sizeof(value));

	sockaddr_in server_addr, client_addr;
	int slen;
	memset((char *) &server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(port);
	::bind(listen_sock, (sockaddr *) &server_addr, sizeof(server_addr));
	listen(listen_sock, 8);
	
	for (;;) {
		slen = sizeof(client_addr);
		SOCKET connfd = accept(listen_sock, (sockaddr *) &client_addr, &slen);
		if (connfd == -1) {
			printf("Error while accepting: %d\n", GetLastError());
			break;
		}
		char *str = inet_ntoa(client_addr.sin_addr);
		printf("Connecting from %s\n", str);
		command_listener_param * param = new command_listener_param;
		printf("socket fd: %d\n", connfd);
		param->client_socket = connfd;
		param->repo = repo;
		memcpy(&param->client_addr, &client_addr, slen);
		HANDLE hThread;
		if ((hThread = (NULL, 0, command_listener, (LPVOID)param, 0, NULL)) == NULL) {
			printf("Create service thread error!\n");
		}
	}
	
}

static bool repeated_read(SOCKET socket,char * buff,int offset,int length){
	int trd = 0;
	while(trd<length){
		int rd = recv(socket,buff+offset+trd,length-trd,0);
		if(rd<0)
			return false;
		trd+=rd;
	}
	return trd;
}

static bool repeated_write(SOCKET socket,char * buff,int offset,int length){
	int trd = 0;
	while(trd<length){
		int rd = send(socket,buff+offset+trd,length-trd,0);
		if(rd<0)
			return false;
		trd+=rd;
	}
	return trd;
}

THREADRET THREADCALL ServerInterface::command_listener(void* args)
{
	command_listener_param * param = (command_listener_param *) args;
	printf("listen fd: %d\n", param->client_socket);
	uint cmdsize;
	MemoryBuffer * cmdbuffer = new MemoryBuffer(4 * 1024);

	for (;;) {
		// receive the query string.
		if (!repeated_read(param->client_socket, (char *) (&cmdsize), 0, sizeof(uint))) {
			printf("Read command length error.\n");
			break;
		}

		if (cmdbuffer->get_length() < cmdsize)
			cmdbuffer->resize(cmdsize);
		if (!repeated_read(param->client_socket, cmdbuffer->get_address(), 0, cmdsize)) {
			printf("Read command error, length %d.\n", cmdsize);
		}

		// TODO execute the query
		string query(cmdbuffer->get_address(), cmdsize);
		if(query.compare("exit") == 0)
			break;

		param->repo->execute(query);

		cout<<"execute over"<<endl;

		// TODO send the result;
		TripleBitRepository::result_iterator iter = param->repo->get_result_begin();
		TripleBitRepository::result_iterator limit = param->repo->get_result_end();

		int writepos = 4;
		while(iter != limit) {
			if(writepos + iter->length() > cmdbuffer->get_length()) {
				cmdbuffer->resize(1024);
			}
			strcpy(cmdbuffer->get_address() + writepos, iter->c_str());
			writepos += iter->length();
			*(cmdbuffer->get_address() + writepos) = '\0';
			writepos++;

			iter++;
		}
		*((uint*)cmdbuffer->get_address()) = writepos - 4;

		int totallen = writepos;
		if (!repeated_write(param->client_socket, cmdbuffer->get_address(), 0, totallen)) {
			printf("Written ResultSet error, length %d\n", totallen);
			break;
		}
	}

	printf("Listener thread exited\n");
	closesocket(param->client_socket);
	delete cmdbuffer;
	delete param;
	
	return 0;	
}

#endif
