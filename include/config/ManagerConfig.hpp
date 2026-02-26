#ifndef MANAGERCONFIG_HPP
# define MANAGERCONFIG_HPP

# include <algorithm>
# include "./config/ServerConfig.hpp"
# include <stdexcept>
# include <vector>

class ManagerConfig {
private:
	std::vector<ServerConfig> _servers;

public:
	ManagerConfig(const std::vector<ServerConfig>& servers);
	ManagerConfig(const ManagerConfig& copy);
	ManagerConfig& operator=(const ManagerConfig& other);
	~ManagerConfig();

	ServerConfig& findServer(int port, const std::string& host_header);

	// DEBUG ONLY
	const std::vector<ServerConfig>& getServers() const { return _servers; }
};

std::ostream& operator<<(std::ostream& os, const ManagerConfig& config);

#endif

// Quando você permite múltiplos servers, você precisa definir um default server por porta
// Exemplo:
// server {
//     listen 8080;
//     server_name example.com;
// }
// server {
//     listen 8080;
// }

// Se vier:

// GET / HTTP/1.1
// Host: unknown.com

// Quem responde?
//  O primeiro server com listen 8080.