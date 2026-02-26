#include "Logger.hpp"
#include "./config/ManagerConfig.hpp"
#include "./config/ParseConfig.hpp"

int	main(int ac, char **av)
{
	(void)av;
	// try
	// {
	// 	Logger &logger = Logger::getInstance();
	// 	logger.init(Logger::DEBUG);

	// 	logger.logInfo("Hello, World!");
	// 	logger.logError("This is an error message.");
	// 	logger.logDebug("This is a debug message.");
	// } catch (const std::exception &e)
	// {
	// 	std::cerr << RED "Exception: " YELLOW << e.what() << RESET << std::endl;
	// }

	Logger &logger = Logger::getInstance();
	logger.init(Logger::DEBUG);

	try {
		if (ac != 2)
			throw std::invalid_argument("Usage: ./webserv [config_file]");

		ParseConfig parser(av[1]);
		std::vector<ServerConfig> servers = parser.parse();

		ManagerConfig basicConfig(servers);
		std::cout << basicConfig << std::endl;
	}
	catch (const std::exception &e) {
		Logger &logger = Logger::getInstance();
		logger.logError(e.what());
		return 1;
	}
	

	return (0);
}