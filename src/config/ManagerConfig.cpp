#include "ManagerConfig.hpp"

ManagerConfig::ManagerConfig(const std::vector<ServerConfig>& servers) : _servers(servers) {
	if (_servers.empty())
		throw std::runtime_error("Configuration error: At least one server block must be defined");
}

ManagerConfig::ManagerConfig(const ManagerConfig& copy){
	*this = copy;
}
ManagerConfig& ManagerConfig::operator=(const ManagerConfig& other){
	if (this != &other)
		_servers = other._servers;
	return	*this;
}

ManagerConfig::~ManagerConfig() {}

ServerConfig& ManagerConfig::findServer(int port, const std::string& host_header) {
	int default_server = -1;
	for (size_t i = 0; i < _servers.size(); i++) {
		const std::vector<int>& ports = _servers[i].getPort();
		if (std::find(ports.begin(), ports.end(), port) != ports.end()) {
			if (default_server == -1)
				default_server = static_cast<int>(i);
			if (_servers[i].getServerName() == host_header)
				return _servers[i];
		}
	}
	if (default_server != -1)
		return _servers[default_server];
	throw std::runtime_error("No server listening on this port");
}