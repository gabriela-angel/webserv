#include "ServerConfig.hpp"

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
	}
	return *this;
}

ServerConfig::~ServerConfig(){}

void ServerConfig::initSetters() {
	_setters["listen"] = &ServerConfig::setListen;
	_setters["server_name"] = &ServerConfig::setServerName; //missing
	_setters["root"] = &ServerConfig::setRoot; //missing
	_setters["error_page"] = &ServerConfig::setErrorPage; //missing
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
			setHost(std::vector<std::string>(1, host_part));
		}
		std::size_t pos;
		unsigned int port = std::stoi(port_part, &pos);

		if (pos != port_part.length() || port < 1 || port > 65535)
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

			std::size_t pos;
			int num = std::stoi(octect, &pos);

			if (pos != octect.length() || num < 0 || num > 255)
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
	int code;
	try {
		for (size_t i = 0; i < value.size() - 1; ++i) {
			std::size_t pos;
			code = std::stoi(value[i], &pos);

			if (pos != value[i].length() || code < 100 || code > 599)
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
		std::size_t pos;
		unsigned long size = std::stoul(value[0], &pos);

		if (pos != value[0].length())
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

LocationConfig& ServerConfig::getLastLocation() {
	if (_locations.empty())
		throw std::runtime_error("No locations defined for this server");
	return _locations.back();
}

void ServerConfig::setDirective(const std::string& key, const std::vector<std::string>& value) {
	std::map<std::string, Setter>::iterator it = _setters.find(key);

	if (it == _setters.end())
		throw std::runtime_error("Unknown directive: " + key);

	(this->*(it->second))(value);
}

void ServerConfig::addLocation(const LocationConfig& location) {
	for (size_t i = 0; i < _locations.size(); ++i) {
		if (_locations[i].getPathPrefix() == location.getPathPrefix())
			throw std::runtime_error("Syntax error: duplicate location path '" + location.getPathPrefix() + "'");
	}

	_locations.push_back(location);
}