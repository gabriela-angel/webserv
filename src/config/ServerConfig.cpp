#include "ServerConfig.hpp"
#include "Logger.hpp"
#include "Socket.hpp"

ServerConfig::ServerConfig() : BaseConfig() {
	_listen.clear();
	_server_name.clear();
	_locations.clear();

	initSetters();
}

ServerConfig::ServerConfig(const ServerConfig& copy) : BaseConfig(copy) {
	_listen = copy._listen;
	_server_name = copy._server_name;
	_locations = copy._locations;

	initSetters();
}

ServerConfig& ServerConfig::operator=(const ServerConfig& other) {
	if (this != &other) {
		BaseConfig::operator=(other);

		_listen = other._listen;
		_server_name = other._server_name;
		_locations = other._locations;
		initSetters();
	}
	return *this;
}

ServerConfig::~ServerConfig(){}

void ServerConfig::initSetters() {
	_setters["listen"] = &ServerConfig::setListen;
	_setters["server_name"] = &ServerConfig::setServerName;
	_setters["root"] = &ServerConfig::setRoot;
	_setters["methods"] = &ServerConfig::setMethods;
	_setters["autoindex"] = &ServerConfig::setAutoindex;
	_setters["index"] = &ServerConfig::setIndexFiles;
	_setters["error_page"] = &ServerConfig::setErrorPage;
	_setters["return"] = &ServerConfig::setRedirect;
	_setters["client_max_body_size"] = &ServerConfig::setClientMaxBodySize;
}

inline static bool isPort(const std::string& str) {
	char *end;
	errno = 0;
	long num = strtol(str.c_str(), &end, 10);
	return errno == 0 && *end == '\0' && num >= 1 && num <= 65535;
}

inline static bool onlyNumeric(const std::string& str) {
	for (size_t i = 0; i < str.length(); i++) {
		if (!std::isdigit(static_cast<unsigned char>(str[i])))
			return false;
	}
	return true;
}

void ServerConfig::setListen(const std::vector<std::string>& value) {
	if (value.size() > 2)
		throw std::runtime_error("Syntax error: listen directive requires at most two arguments");
	if (value[0].empty())
		throw std::runtime_error("Syntax error: listen directive requires a non-empty argument");
	if (value.size() == 2 && value[1] != "default_server")
		throw std::runtime_error("Syntax error: invalid argument '" + value[1] + "' for listen directive");

	
	std::string arg1 = value[0];

	
	if (isPort(arg1)) {
		ListenDirective directive = handleAddress("0.0.0.0", arg1.c_str(), value.size() == 2);
		_listen.push_back(directive);
		return ;
	}


	size_t colon_pos = arg1.find(':');
	if (colon_pos != std::string::npos)
	{
		std::string host_part = arg1.substr(0, colon_pos);
		std::string port_part = arg1.substr(colon_pos + 1);

		if (!isPort(port_part))
			throw std::runtime_error("Syntax error: invalid port number '" + port_part + "' in listen directive " + arg1);

		if (onlyNumeric(host_part))
			throw std::runtime_error("Syntax error: invalid host '" + host_part + "' in listen directive " + arg1);

		ListenDirective directive = handleAddress(host_part.c_str(), port_part.c_str(), value.size() == 2);
		_listen.push_back(directive);
		return ;
	}
	

	// If we reach here, it means the argument is a host without a port
	if (onlyNumeric(arg1))
			throw std::runtime_error("Syntax error: invalid host '" + arg1 + "' in listen directive.");
	ListenDirective directive = handleAddress(arg1.c_str(), "80", value.size() == 2);
	_listen.push_back(directive);
}

ServerConfig::ListenDirective ServerConfig::handleAddress(const char *host, const char *port, bool default_server) {
	struct addrinfo hints;
    struct addrinfo *res;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET; 			// IPv4
	hints.ai_socktype = SOCK_STREAM;	// TCP
	hints.ai_flags = AI_PASSIVE;		// If host is empty, use INADDR_ANY

	int status = getaddrinfo(host, port, &hints, &res);

	if (status != 0) {
		freeaddrinfo(res);
		throw std::runtime_error("Invalid host address '" + std::string(host) + ":" + std::string(port) + "': " + std::string(gai_strerror(status)));
	}

	struct sockaddr_in* ipv4 = (struct sockaddr_in*)res->ai_addr;
	std::string ip_str = Socket::inetNtop(ipv4->sin_addr.s_addr);

	freeaddrinfo(res);
	return ListenDirective(ip_str, from_string<int>(port), default_server);
}

