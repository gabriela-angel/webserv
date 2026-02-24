#ifndef SERVERCONFIG_HPP
# define SERVERCONFIG_HPP

# include <vector>
# include <string>
# include <map>
# include <LocationConfig.hpp>
# include <stdexcept>
# include <sstream>
# include <sys/stat.h>

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
	LocationConfig& getLastLocation();

	void setDirective(const std::string& key, const std::vector<std::string>& value);
	void addLocation(const LocationConfig& location);
};

#endif