#ifndef SERVERCONFIG_HPP
# define SERVERCONFIG_HPP

# include "BaseConfig.hpp"
# include "LocationConfig.hpp"
# include <netdb.h>
# include <cstdlib>
# include <map>
# include <string>
# include <stdexcept>
# include <sstream>
# include <vector>

class ServerConfig : public BaseConfig {
public:
	struct ListenDirective {
		std::string host;
		int port;
		bool default_server;

		ListenDirective() : host("0.0.0.0"), port(80), default_server(false) {}
		ListenDirective(const std::string& host, int port) : host(host), port(port), default_server(false) {}
		ListenDirective(const std::string& host, int port, bool default_server) : host(host), port(port), default_server(default_server) {}
	};

	ServerConfig();
	ServerConfig(const ServerConfig& copy);
	ServerConfig& operator=(const ServerConfig& other);
	~ServerConfig();

	// DEBUG ONLY FOR PORTPAIR
	const std::vector<ListenDirective>& getPortPair() const;
	const std::vector<std::string>& getServerName() const;
	std::vector<LocationConfig>& getLocations();
	const std::vector<LocationConfig>& getLocations() const;


	void setDirective(const std::string& key, const std::vector<std::string>& value);
	void addLocation(const LocationConfig& location);
	bool listensOnPort(int port) const;
	bool isDefaultServer() const;
private:
	std::vector<ListenDirective> _listen;
	std::vector<std::string>	_server_name;
	std::vector<LocationConfig>	_locations;

	typedef void (ServerConfig::*Setter)(const std::vector<std::string>&);
	std::map<std::string, Setter> _setters;

	void initSetters();

	ListenDirective handleAddress(const char *host, const char *port, bool default_server);
	void setListen(const std::vector<std::string>& value);
	void setServerName(const std::vector<std::string>& value);
};

std::ostream& operator<<(std::ostream& os, const ServerConfig& config);

# include "utils.tpp"

#endif