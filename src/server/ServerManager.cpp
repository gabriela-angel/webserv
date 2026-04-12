#include "ServerManager.hpp"
#include "HttpRequest.hpp"
#include "RequestProcessor.hpp"

inline static std::vector<ConfigMap> mapSocketConfig(const std::vector<ServerConfig> &servers)
{
	typedef std::vector<ServerConfig>			ServerConfigVector;
	typedef ServerConfigVector::const_iterator	ServerConfigIterator;

	std::vector<ConfigMap> configMaps;
	for (ServerConfigIterator it = servers.begin(); it != servers.end(); ++it)
	{
		const ServerConfig& serverConfig = *it;
		const std::vector<ServerConfig::ListenDirective>& listens = serverConfig.getPortPair();

		for (size_t i = 0; i < listens.size(); i++)
		{
			Socket::Config config;
			config.address = inet_addr(listens[i].host.c_str());
			config.port = listens[i].port;

			size_t found = configMaps.size();
			for (size_t j = 0; j < configMaps.size(); j++)
			{
				if (configMaps[j].config.address == config.address &&
					configMaps[j].config.port == config.port)
				{
					found = j;
					break;
				}
			}

			if (found == configMaps.size())
			{
				ConfigMap map;
				map.config = config;
				map.servers.push_back(serverConfig);
				configMaps.push_back(map);
			}
			else
				configMaps[found].servers.push_back(serverConfig);
		}
	}
	return configMaps;
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

	typedef std::vector<ConfigMap>					SocketConfigMapVector;
	typedef SocketConfigMapVector::const_iterator	SocketConfigMapIterator;
	SocketConfigMapVector socketsToCreate = mapSocketConfig(_configs.getServers());

	for(SocketConfigMapIterator it = socketsToCreate.begin(); it != socketsToCreate.end(); ++it)
	{
		Socket *newSocket = new Socket((*it).config);
		if (!newSocket->init())
		{
			delete newSocket;
			throw std::runtime_error("Failed to initialize socket on " + Socket::inetNtop((*it).config.address) + ":" + to_string((*it).config.port));
		}
		_sockets.push_back(newSocket);
		_epoll.addServer(newSocket->getFD());
		_serversMap[newSocket->getFD()] = (*it).servers;
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

std::string ServerManager::_generateSecureSessionId()
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
	_logger.logDebug("Removing client socket: " + to_string(clientSocket));
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
	ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
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
	
	client.stateMachine.buffer.append(buffer, bytesRead);
	_logger.logDebug("Client " + to_string(client.clientSocket) + " read buffer: \n" + client.stateMachine.buffer);
	
	HttpRequest::processClient(client, *this);
	if (client.stateMachine.state == StateMachine::DONE || client.stateMachine.state == StateMachine::ERROR)
	{
		client.stateMachine.updateActivity();
		client.stateMachine.state = StateMachine::READING_REQUEST_LINE; // Reset state for next request (if keep-alive)
		_epoll.modifyClient(clientSocket, EPOLLOUT);
		_handleSession(client);
	}
}

void ServerManager::handleWrite(int clientSocket)
{
	ClientData &client = _clientMap[clientSocket];

	if (client.exception)
	{
		HttpResponse errorResponse;
		HttpStatus::Code statusCode = client.exception.getStatusCode();
		errorResponse.setStatus(statusCode);
		errorResponse.setBody("<html><body><h1>" + to_string(statusCode) + " " + HttpStatus::reasonPhrase(statusCode) + "</h1></body></html>");
		errorResponse.addHeader("Content-Type", "text/html");
		errorResponse.addHeader("Set-Cookie", std::string(SESSIONID) + "=" + client.sessionId + "; Max-Age=" + to_string(MAX_SESSION_INACTIVITY) + "; Path=/; HttpOnly");
		errorResponse.addHeader("Content-Length", to_string(errorResponse.getBody().size()));
		std::string responseStr = errorResponse.toString();
		_logger.logDebug("Sending error response to client " + to_string(client.clientSocket) + ":\n" + responseStr);
		if (client.exception.shouldClose())
			errorResponse.addHeader("Connection", "close");
		
		if (!_secureSend(clientSocket, responseStr))
		{
			removeClient(clientSocket);
			return ;
		}

		if (client.stateMachine.httpData.keepAlive == false)
		{
			removeClient(clientSocket);
			return ;
		}

		if (client.exception.shouldClose())
			removeClient(clientSocket);
		else
			_epoll.modifyClient(clientSocket, EPOLLIN);
		
		return;
	}


	HttpData &httpData = client.stateMachine.httpData;
	
 	HttpStruct request;

	request.method = httpData.requestLine.method;
	request.HTTPVersion = httpData.requestLine.version;
	request.uri = httpData.requestLine.uri;
	request.headers = httpData.headers;
	request.host = httpData.host;
	size_t colon_pos = httpData.host.find(':');
	if (colon_pos != std::string::npos)
		request.host = httpData.host.substr(0, colon_pos);
	if (request.method == "POST" || request.method == "PUT")
		request.body = httpData.body;
	request.serverSocket = client.serverSocket;

	// Get IP and Port from server socket
	struct sockaddr_in server_addr;
	socklen_t server_addr_len = sizeof(server_addr);
	if (getsockname(client.serverSocket, (struct sockaddr *)&server_addr, &server_addr_len) < 0)
	{
		_logger.logError("Error getting server socket name");
		_logger.logDebug("error code: " + to_string(errno));
		_logger.logDebug("error message: " + std::string(strerror(errno)));
		removeClient(clientSocket);
		return;
	}	
	
	request.port = ntohs(server_addr.sin_port);
	request.ip = Socket::inetNtop(server_addr.sin_addr.s_addr);

	HttpResponse response = RequestProcessor::process(request, *this);

	// Session Cookie
	response.addHeader("Set-Cookie", std::string(SESSIONID) + "=" + client.sessionId + "; Max-Age=" + to_string(MAX_SESSION_INACTIVITY) + "; Path=/; HttpOnly");

	// Handle Content-Length for responses without body
	if (response.getHeaders().hasKey("Content-Length") == false)
		response.addHeader("Content-Length", to_string(response.getBody().size()));

	std::string responseStr = response.toString();
	if (!_secureSend(clientSocket, responseStr))
	{
		removeClient(clientSocket);
		return;
	}
	
	if (httpData.keepAlive)
		_epoll.modifyClient(clientSocket, EPOLLIN);
	else
		removeClient(clientSocket);
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
	std::string newSessionId = _generateSecureSessionId();
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

const ServerConfig& ServerManager::findServer(int serverSocket, const std::string &host_header) const
{
    const std::vector<ServerConfig> &servers = _serversMap.at(serverSocket);
    const ServerConfig* default_server = NULL;

    for (size_t i = 0; i < servers.size(); ++i)
    {
        if (!default_server && servers[i].isDefaultServer())
            default_server = &servers[i];

        const std::vector<std::string> &serverNames = servers[i].getServerName();
        for (size_t j = 0; j < serverNames.size(); ++j)
        {
            if (serverNames[j] == host_header)
                return servers[i]; // primeiro match
        }
    }

    if (default_server)
        return *default_server;
    return servers[0];
}

bool ServerManager::_secureSend(int clientSocket, const std::string &data)
{
	size_t total = 0;
	while (total < data.size()) {
		ssize_t bytesSent = send(clientSocket, (data.c_str() + total), (data.size() - total), MSG_NOSIGNAL);

		if (bytesSent < 0)	{
			_logger.logError("Error sending response to client socket");
			_logger.logDebug("error code: " + to_string(errno));
			_logger.logDebug("error message: " + std::string(strerror(errno)));
			return false;
		}
		total += bytesSent;
	}
	_logger.logDebug("Response to client " + to_string(clientSocket) + ":\n" + data);
	_logger.logInfo("Sent response to client " + to_string(clientSocket) + ", bytes sent: " + to_string(total));
	return true;
}