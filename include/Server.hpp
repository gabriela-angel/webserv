#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

#include "Logger.hpp"

#define SOMAXCONN 4096


// Mock configuration struct for development purposes. --------------------------------------------------------
struct Config {
	in_addr_t address;
	int port;
	// Constructor with default values
	Config() : address(INADDR_ANY), port(8080) {}
};
// ------------------------------------------------------------------------------------------------------------


// A TCP server class that encapsulates the creation, configuration, binding, and listening of a server socket.
class Server {
	private:
		static Logger& _logger;					// Logger instance
		int	_serverSocket;								// Server main socket
		int	_backlog;									// Maximum number of pending connections
		struct sockaddr_in _serv_addr;					// Server address structure
		Config _config;									// Server configuration

	public:
		Server();
		~Server();
		bool init( void );

		// Getters
		int getServerSocket() const { return _serverSocket; }

	private:
		bool	_createSocket( void );
		bool	_configureSocket( void );
		bool	_bindSocket( void );
		bool	_listenSocket( void );
};

#include "utils.tpp"