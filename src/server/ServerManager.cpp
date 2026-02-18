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
	_logger.logInfo("Accepted new connection from " + std::string(inetNtop(client_addr.sin_addr.s_addr)) + ":" + to_string(ntohs(client_addr.sin_port)));
	_epoll.addClient(client_socket, EPOLLIN);
	_clientMap[client_socket] = ClientData(client_addr, client_socket, serverSocket);
}

ServerManager::~ServerManager()
{
	for (std::vector<Server *>::iterator it = _servers.begin(); it != _servers.end(); ++it)
		delete *it;
}

void ServerManager::handleRead(int clientSocket)
{
	ClientData &client = _clientMap[clientSocket];
	char buffer[4096];
	ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
	if (bytesRead < 0)
	{
		_logger.logError("Error reading from client socket");
		_clientMap.erase(clientSocket);
		close(clientSocket);
		return;
	}
	else if (bytesRead == 0)
	{
		_logger.logInfo("Client disconnected");
		_clientMap.erase(clientSocket);
		close(clientSocket);
		return;
	}
	
	buffer[bytesRead] = '\0';
	client.readBuffer += std::string(buffer);
	HttpRequest::updateState(client);
	if (client.readState == ERROR)
	{
		_logger.logError("Error parsing HTTP request from client... closing connection");
		_epoll.removeClient(clientSocket);
		_clientMap.erase(clientSocket);
		close(clientSocket);
		return;
	}
	
	
	if (client.readState == READING)
	{
		_logger.logInfo("Partial HTTP request received from client, waiting for more data");
		return;
	}
	
	
	if (client.readState == READ_COMPLETE)
	{
		_logger.logInfo("Complete HTTP request received from client");
		_epoll.modifyClient(clientSocket, EPOLLOUT);
		return;
	}
}