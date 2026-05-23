#ifndef PARSECONFIG_HPP
# define PARSECONFIG_HPP

# include "./config/ServerConfig.hpp"
# include "./utils.hpp"
# include <fstream>
# include <sstream>
# include <stdexcept>
# include <vector>

class ParseConfig {
private:
	enum Context {
			GLOBAL,
			SERVER,
			LOCATION
		};

	std::string _filename;
	int _line_counter;
	Context _context;

	ParseConfig(const ParseConfig& copy);
	ParseConfig& operator=(const ParseConfig& other);

	void cutComments(std::string& line);

	bool setKeyValue(const std::string& line, std::string& key, std::vector<std::string>& value);
	bool setContext(const std::string& key, const std::vector<std::string>& value, std::vector<ServerConfig>& servers);
	void parseDirective(const std::string& key, const std::vector<std::string>& value, std::vector<ServerConfig>& servers);
public:
	ParseConfig(const std::string& filename);
	~ParseConfig();

	std::vector<ServerConfig> parse();
};

#endif
