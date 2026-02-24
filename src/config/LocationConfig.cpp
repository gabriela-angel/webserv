#include "LocationConfig.hpp"

LocationConfig::LocationConfig(const std::vector<std::string>& values) {
	if (values.size() != 2 || (values[0][0] != '/' || values[1] != "{"))
		throw std::runtime_error("Syntax error: Invalid location block definition");
	
	_path_prefix = values[0];
	_has_root = false;
	_root = "";
	_methods.clear();
	_autoindex = false;
	_index_files.clear();
	_upload = "";
	_is_cgi = false;
	_cgi.clear();
	_has_redirect = false;
	_redirect_code = 0;
	_redirect_url = "";

	initSetters();
}

LocationConfig::LocationConfig(const LocationConfig& copy) {
	*this = copy;
}

LocationConfig& LocationConfig::operator=(const LocationConfig& other) {
	if (this != &other) {
		_path_prefix = other._path_prefix;
		_has_root = other._has_root;
		_root = other._root;
		_allowed_methods = other._allowed_methods;
		_autoindex = other._autoindex;
		_index_files = other._index_files;
		_upload_enabled = other._upload_enabled;
		_upload_path = other._upload_path;
		_cgi = other._cgi;
		_has_redirect = other._has_redirect;
		_redirect_code = other._redirect_code;
		_redirect_url = other._redirect_url;
	}
	return *this;
}

LocationConfig::~LocationConfig() {}

void LocationConfig::initSetters() {
	_setters["root"] = &LocationConfig::setRoot;
	_setters["methods"] = &LocationConfig::setMethods;
	_setters["autoindex"] = &LocationConfig::setAutoindex;
	_setters["index"] = &LocationConfig::setIndexFiles;
	_setters["upload"] = &LocationConfig::setUpload;
	_setters["cgi"] = &LocationConfig::setCgi;
	_setters["return"] = &LocationConfig::setRedirect;
}

void LocationConfig::setRoot(const std::vector<std::string>& value) {
	if (_has_root)
		throw std::runtime_error("Syntax error: root directive is duplicated");
	if (value.size() != 1)
		throw std::runtime_error("Syntax error: root directive requires exactly one argument");
	if (!isValidPath(value[0]))
		throw std::runtime_error("Syntax error: root directive requires a valid path");
	_root = value[0];
	_has_root = true;
}

void LocationConfig::setMethods(const std::vector<std::string>& value) {
	if (value.empty())
		throw std::runtime_error("Syntax error: methods directive requires at least one argument");
	for (size_t i = 0; i < value.size(); i++) {
		std::string method = toUpper(value[i]);
		if (method != "GET" && method != "POST" && method != "DELETE")
			throw std::runtime_error("Syntax error: methods directive only accepts GET, POST, and DELETE as arguments");
		_methods.push_back(method);
	}
}

void LocationConfig::setAutoindex(const std::vector<std::string>& value) {
	if (value.size() != 1)
		throw std::runtime_error("Syntax error: autoindex directive requires exactly one argument");
	if (value[0] == "on")
		_autoindex = true;
	else if (value[0] == "off")
		_autoindex = false;
	else
		throw std::runtime_error("Syntax error: autoindex directive requires 'on' or 'off' as argument");
}

void LocationConfig::setIndexFiles(const std::vector<std::string>& value) {
	if (value.empty())
		throw std::runtime_error("Syntax error: index directive requires at least one argument");
	_index_files = value;
}

void LocationConfig::setUpload(const std::vector<std::string>& value) {
	if (value.size() != 1)
		throw std::runtime_error("Syntax error: upload directive requires exactly one argument");
	if (!isValidPath(value[0]))
		throw std::runtime_error("Syntax error: upload directive requires a valid path");
	_upload = value[0];
}

void LocationConfig::setCgi(const std::vector<std::string>& value) {
	if (value.size() != 2)
		throw std::runtime_error("Syntax error: cgi directive requires exactly two arguments");
	if (value[0][0] != '.')
		throw std::runtime_error("Syntax error: cgi directive requires a file extension as the first argument");
	if (!isValidPath(value[1]))
		throw std::runtime_error("Syntax error: cgi directive requires a valid directory as the second argument");
	_is_cgi = true;
	_cgi[value[0]] = value[1];
}

void LocationConfig::setRedirect(const std::vector<std::string>& value) {
	if (value.size() > 2 || value.empty())
		throw std::runtime_error("Syntax error: return directive requires exactly two arguments");
	
	int code;
	try {
		std::size_t pos;
		code = std::stoi(value[0], &pos);

		if (pos != value[0].length() || code < 100 || code > 599)
			throw std::runtime_error("");
	} catch (...) {
		throw std::runtime_error("Syntax error: return directive requires a valid HTTP redirection status code");
	}

	_has_redirect = true;
	_redirect_code = code;
	if (value.size() == 2)
		_redirect_url = value[1];
}

std::string LocationConfig::toUpper(std::string str)
{
	for (size_t i = 0; i < str.length(); i++)
		str[i] = static_cast<char>(std::toupper(static_cast<unsigned char>(str[i])));
	return str;
}

bool LocationConfig::isValidPath(const std::string& path) {
	struct stat info;
	if (stat(path.c_str(), &info) != 0 || !S_ISDIR(info.st_mode))
		return false;
	return true;
}

void LocationConfig::setDirective(const std::string& key, const std::vector<std::string>& value) {
	std::map<std::string, Setter>::iterator it = _setters.find(key);

	if (it == _setters.end())
		throw std::runtime_error("Unknown directive: " + key);

	(this->*(it->second))(value);
}