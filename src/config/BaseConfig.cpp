#include "./config/BaseConfig.hpp"

BaseConfig::BaseConfig() {
	_has_root = false;
	_root = "";
	_methods.clear();
	_has_autoindex = false;
	_autoindex = false;
	_has_index_files = false;
	_index_files.clear();
	_error_pages.clear();
	_has_redirect = false;
	_redirect_code = 0;
	_redirect_url = "";
	_has_client_max_body_size = false;
	_client_max_body_size = DEFAULT_CLIENT_MAX_BODY_SIZE;
}

BaseConfig::BaseConfig(const BaseConfig& copy) {
	_has_root = copy._has_root;
	_root = copy._root;
	_methods = copy._methods;
	_autoindex = copy._autoindex;
	_index_files = copy._index_files;
	_error_pages = copy._error_pages;
	_has_redirect = copy._has_redirect;
	_redirect_code = copy._redirect_code;
	_redirect_url = copy._redirect_url;
	_has_client_max_body_size = copy._has_client_max_body_size;
	_client_max_body_size = copy._client_max_body_size;
}

BaseConfig& BaseConfig::operator=(const BaseConfig& other) {
	if (this != &other) {
		_has_root = other._has_root;
		_root = other._root;
		_methods = other._methods;
		_autoindex = other._autoindex;
		_index_files = other._index_files;
		_error_pages = other._error_pages;
		_has_redirect = other._has_redirect;
		_redirect_code = other._redirect_code;
		_redirect_url = other._redirect_url;
		_has_client_max_body_size = other._has_client_max_body_size;
		_client_max_body_size = other._client_max_body_size;
	}
	return *this;
}

BaseConfig::~BaseConfig() {}

void BaseConfig::setRoot(const std::vector<std::string>& value) {
	if (_has_root)
		throw std::runtime_error("Syntax error: root directive is duplicated");
	if (value.size() != 1)
		throw std::runtime_error("Syntax error: root directive requires exactly one argument");
	if (!isValidDirectory(value[0]))
		throw std::runtime_error("Syntax error: root directive requires a valid directory");
	_root = value[0];
	_has_root = true;
}

void BaseConfig::setMethods(const std::vector<std::string>& value) {
	if (value.empty())
		throw std::runtime_error("Syntax error: methods directive requires at least one argument");
	std::vector<std::string> new_methods;
	for (size_t i = 0; i < value.size(); i++) {
		std::string method = toUpper(value[i]);
		if (method != "GET" && method != "POST" && method != "DELETE")
			throw std::runtime_error("Syntax error: methods directive only accepts GET, POST, and DELETE as arguments");
		if (std::find(new_methods.begin(), new_methods.end(), method) != new_methods.end())
			throw std::runtime_error("Syntax error: methods directive contains duplicate method '" + method + "'");
		new_methods.push_back(method);
	}
	_methods = new_methods;
}

void BaseConfig::setAutoindex(const std::vector<std::string>& value) {
	if (value.size() != 1)
		throw std::runtime_error("Syntax error: autoindex directive requires exactly one argument");
	if (_has_autoindex)
		throw std::runtime_error("Syntax error: autoindex directive is duplicated");
	if (value[0] == "on")
		_autoindex = true;
	else if (value[0] == "off")
		_autoindex = false;
	else
		throw std::runtime_error("Syntax error: autoindex directive requires 'on' or 'off' as argument");
	_has_autoindex = true;
}

void BaseConfig::setIndexFiles(const std::vector<std::string>& value) {
	if (value.empty())
		throw std::runtime_error("Syntax error: index directive requires at least one argument");
	if (_has_index_files)
		throw std::runtime_error("Syntax error: index directive is duplicated");
	_index_files = value;
	_has_index_files = true;
}

void BaseConfig::setErrorPage(const std::vector<std::string>& value) {
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

void BaseConfig::setRedirect(const std::vector<std::string>& value) {
	if (value.size() > 2 || value.empty())
		throw std::runtime_error("Syntax error: return directive requires exactly two arguments");
	
	long code;
	try {
		char *end;
		errno = 0;
		code = std::strtol(value[0].c_str(), &end, 10);

		if (errno != 0 || *end != '\0' || code < 100 || code > 599)
			throw std::runtime_error("");
	} catch (...) {
		throw std::runtime_error("Syntax error: return directive requires a valid HTTP redirection status code");
	}

	_has_redirect = true;
	_redirect_code = code;
	if (value.size() == 2)
		_redirect_url = value[1];
}

void BaseConfig::setClientMaxBodySize(const std::vector<std::string>& value) {
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

const std::string& BaseConfig::getRoot() const {
	return _root;
}

const std::vector<std::string>& BaseConfig::getMethods() const {
	return _methods;
}

const bool& BaseConfig::getAutoindex() const {
	return _autoindex;
}

const std::vector<std::string>& BaseConfig::getIndexFiles() const {
	return _index_files;
}

const std::map<int, std::string>& BaseConfig::getErrorPages() const {
	return _error_pages;
}

const int& BaseConfig::getRedirectCode() const {
	return _redirect_code;
}

const std::string& BaseConfig::getRedirectUrl() const {
	return _redirect_url;
}

const size_t& BaseConfig::getClientMaxBodySize() const {
	return _client_max_body_size;
}

const bool& BaseConfig::hasRoot() const {
	return _has_root;
}

const bool& BaseConfig::hasAutoindex() const {
	return _has_autoindex;
}

const bool& BaseConfig::hasIndexFiles() const {
	return _has_index_files;
}

const bool& BaseConfig::hasRedirect() const {
	return _has_redirect;
}