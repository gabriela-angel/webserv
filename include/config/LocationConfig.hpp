#ifndef LOCATIONCONFIG_HPP
# define LOCATIONCONFIG_HPP

# include <cerrno>
# include <cstdlib>
# include <map>
# include <string>
# include <stdexcept>
# include <sys/stat.h>
# include <vector>
// DEBUG ONLY
# include <iostream>


class LocationConfig {
private:
	std::string							_path_prefix;
	bool								_has_root;
	std::string							_root;
	// create enum for methods ?
	std::vector<std::string>			_methods;
	bool								_autoindex;
	std::vector<std::string>			_index_files;
	std::string							_upload;
	bool								_is_cgi;
	std::map<std::string, std::string>	_cgi;
	// ex: cgi .php /usr/bin/php-cgi;
	bool								_has_redirect;
	int									_redirect_code;
	std::string							_redirect_url;

	typedef void (LocationConfig::*Setter)(const std::vector<std::string>&);
	std::map<std::string, Setter> _setters;

	void initSetters();
	void setRoot(const std::vector<std::string>& value);
	void setMethods(const std::vector<std::string>& value);
	void setAutoindex(const std::vector<std::string>& value);
	void setIndexFiles(const std::vector<std::string>& value);
	void setUpload(const std::vector<std::string>& value);
	void setCgi(const std::vector<std::string>& value);
	void setRedirect(const std::vector<std::string>& value);

	std::string toUpper(std::string str); // utility func
	bool isValidPath(const std::string& path); // utility func that I also use in server config, maybe move it to a common utils file later

	public:
	LocationConfig(const std::vector<std::string>& values);
	LocationConfig(const LocationConfig& copy);
	LocationConfig& operator=(const LocationConfig& other);
	~LocationConfig();

	void setDirective(const std::string& key, const std::vector<std::string>& value);
	const std::string& getPathPrefix() const;

	// DEBUIG ONLY
	const std::string& getRoot() const { return _root; }
	const std::vector<std::string>& getMethods() const { return _methods; }
	const bool& getAutoindex() const { return _autoindex; }
	const std::vector<std::string>& getIndexFiles() const { return _index_files; }
	const std::string& getUpload() const { return _upload; }
	const bool& getIsCgi() const { return _is_cgi; }
	const std::map<std::string, std::string>& getCgi() const { return _cgi; }
	const bool& getHasRedirect() const { return _has_redirect; }
	const int& getRedirectCode() const { return _redirect_code; }
	const std::string& getRedirectUrl() const { return _redirect_url; }
};

std::ostream& operator<<(std::ostream& os, const LocationConfig& config);

#endif