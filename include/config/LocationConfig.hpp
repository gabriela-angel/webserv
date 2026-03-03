#ifndef LOCATIONCONFIG_HPP
# define LOCATIONCONFIG_HPP

# include "./config/BaseConfig.hpp"
# include <cstdlib>
# include <map>
# include <string>
# include <stdexcept>
# include <sys/stat.h>
# include <vector>
# include <unistd.h>
// DEBUG ONLY
# include <iostream>


class LocationConfig : public BaseConfig {
private:
	std::string							_path_prefix;
	std::string							_upload;
	bool								_is_cgi;
	std::map<std::string, std::string>	_cgi;

	typedef void (LocationConfig::*Setter)(const std::vector<std::string>&);
	std::map<std::string, Setter> _setters;

	void initSetters();
	void setUpload(const std::vector<std::string>& value);
	void setCgi(const std::vector<std::string>& value);

public:
	LocationConfig(const std::vector<std::string>& values);
	LocationConfig(const LocationConfig& copy);
	LocationConfig& operator=(const LocationConfig& other);
	~LocationConfig();

	void setDirective(const std::string& key, const std::vector<std::string>& value);
	const std::string& getPathPrefix() const;

	// DEBUG ONLY
	const std::string& getUpload() const { return _upload; }
	const bool& getIsCgi() const { return _is_cgi; }
	const std::map<std::string, std::string>& getCgi() const { return _cgi; }
};

std::ostream& operator<<(std::ostream& os, const LocationConfig& config);

#endif