#include "./config/ServerConfig.hpp"
#include "Logger.hpp"

ServerConfig::ServerConfig(){
	_has_port = false;
	_port.clear();
	_host = "0.0.0.0";
	_server_name = "";
	_has_root = false;
	_root = "";
	_error_pages.clear();
	_has_client_max_body_size = false;
	_client_max_body_size = DEFAULT_CLIENT_MAX_BODY_SIZE;
	_locations.clear();

	initSetters();
}

ServerConfig::ServerConfig(const ServerConfig& copy) {
	*this = copy;
}

ServerConfig& ServerConfig::operator=(const ServerConfig& other){
	if (this != &other) {
		_has_port = other._has_port;
		_port = other._port;
		_host = other._host;
		_server_name = other._server_name;
		_has_root = other._has_root;
		_root = other._root;
		_error_pages = other._error_pages;
		_client_max_body_size = other._client_max_body_size;
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
	_setters["error_page"] = &ServerConfig::setErrorPage;
	_setters["client_max_body_size"] = &ServerConfig::setClientMaxBodySize;
}

void ServerConfig::setListen(const std::vector<std::string>& value) {
	if (value.size() != 1)
		throw std::runtime_error("Syntax error: listen directive requires exactly one argument");
	
	try {
		std::string port_part = value[0];
		if (value[0].find(':') != std::string::npos) {
			size_t colon_pos = value[0].find(':');
			port_part = value[0].substr(colon_pos + 1);
			std::string host_part = value[0].substr(0, colon_pos);
			setHost(host_part);
		}
		char *end;
		errno = 0;
		long port = std::strtol(port_part.c_str(), &end, 10);

		if (errno != 0 || *end != '\0' || port < 1 || port > 65535)
			throw std::runtime_error("");

		_port.push_back(port);
		_has_port = true;
	} catch (...) {
		throw std::runtime_error("Syntax error: listen directive requires a valid port number");
	}
}

void ServerConfig::setHost(const std::string& value) {
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
	_host = value;
}

void ServerConfig::setServerName(const std::vector<std::string>& value) {
	if (_server_name != "")
		throw std::runtime_error("Syntax error: server_name directive is duplicated");
	if (value.size() != 1 || value[0].empty())
		throw std::runtime_error("Syntax error: server_name directive requires exactly one argument");

	for (size_t i = 0; i < value[0].length(); i++)
	{
		if (!std::isalnum(static_cast<unsigned char>(value[0][i])) && value[0][i] != '-' && value[0][i] != '.')
			throw std::runtime_error("Syntax error: server_name must not contain special characters");
	}

	_server_name = value[0];
}

void ServerConfig::setRoot(const std::vector<std::string>& value) {
	if (_has_root)
		throw std::runtime_error("Syntax error: root directive is duplicated");
	if (value.size() != 1)
		throw std::runtime_error("Syntax error: root directive requires exactly one argument");
	if (!isValidPath(value[0]))
		throw std::runtime_error("Syntax error: root directive requires a valid path");
	_root = value[0];
	_has_root = true;
}

void ServerConfig::setErrorPage(const std::vector<std::string>& value) {
	if (value.size() < 2 || value.empty())
		throw std::runtime_error("Syntax error: error_page directive requires two arguments");

	std::string path = value.back();
	long code;
	try {
		for (size_t i = 0; i < value.size() - 1; ++i) {
			char *end;
			errno = 0;
			code = std::strtol(value[i].c_str(), &end, 10);

			if (errno != 0 || *end != '\0' || code < 100 || code > 599)
				throw std::runtime_error("");

			_error_pages[code] = path;
		}
	} catch (...) {
		throw std::runtime_error("Syntax error: error_page directive requires a valid HTTP status code");
	}
}

void ServerConfig::setClientMaxBodySize(const std::vector<std::string>& value) {
	if (_has_client_max_body_size)
		throw std::runtime_error("Syntax error: client_max_body_size directive is duplicated");
	if (value.size() != 1)
		throw std::runtime_error("Syntax error: client_max_body_size directive requires exactly one argument");
	
	try {
		char *end;
		errno = 0;
		unsigned long size = std::strtoul(value[0].c_str(), &end, 10);

		if (errno != 0 || *end != '\0')
			throw std::runtime_error("");

		_client_max_body_size = size;
		_has_client_max_body_size = true;
	} catch (...)
	{
		throw std::runtime_error("Syntax error: client_max_body_size directive requires a valid size argument");
	}
}

bool ServerConfig::isValidPath(const std::string& path) {
	struct stat info;
	if (stat(path.c_str(), &info) != 0 || !S_ISDIR(info.st_mode))
		return false;
	return true;
}

const std::vector<int>& ServerConfig::getPort() const {
	return _port;
}

const std::string ServerConfig::getServerName() const {
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
	os << "Host: " << config.getHost() << "\n";
	os << "Port(s): ";
	const std::vector<int>& ports = config.getPort();
	for (size_t i = 0; i < ports.size(); i++) {
		os << ports[i] << " ";
	}
	os << "\n";
	os << "Server Name: " << config.getServerName() << "\n";
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
