#include "./config/LocationConfig.hpp"

LocationConfig::LocationConfig(const std::vector<std::string>& values) : BaseConfig() {
	if (values.size() != 2 || (values[0][0] != '/' || values[1] != "{"))
		throw std::runtime_error("Syntax error: Invalid location block definition");
	
	if (values[0].length() > 1 && values[0][values[0].length() - 1] == '/')
		_path_prefix = values[0].substr(0, values[0].length() - 1);
	else
		_path_prefix = values[0];
	_upload = "";
	_is_cgi = false;
	_cgi.clear();

	initSetters();
}

LocationConfig::LocationConfig(const LocationConfig& copy) : BaseConfig(copy) {
	_path_prefix = copy._path_prefix;
	_upload = copy._upload;
	_is_cgi = copy._is_cgi;
	_cgi = copy._cgi;

	initSetters();
}

LocationConfig& LocationConfig::operator=(const LocationConfig& other) {
	if (this != &other) {
		BaseConfig::operator=(other);

		_path_prefix = other._path_prefix;
		_upload = other._upload;
		_cgi = other._cgi;
		_is_cgi = other._is_cgi;
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
	_setters["error_page"] = &LocationConfig::setErrorPage;
	_setters["upload"] = &LocationConfig::setUpload;
	_setters["cgi"] = &LocationConfig::setCgi;
	_setters["return"] = &LocationConfig::setRedirect;
	_setters["client_max_body_size"] = &LocationConfig::setClientMaxBodySize;
}

void LocationConfig::setUpload(const std::vector<std::string>& value) {
	if (value.size() != 1)
		throw std::runtime_error("Syntax error: upload directive requires exactly one argument");
	if (!isValidDirectory(value[0]))
		throw std::runtime_error("Syntax error: upload directive requires a valid path");
	_upload = value[0];
}

void LocationConfig::setCgi(const std::vector<std::string>& value) {
	if (value.size() != 2)
		throw std::runtime_error("Syntax error: cgi directive requires exactly two arguments");
	if (value[0].length() < 2 || value[0][0] != '.')
		throw std::runtime_error("Syntax error: cgi directive requires a file extension as the first argument");
	if (!isValidFile(value[1]) || access(value[1].c_str(), X_OK) != 0)
		throw std::runtime_error("Syntax error: cgi directive requires a valid executable path as the second argument");
	if (_cgi.find(value[0]) != _cgi.end())
		throw std::runtime_error("Syntax error: cgi directive has duplicate file extension");
	_is_cgi = true;
	_cgi[value[0]] = value[1];
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
	if (config.hasRedirect()) {
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