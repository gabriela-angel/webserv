#include "ServerManager.hpp"
#include "HttpRequest.hpp"

std::vector<Socket::Config> mapSocketConfig(const std::vector<ServerConfig> &servers)
{
	
	typedef std::vector<ServerConfig> ServerConfigVector;
	typedef ServerConfigVector::const_iterator ServerConfigIterator;

	std::vector<Socket::Config> socketConfigs;
	for(ServerConfigIterator it = servers.begin(); it != servers.end(); ++it)
	{
		ServerConfig serverConfig = *it;
		std::vector<ServerConfig::ListenDirective> listens = serverConfig.getPortPair();

		for (size_t i = 0; i < listens.size(); i++)
		{
			
			Socket::Config config;
			config.address = inet_addr(listens[i].host.c_str());
			config.port = listens[i].port;

			bool duplicate = false;
			for (size_t j = 0; j < socketConfigs.size(); j++)
			{
				if (socketConfigs[j].address == config.address && socketConfigs[j].port == config.port)
				{
					duplicate = true;
					break;
				}
			}
			if (!duplicate)
				socketConfigs.push_back(config);
		}
	}
	return socketConfigs;
}


ServerManager::ServerManager(const char *configFilePath)
:
	_logger(Logger::getInstance()),
	_epoll(EpollManager::getInstance()),
	_configs(ParseConfig(configFilePath).parse())
{
	/*-- Prepare to generate secure session IDs --*/
	_urandom_fd = open("/dev/urandom", O_RDONLY);
	if (_urandom_fd < 0)
		throw std::runtime_error("Failed to open /dev/urandom, cannot generate secure session IDs");

	/*-- Starting Servers --*/

	typedef std::vector<Socket::Config> SocketConfigVector;
	typedef SocketConfigVector::const_iterator SocketConfigIterator;
	SocketConfigVector socketsToCreate = mapSocketConfig(_configs.getServers());

	for(SocketConfigIterator it = socketsToCreate.begin(); it != socketsToCreate.end(); ++it)
	{
		Socket *newSocket = new Socket(*it);
		if (!newSocket->init())
		{
			delete newSocket;
			throw std::runtime_error("Failed to initialize socket on " + Socket::inetNtop(it->address) + ":" + to_string(it->port));
		}
		_sockets.push_back(newSocket);
		_epoll.addServer(newSocket->getFD());
	}
}

ServerManager::~ServerManager()
{
	close(_urandom_fd);
	for (ClientIterator it = _clientMap.begin(); it != _clientMap.end(); ++it)
		close(it->first);
	for (std::vector<Socket *>::iterator it = _sockets.begin(); it != _sockets.end(); ++it)
		delete *it;
}

// ----------------------------------------- Client Management -----------------------------------------

std::string ServerManager::generateSecureSessionId()
{
    const size_t byteLength = 16; // 128 bits
    unsigned char buffer[byteLength];

    ssize_t result = read(_urandom_fd, buffer, byteLength);

    if (result != (ssize_t)byteLength)
        throw std::runtime_error("Failed to read enough random bytes for session ID generation");

    std::ostringstream oss;
    for (size_t i = 0; i < byteLength; ++i)
    {
        oss << std::hex
            << std::setw(2)
            << std::setfill('0')
            << (int)buffer[i];
    }

    return oss.str();
}

void ServerManager::removeSession(const std::string &sessionId)
{
	_sessions.erase(sessionId);
}

void ServerManager::removeClient(int clientSocket)
{
	_logger.logInfo("Removing client socket: " + to_string(clientSocket));
	_epoll.removeClient(clientSocket);
	_clientMap.erase(clientSocket);
	close(clientSocket);
}

