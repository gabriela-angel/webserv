#include "LocationConfig.hpp"

LocationConfig::LocationConfig(const std::vector<std::string>& values) {
	if (values.size() != 2 || (values[0][0] != '/' || values[1] != "{"))
		throw std::runtime_error("Syntax error: Invalid location block definition");
	_path_prefix = values[0];
	//set others to default values
}

LocationConfig::LocationConfig(const LocationConfig& copy) {
	*this = copy;
}

LocationConfig& LocationConfig::operator=(const LocationConfig& other); //missing

LocationConfig::~LocationConfig() {}

void LocationConfig::setDirective(const std::string& key, const std::vector<std::string>& value); //missing