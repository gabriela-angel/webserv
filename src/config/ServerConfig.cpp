#include "./config/ServerConfig.hpp"
#include "Logger.hpp"

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

void ServerConfig::setListen(const std::vector<std::string>& value) {
	if (value.size() != 1)
		throw std::runtime_error("Syntax error: listen directive requires exactly one argument");

	try {
		std::string port_part = value[0];
		std::string host_part = "";
		if (value[0].find(':') != std::string::npos) {
			size_t colon_pos = value[0].find(':');
			port_part = value[0].substr(colon_pos + 1);
			host_part = value[0].substr(0, colon_pos);
		}
		char *end;
		errno = 0;
		long port = std::strtol(port_part.c_str(), &end, 10);

		if (errno != 0 || *end != '\0' || port < 1 || port > 65535)
			throw std::runtime_error("");

		setHost(host_part, static_cast<int>(port));
	} catch (...) {
		throw std::runtime_error("Syntax error: listen directive requires a valid port number and optional host");
	}
}

void ServerConfig::setHost(const std::string& value, int port_part) {
	if (value.empty()) {
		_listen[port_part] = "0.0.0.0";
	}
	
	std::istringstream iss(value);
	std::string octect;
	int count = 0;

	while (std::getline(iss, octect, '.')) {
		try {
			if (octect.empty())
				throw std::runtime_error("");

			char *end;
			errno = 0;
			long num = strtol(octect.c_str(), &end, 10);

			if (errno != 0 || *end != '\0' || num < 0 || num > 255)
				throw std::runtime_error("");
			count++;
		} catch (...) {
			throw std::runtime_error("Syntax error: host directive requires a valid IP address");
		}
	}
	if (count != 4 )
		throw std::runtime_error("Syntax error: host directive requires a valid IP address");
	_listen[port_part] = value;
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


//NOT GOOD BEHAVIOUR
const std::map<int, std::string>& ServerConfig::getPortPair() const {
	if (_listen.empty())
		throw std::runtime_error("No listen directive defined for this server");
	return _listen;
}

bool ServerConfig::listensOnPort(int port) const {
	std::map<int, std::string>::const_iterator it = _listen.find(port);
	return it != _listen.end();
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
	const std::map<int, std::string>& listen = config.getPortPair();
	for (std::map<int, std::string>::const_iterator it = listen.begin(); it != listen.end(); ++it) {
		os << it->first << " (" << it->second << ") ";
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
