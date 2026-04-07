#include "Socket.hpp"

Logger& Socket::_logger = Logger::getInstance();

Socket::Socket(const Config &config) : _fd(-1), _backlog(SOMAXCONN), _config(config) {}

std::string Socket::inetNtop(uint32_t binary_ip)
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

Socket::~Socket() {
	if (_fd != -1)
		close(_fd);
}

bool Socket::init( void ) { 
	if (!_openSocket()) return false;
	if (!_configureSocket()) return false;
	if (!_bindSocket()) return false;
	if (!_listenSocket()) return false;
	return true;
}

bool Socket::_openSocket( void ) {
	_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_fd < 0)
	{
		_logger.logFatal("Error opening socket");
		_logger.logDebug("error code: " + to_string(errno));
		_logger.logDebug("error message: " + std::string(strerror(errno)));
		return false;
	}
	_logger.logDebug("Socket created");
	return true;
}

bool Socket::_configureSocket( void ) {
	// Set socket options to allow address reuse
	int opt = 1;
	if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
		_logger.logFatal("Error on setting socket options");
		_logger.logDebug("error code: " + to_string(errno));
		_logger.logDebug("error message: " + std::string(strerror(errno)));
		return false;
	}
	_logger.logDebug("Socket options set");
	return true;
}

bool Socket::_bindSocket( void ) {

	struct sockaddr_in _serv_addr;

	_serv_addr.sin_family = AF_INET;
	_serv_addr.sin_addr.s_addr = _config.address;
	_serv_addr.sin_port = htons(_config.port);
	if (bind(_fd, (struct sockaddr *)&_serv_addr, sizeof(_serv_addr)) < 0)
	{
		_logger.logFatal("Error on binding to " + Socket::inetNtop(_config.address) + ":" + to_string(_config.port));
		_logger.logDebug("error code: " + to_string(errno));
		_logger.logDebug("error message: " + std::string(strerror(errno)));
		return false;
	}
	_logger.logDebug("Binded to " + Socket::inetNtop(_config.address) + ":" + to_string(_config.port));
	return true;
}

bool Socket::_listenSocket( void ) {
	if (listen(_fd, _backlog) < 0)
	{
		_logger.logFatal("Error on listening to " + Socket::inetNtop(_config.address) + ":" + to_string(_config.port));
		_logger.logDebug("error code: " + to_string(errno));
		_logger.logDebug("error message: " + std::string(strerror(errno)));
		return false;
	}
	_logger.logInfo("Socket listening on " + Socket::inetNtop(_config.address) + ":" + to_string(_config.port));
	return true;
}