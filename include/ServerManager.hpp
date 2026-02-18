#pragma once

#include "EpollManager.hpp"
#include "Server.hpp"
#include <map>
#include <string>
#include <vector>

enum ReadState
{
	READING,
	READ_COMPLETE,
	ERROR
};

struct	ClientData
{
	// Client information
	int clientSocket;
	int	serverSocket;
	struct sockaddr_in client_addr;

	// Client buffers
	std::string readBuffer;
	ReadState readState;
	long contentLength;

	ClientData()
	: 
		clientSocket(-1),
		serverSocket(-1),
		client_addr(),
		readBuffer(""),
		readState(READING),
		contentLength(-1)
	{}

	ClientData(const struct sockaddr_in &addr, int clientSocket, int serverSocket)
	: 
		clientSocket(clientSocket),
		serverSocket(serverSocket),
		client_addr(addr),
		readBuffer(""),
		readState(READING),
		contentLength(-1)
	{}
};

class ServerManager
{
  private:
	Logger& _logger;
	EpollManager& _epoll;
	std::vector<Server *> _servers;
	std::map<int, ClientData> _clientMap;	// <clientSocket, ClientData>

  public:
	ServerManager();
	~ServerManager();

	void acceptConnection(int serverSocket);
	void handleRead(int clientSocket);
	// void handleWrite(int clientSocket);

	// Getters
	const std::vector<Server *> &getServers() const { return _servers; }
	const std::map<int, ClientData> &getClientMap() const { return _clientMap; }

  private:
};