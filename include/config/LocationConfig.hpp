#ifndef LOCATIONCONFIG_HPP
# define LOCATIONCONFIG_HPP

# include <string>
# include <vector>
# include <map>
# include <stdexcept> 

class LocationConfig {
private:
	std::string _path_prefix; // (/images)
	std::string _root; // (optional)
	// create enum for methods
	std::vector<std::string> _allowed_methods;
	bool _autoindex;
	std::vector<std::string> _index_files;
	bool _upload_enabled;
	std::string _upload_path;
	std::map<std::string, std::string> _cgi; // ex: cgi .php /usr/bin/php-cgi;
	bool _has_redirect;
	int _redirect_code;
	std::string _redirect_url;
	
	LocationConfig(const LocationConfig& copy);
	LocationConfig& operator=(const LocationConfig& other);
public:
	LocationConfig(const std::vector<std::string>& values);
	~LocationConfig();

	void setDirective(const std::string& key, const std::vector<std::string>& value);
	const std::string& getPathPrefix() const;
};

#endif