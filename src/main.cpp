#include "Logger.hpp"

int	main(void)
{
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

	return (0);
}