void ServerConfig::setServerName(const std::vector<std::string>& value) {
	if (!_server_name.empty())
		throw std::runtime_error("Syntax error: server_name directive is duplicated");
	if (value.size() < 1)
		throw std::runtime_error("Syntax error: server_name directive requires at least one argument");

	for (size_t j = 0; j < value.size(); j++)
	{
		if (value[j].empty())
			throw std::runtime_error("Syntax error: server_name directive requires non-empty arguments");
		
		for (size_t i = 0; i < value[j].length(); i++)
		{
			if (!std::isalnum(static_cast<unsigned char>(value[j][i])) && value[j][i] != '-' && value[j][i] != '.')
				throw std::runtime_error("Syntax error: server_name must not contain special characters");
			if ((i == 0 && value[j][i] == '-') || (i > 0 && value[j][i] == '.' && value[j][i - 1] == '-'))
				throw std::runtime_error("Syntax error: server_name must not start or end with a special character");
			if (i != 0 && value[j][i] == '.' && value[j][i - 1] == '.')
				throw std::runtime_error("Syntax error: server_name must not contain consecutive dots");
		}
	}

	_server_name = value;
}

const std::vector<ServerConfig::ListenDirective>& ServerConfig::getPortPair() const {
	if (_listen.empty())
		throw std::runtime_error("No listen directive defined for this server");
	return _listen;
}

bool ServerConfig::listensOnPort(int port) const {
	for (size_t i = 0; i < _listen.size(); ++i) {
		if (_listen[i].port == port)
			return true;
	}
	return false;
}

bool ServerConfig::isDefaultServer(void) const{
	for (size_t i = 0; i < _listen.size(); ++i) {
		if (_listen[i].default_server)
			return true;
	}
	return false;
}

const std::vector<std::string>& ServerConfig::getServerName() const {
	return _server_name;
}

std::vector<LocationConfig>& ServerConfig::getLocations() {
	if (_locations.empty())
		throw std::runtime_error("No locations defined for this server");
	return _locations;
}

const std::vector<LocationConfig>& ServerConfig::getLocations() const {
	if (_locations.empty())
		throw std::runtime_error("No locations defined for this server");
	return _locations;
}

void ServerConfig::setDirective(const std::string& key, const std::vector<std::string>& value) {
	std::map<std::string, Setter>::iterator it = _setters.find(key);

	if (it == _setters.end())
		throw std::runtime_error("Unknown directive: \'" + key + "\'");

	(this->*(it->second))(value);
}

void ServerConfig::addLocation(const LocationConfig& location) {
	for (size_t i = 0; i < _locations.size(); ++i) {
		if (_locations[i].getPathPrefix() == location.getPathPrefix())
			throw std::runtime_error("Syntax error: duplicate location path '" + location.getPathPrefix() + "'");
	}

	_locations.push_back(location);
}



// debug
std::ostream& operator<<(std::ostream& os, const ServerConfig& config) {
	os << "Port(s): ";
	const std::vector<ServerConfig::ListenDirective>& listen = config.getPortPair();
	for (size_t i = 0; i < listen.size(); i++) {
		os << listen[i].host << ":" << listen[i].port;
		if (listen[i].default_server)
			os << " (default)";
		if (i != listen.size() - 1)
			os << ", ";
	}
	os << "\n";
	for (size_t i = 0; i < config.getServerName().size(); i++) {
		os << "Server Name: " << config.getServerName()[i] << "\n";
	}
	os << "Root: " << config.getRoot() << "\n";
	os << "Client Max Body Size: " << config.getClientMaxBodySize() << "\n";
	os << "Error Pages:\n";
	const std::map<int, std::string>& errorPages = config.getErrorPages();
	for (std::map<int, std::string>::const_iterator it = errorPages.begin(); it != errorPages.end(); ++it) {
		os << "  " << it->first << ": " << it->second << "\n";
	}
	os << "Locations:\n";
	const std::vector<LocationConfig>& locations = config.getLocations();
	for (size_t i = 0; i < locations.size(); i++) {
		os << locations[i];
	}
	return os;
}
