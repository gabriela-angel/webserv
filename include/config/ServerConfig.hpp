#ifndef SERVERCONFIG_HPP
# define SERVERCONFIG_HPP

# include "./config/BaseConfig.hpp"
# include "./config/LocationConfig.hpp"
# include <cstdlib>
# include <map>
# include <string>
# include <stdexcept>
# include <sstream>
# include <vector>

class ServerConfig : public BaseConfig {
private:
	std::map<int, std::string>	_listen;
	std::vector<std::string>	_server_name;
	std::vector<LocationConfig>	_locations;

	typedef void (ServerConfig::*Setter)(const std::vector<std::string>&);
	std::map<std::string, Setter> _setters;

	void initSetters();

	void setListen(const std::vector<std::string>& value);
	void setHost(const std::string& value, int port);
	void setServerName(const std::vector<std::string>& value);
public:
	ServerConfig();
	ServerConfig(const ServerConfig& copy);
	ServerConfig& operator=(const ServerConfig& other);
	~ServerConfig();

	// DEBUG ONLY FOR PORTPAIR
	const std::map<int, std::string>& getPortPair() const;
	const std::vector<std::string> getServerName() const;
	std::vector<LocationConfig>& getLocations();
	const std::vector<LocationConfig>& getLocations() const;


	void setDirective(const std::string& key, const std::vector<std::string>& value);
	void addLocation(const LocationConfig& location);
	bool listensOnPort(int port) const;
};

std::ostream& operator<<(std::ostream& os, const ServerConfig& config);

#endif