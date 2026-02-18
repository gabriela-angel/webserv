#include "Server.hpp"

Logger& Server::_logger = Logger::getInstance();

Server::Server() : _serverSocket(-1), _backlog(SOMAXCONN) {}
Server::~Server() {
	if (_serverSocket != -1)
		close(_serverSocket);
}

bool Server::init( void ) { 
	if (!_createSocket()) return false;
	if (!_configureSocket()) return false;
	if (!_bindSocket()) return false;
	if (!_listenSocket()) return false;
	return true;
}

bool Server::_createSocket( void ) {
	_serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (_serverSocket < 0)
	{
		_logger.logFatal("Error opening socket");
		_logger.logDebug("error code: " + to_string(errno));
		_logger.logDebug("error message: " + std::string(strerror(errno)));
		return false;
	}
	_logger.logDebug("Socket created");
	return true;
}

bool Server::_configureSocket( void ) {
	// Set socket options to allow address reuse
	int opt = 1;
	if (setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
		_logger.logFatal("Error on setting socket options");
		_logger.logDebug("error code: " + to_string(errno));
		_logger.logDebug("error message: " + std::string(strerror(errno)));
		close(_serverSocket);
		return false;
	}
	_logger.logDebug("Socket options set");
	return true;
}

bool Server::_bindSocket( void ) {
	_serv_addr.sin_family = AF_INET;
	_serv_addr.sin_addr.s_addr = _config.address;
	_serv_addr.sin_port = htons(_config.port);
	if (bind(_serverSocket, (struct sockaddr *)&_serv_addr, sizeof(_serv_addr)) < 0)
	{
		_logger.logFatal("Error on binding to port " + to_string(_config.port));
		_logger.logDebug("error code: " + to_string(errno));
		_logger.logDebug("error message: " + std::string(strerror(errno)));
		close(_serverSocket);
		return false;
	}
	_logger.logDebug("Binded to port " + to_string(_config.port));
	return true;
}

bool Server::_listenSocket( void ) {
	if (listen(_serverSocket, _backlog) < 0)
	{
		_logger.logFatal("Error on listening to port " + to_string(_config.port));
		_logger.logDebug("error code: " + to_string(errno));
		_logger.logDebug("error message: " + std::string(strerror(errno)));
		close(_serverSocket);
		return false;
	}
	_logger.logDebug("Listening on port " + to_string(_config.port));
	return true;
}