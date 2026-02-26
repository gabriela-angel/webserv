#include "./config/LocationConfig.hpp"

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
		_methods = other._methods;
		_autoindex = other._autoindex;
		_index_files = other._index_files;
		_upload = other._upload;
		_cgi = other._cgi;
		_has_redirect = other._has_redirect;
		_redirect_code = other._redirect_code;
		_redirect_url = other._redirect_url;
		initSetters();
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

const std::string& LocationConfig::getPathPrefix() const {
	return _path_prefix;
}

// debug
std::ostream& operator<<(std::ostream& os, const LocationConfig& config) {
	os << "  Path Prefix: " << config.getPathPrefix() << "\n";
	if (config.getHasRedirect()) {
		os << "  Redirect: " << config.getRedirectCode() << " " << config.getRedirectUrl() << "\n";
	}
	os << "  Root: " << config.getRoot() << "\n";
	os << "  Methods: ";
	for (size_t i = 0; i < config.getMethods().size(); i++) {
		os << config.getMethods()[i] << " ";
	}
	os << "\n";
	os << "  Autoindex: " << (config.getAutoindex() ? "on" : "off") << "\n";
	if (!config.getIndexFiles().empty()) {
		os << "  Index Files: ";
		for (size_t i = 0; i < config.getIndexFiles().size(); i++) {
			os << config.getIndexFiles()[i] << " ";
		}
		os << "\n";
	}
	if (config.getIsCgi()) {
		os << "  CGI:\n";
		const std::map<std::string, std::string>& cgi = config.getCgi();
		for (std::map<std::string, std::string>::const_iterator it = cgi.begin(); it != cgi.end(); ++it) {
			os << "    Extension: " << it->first << ", Executable: " << it->second << "\n";
		}
	}
	if (!config.getUpload().empty()) {
		os << "  Upload: " << config.getUpload() << "\n";
	}
	return os;
}