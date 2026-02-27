#include "ServerManager.hpp"
#include "HttpRequest.hpp"

inline static std::string inetNtop(uint32_t binary_ip)
{
	uint32_t host_ip = ntohl(binary_ip);

	int byte1 = (host_ip >> 24) & 0xFF;
	int byte2 = (host_ip >> 16) & 0xFF;
	int byte3 = (host_ip >> 8) & 0xFF;
	int byte4 = host_ip & 0xFF;
	std::ostringstream ip_stream;
	ip_stream << byte1 << "." << byte2 << "." << byte3 << "." << byte4;
	return ip_stream.str();
}

ServerManager::ServerManager() : _logger(Logger::getInstance()), _epoll(EpollManager::getInstance())
{
	Server *newServer = new Server();
	if (!newServer->init())
	{
		delete newServer;
		throw std::runtime_error("Failed to initialize the server");
	}
	_servers.push_back(newServer);
	_epoll.addServer(newServer->getSocket());
}
ServerManager::~ServerManager()
{
	for (ClientIterator it = _clientMap.begin(); it != _clientMap.end(); ++it)
		close(it->first);
	for (std::vector<Server *>::iterator it = _servers.begin(); it != _servers.end(); ++it)
		delete *it;
}

// ----------------------------------------- Client Management -----------------------------------------

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
	
	_logger.logInfo("Accepted new connection from " + std::string(inetNtop(client_addr.sin_addr.s_addr)) + ":" + to_string(ntohs(client_addr.sin_port)));
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
	}
	
}

void ServerManager::handleWrite(int clientSocket)
{
	/*
		TO DO:
		- Identify Session via cookies
			- If no session, create Session and set cookie with Set-Cookie header
				- Set-Cookie: <name>=<value>; Expires=<date>; Max-Age=<seconds>; Domain=<domain>; Path=<path>; Secure; HttpOnly

		- Check Exceptions to handle errors
			- send Response and if shouldClose, remove client
		- Handle Redirects (3xx)
		- Handle Keep-Alive (Connection: keep-alive)
		- Handle Connection Close (Connection: close)
	*/
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