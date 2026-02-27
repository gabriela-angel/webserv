#pragma once

#include "EpollManager.hpp"
#include "Server.hpp"
#include "Http.hpp"
#include <map>
#include <string>
#include <vector>

#define CLI_BUFFER_SIZE 4096
#define CLI_FIRST_READ_TIMEOUT 5
#define CLI_TIMEOUT 30

class ServerManager
{
  private:
	Logger&						_logger;
	EpollManager&				_epoll;
	std::vector<Server *>		_servers;
	std::map<int, ClientData>	_clientMap;	// <clientSocket, ClientData>

  public:
	typedef std::map<int, ClientData>::iterator ClientIterator;
	ServerManager();
	~ServerManager();

	void	acceptConnection(int serverSocket);
	void	handleRead(int clientSocket);
	void	removeClient(int clientSocket);
	bool	isServerSocket(int sockfd) const;
	int		getEpollFD() const;
	// void handleWrite(int clientSocket);

	// Getters
	const std::vector<Server *> &getServers() const { return _servers; }
	const std::map<int, ClientData> &getClientMap() const { return _clientMap; }

  private:
};