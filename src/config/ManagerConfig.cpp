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

const ServerConfig& ManagerConfig::findServer(int port, const std::string& host_header) const {
	int default_server = -1;
	for (size_t i = 0; i < _servers.size(); i++) {
		if (_servers[i].listensOnPort(port)) {
			const std::vector<std::string>& server_names = _servers[i].getServerName();
			if (std::find(server_names.begin(), server_names.end(), host_header) != server_names.end())
				return _servers[i];
			if (default_server == -1 || _servers[i].isDefaultServer())
				default_server = i;
		}
	}
	if (default_server != -1)
		return _servers[default_server];
	throw HttpException(HttpException::ParseError::MALFORMED_HEADER);
}

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