#include "ManagerConfig.hpp"
#include "HttpException.hpp"

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

//debug
std::ostream& operator<<(std::ostream& os, const ManagerConfig& config) {
	const std::vector<ServerConfig>& servers = config.getServers();
	for (size_t i = 0; i < servers.size(); i++) {
		os << "Server " << i + 1 << ":\n";
		os << servers[i];
		if (i != servers.size() - 1)
			os << "\n";
	}
	return os;
}