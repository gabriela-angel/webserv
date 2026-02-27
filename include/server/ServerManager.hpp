#pragma once

#include "EpollManager.hpp"
#include "Server.hpp"
#include "Http.hpp"
#include <map>
#include <string>
#include <vector>
#include <unistd.h>
#include <fcntl.h>
#include <stdexcept>
#include <sstream>
#include <iomanip>


#define CLI_BUFFER_SIZE 4096
#define CLI_FIRST_READ_TIMEOUT 5
#define CLI_TIMEOUT 30

#define SESSIONID "sessionId"
#define MAX_SESSION_INACTIVITY 600	// 10 minutes
#define MAX_SESSION_LIFETIME 7200	// 2	hours

struct Session
{
	std::string sessionId;
	std::time_t createdAt;
	std::time_t lastActivity;

	Session(const std::string &id) : sessionId(id), createdAt(std::time(NULL)), lastActivity(std::time(NULL)) {}
	Session() : sessionId(""), createdAt(std::time(NULL)), lastActivity(std::time(NULL)) {}
};

class ServerManager
{
  public:
	typedef std::map<std::string, Session>				Sessions;		// <sessionId, Session*>
	typedef std::map<std::string, Session>::iterator	SessionIterator;
	typedef std::map<int, ClientData>::iterator			ClientIterator;

  private:	
	Logger&						_logger;
	EpollManager&				_epoll;
	std::vector<Server *>		_servers;
	std::map<int, ClientData>	_clientMap;		// <clientSocket, ClientData>
	Sessions					_sessions;		// <sessionId, SessionData>
	int							_urandom_fd;	// File descriptor for /dev/urandom to get true random numbers

  public:
	ServerManager();
	~ServerManager();

	void	acceptConnection(int serverSocket);
	void	handleRead(int clientSocket);
	void	handleWrite(int clientSocket);
	void	removeClient(int clientSocket);
	void	removeSession(const std::string &sessionId);
	bool	isServerSocket(int sockfd) const;
	int		getEpollFD() const;

	// Getters
	const std::map<int, ClientData> &getClientMap() const { return _clientMap; }
	const Sessions &getSessions() const { return _sessions; }

  private:
	std::string generateSecureSessionId();
	void _handleSession(ClientData &client);
};