void ServerManager::acceptConnection( int serverSocket ){
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);
	int client_socket = accept(serverSocket, (struct sockaddr *)&client_addr, &client_len);
	if (client_socket < 0)
	{
		_logger.logError("Error accepting new connection");
		_logger.logDebug("error code: " + to_string(errno));
		_logger.logDebug("error message: " + std::string(strerror(errno)));
		return;
	}

	struct timeval timeout = {2, 0}; // 2 seconds timeout in recv
	if (setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout)) < 0)
	{
		_logger.logError("Error setting socket timeout");
		_logger.logDebug("error code: " + to_string(errno));
		_logger.logDebug("error message: " + std::string(strerror(errno)));
		removeClient(client_socket);
		return;
	}
	
	_logger.logInfo("Accepted new connection from " + std::string(Socket::inetNtop(client_addr.sin_addr.s_addr)) + ":" + to_string(ntohs(client_addr.sin_port)));
	_epoll.addClient(client_socket, EPOLLIN);
	_clientMap[client_socket] = ClientData(client_addr, client_socket, serverSocket);
}

void ServerManager::handleRead(int clientSocket)
{
	ClientData &client = _clientMap[clientSocket];
	char buffer[CLI_BUFFER_SIZE];
	ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
	if (bytesRead < 0)
	{
		_logger.logError("Error reading from client socket");
		_logger.logDebug("error code: " + to_string(errno));
		_logger.logDebug("error message: " + std::string(strerror(errno)));
		removeClient(clientSocket);
		return;
	}
	else if (bytesRead == 0)
	{
		_logger.logInfo("Client " + to_string(clientSocket) + " disconnected");
		removeClient(clientSocket);
		return;
	}
	
	buffer[bytesRead] = '\0';
	client.stateMachine.buffer += std::string(buffer);
	_logger.logDebug("Client " + to_string(client.clientSocket) + " read buffer: \n" + client.stateMachine.buffer);
	
	HttpRequest::processClient(client);
	if (client.stateMachine.state == StateMachine::DONE || client.stateMachine.state == StateMachine::ERROR)
	{
		client.stateMachine.updateActivity();
		_epoll.modifyClient(clientSocket, EPOLLOUT);
		_handleSession(client);
	}
}

void ServerManager::handleWrite(int clientSocket)
{
	ClientData &client = _clientMap[clientSocket];
	HttpData &httpData = client.stateMachine.httpData;
	
 	HttpStruct responseData;

	responseData.method = httpData.requestLine.method;
	responseData.HTTPVersion = httpData.requestLine.version;
	responseData.uri = httpData.requestLine.uri;
	responseData.host = httpData.host;
	responseData.headers = httpData.headers;
	if (responseData.method == "POST" || responseData.method == "PUT")
		responseData.body = httpData.body;
	responseData.exception = client.exception;

	// HttpResponse parseResponse(responseData)
	// Add Set-Cookie: SESSIONID=<value>; Max-Age=MAX_SESSION_INACTIVITY; Path=/; HttpOnly
	
	/*
		- Session Management
			- Implement a random feature that uses Sessions (already implemented in clients)

		- Check Exceptions to handle errors
			- send Response and if shouldClose, remove client
		- Handle Keep-Alive (Connection: keep-alive)

		- send Response with Set-Cookie: SESSIONID=<value>; Max-Age=MAX_SESSION_INACTIVITY; Path=/; HttpOnly
	*/

	
}

void ServerManager::_handleSession(ClientData &client)
{
	Http::Cookies &cookies = client.stateMachine.httpData.cookies;

	// if cookie sessionId exists
	if (cookies.count(SESSIONID) > 0)
	{
		// if sessionId refers a valid session
		if (_sessions.count(cookies[SESSIONID]) > 0)
		{
			_logger.logInfo("Client " + to_string(client.clientSocket) + " has valid session: " + cookies[SESSIONID]);
			client.sessionId = cookies[SESSIONID];
			return ;
		}
	}

	// If the the execution reaches this point, client has no valid session
	std::string newSessionId = generateSecureSessionId();
	_sessions[newSessionId] = Session(newSessionId);
	client.sessionId = newSessionId;
	_logger.logInfo("Client " + to_string(client.clientSocket) + " assigned new session: " + newSessionId);
}

// ------------------------------------------ Utility Methods -----------------------------------------

bool ServerManager::isServerSocket(int sockfd) const
{
	return _epoll.isServerSocket(sockfd);
}

int ServerManager::getEpollFD() const
{
	return _epoll.getEpollFD();
}