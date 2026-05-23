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

	// DEBUG ONLY
	const std::vector<ServerConfig>& getServers() const { return _servers; }
};

std::ostream& operator<<(std::ostream& os, const ManagerConfig& config);

#endif
