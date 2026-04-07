#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

#include "ServerConfig.hpp"
#include "Logger.hpp"

#define SOMAXCONN 4096


// A TCP server class that encapsulates the creation, configuration, binding, and listening of a server socket.
class Socket {
	public:
	struct Config {
		in_addr_t address;
		int port;

		Config() : address(INADDR_ANY), port(8080) {}
		Config(in_addr_t addr, int p) : address(addr), port(p) {}
	};


	private:
		Socket(){};
		static Logger& _logger;							// Logger instance
		int	_fd;										// Server main socket
		int	_backlog;									// Maximum number of pending connections
		Config _config;									// Server configuration

	public:
		Socket(const Config &config);
		~Socket();
		bool init( void );

		// Getters
		int getFD() const { return _fd; }
		static std::string inetNtop(uint32_t binary_ip);

	private:
		bool	_openSocket( void );
		bool	_configureSocket( void );
		bool	_bindSocket( void );
		bool	_listenSocket( void );
};

#include "utils.tpp"