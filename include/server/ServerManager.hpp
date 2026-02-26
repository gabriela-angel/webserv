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

enum ReadState
{
	READING_REQUEST_LINE,
	READING_HEADERS,
	READING_BODY,
	DONE,
	ERROR
};

struct StateMachine
{
	std::string buffer;
	struct HttpData httpData;
	ReadState state;

	StateMachine() : buffer(""), httpData(), state(READING_REQUEST_LINE) {};
};

struct	ClientData
{
	// Client information
	int clientSocket;
	int	serverSocket;
	struct sockaddr_in client_addr;

	// Client State Machine for parsing HTTP requests
	StateMachine stateMachine;

	// Client Timeout data
	bool firstRead;
	std::time_t lastActivityTime;

	// Client Error ?
	HttpException exception;

	// Constructors
	ClientData(const struct sockaddr_in &addr, int clientSocket, int serverSocket)
	: 
		clientSocket(clientSocket),
		serverSocket(serverSocket),
		client_addr(addr),
		stateMachine(StateMachine()),
		firstRead(false),
		lastActivityTime(std::time(NULL)),
		exception()
	{}
	
	ClientData()
	: 
		clientSocket(-1),
		serverSocket(-1),
		client_addr(),
		stateMachine(),
		firstRead(false),
		lastActivityTime(0),
		exception()
	{}

};

class ServerManager
{
  private:
	Logger&						_logger;
	EpollManager&				_epoll;
	std::vector<Server *>		_servers;
	std::map<int, ClientData>	_clientMap;	// <clientSocket, ClientData>

  public:
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