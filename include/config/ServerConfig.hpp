#ifndef SERVERCONFIG_HPP
# define SERVERCONFIG_HPP

# include "./config/LocationConfig.hpp"
# include <cerrno>
# include <cstdlib>
# include <map>
# include <string>
# include <stdexcept>
# include <sstream>
# include <sys/stat.h>
# include <vector>

# define DEFAULT_CLIENT_MAX_BODY_SIZE 1048576 // CHECK MACRO LUIZ LATER

class ServerConfig {
private:
	bool						_has_port;
	std::vector<int>			_port;
	std::string					_host; // ip address
	std::string					_server_name;
	bool						_has_root;
	std::string _root; //might be on location -- either make it mndatory on config or provide a default
	std::map<int, std::string>	_error_pages; //if not defined we need to return default ones
	bool						_has_client_max_body_size;
	size_t						_client_max_body_size; //currently not accepting suffixes like 1M, 500K, etc. but we can add that later
	std::vector<LocationConfig>	_locations;

	typedef void (ServerConfig::*Setter)(const std::vector<std::string>&);
	std::map<std::string, Setter> _setters;

	void initSetters();

	void setHost(const std::string& value);
	void setListen(const std::vector<std::string>& value);
	void setServerName(const std::vector<std::string>& value);
	void setRoot(const std::vector<std::string>& value);
	void setErrorPage(const std::vector<std::string>& value);
	void setClientMaxBodySize(const std::vector<std::string>& value);

	bool isValidPath(const std::string& path); // utility func that I also use in location config, maybe move it to a common utils file later
public:
	ServerConfig();
	ServerConfig(const ServerConfig& copy);
	ServerConfig& operator=(const ServerConfig& other);
	~ServerConfig();

	const std::vector<int>& getPort() const;
	const std::string getServerName() const;
	std::vector<LocationConfig>& getLocations();
	const std::vector<LocationConfig>& getLocations() const;


	void setDirective(const std::string& key, const std::vector<std::string>& value);
	void addLocation(const LocationConfig& location);

	// DEBUG ONLY
	const std::string& getHost() const { return _host; }
	const std::string& getRoot() const { return _root; }
	const std::map<int, std::string>& getErrorPages() const { return _error_pages; }
	const size_t& getClientMaxBodySize() const { return _client_max_body_size; }
};

std::ostream& operator<<(std::ostream& os, const ServerConfig& config);

#endif