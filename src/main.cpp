#include "Logger.hpp"
#include "ManagerConfig.hpp"
#include "ParseConfig.hpp"

int	main(int ac, char **av)
{
	(void)av;
	try
	{
		Logger &logger = Logger::getInstance();
		logger.init(Logger::DEBUG);

		logger.logInfo("Hello, World!");
		logger.logError("This is an error message.");
		logger.logDebug("This is a debug message.");
	} catch (const std::exception &e)
	{
		std::cerr << RED "Exception: " YELLOW << e.what() << RESET << std::endl;
	}

	//  Your program must use a configuration file, provided as an argument on the command line, or available in a default path.
	try {
		if (ac != 2)
			throw std::invalid_argument("Usage: ./webserv [config_file]");

		ParseConfig parser(av[1]);
		std::vector<ServerConfig> servers = parser.parse();

		ManagerConfig basicConfig(servers);
	}
	catch (const std::exception &e) {
		Logger &logger = Logger::getInstance();
		logger.logError(e.what());
		return 1;
	}
	

	return (0);
}