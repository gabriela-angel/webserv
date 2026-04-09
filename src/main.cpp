#include "Logger.hpp"
#include "EventLoop.hpp"
#include <iostream>
#include <map>

int	main(int ac, char **av, char **env)
{
	try {
		if (ac != 2)
			throw std::invalid_argument("Usage: ./webserv [config_file]");
			
		// Initialize Logger
		Logger &logger = Logger::getInstance();
		for (int i = 0; env[i] != NULL; ++i) {
			std::string envVar(env[i]);
			size_t pos = envVar.find("WS_LOG_LEVEL=");
			if (pos != std::string::npos) {
				std::string logLevelStr = envVar.substr(pos + std::string("WS_LOG_LEVEL=").length());
				if (logLevelStr == "NO_LOGS" || logLevelStr == "0")
					logger.init(Logger::NO_LOGS);
				else if (logLevelStr == "FATAL" || logLevelStr == "1")
					logger.init(Logger::FATAL);
				else if (logLevelStr == "CRITICAL" || logLevelStr == "2")
					logger.init(Logger::CRITICAL);
				else if (logLevelStr == "ERROR" || logLevelStr == "3")
					logger.init(Logger::ERROR);
				else if (logLevelStr == "WARNING" || logLevelStr == "4")
					logger.init(Logger::WARNING);
				else if (logLevelStr == "INFO" || logLevelStr == "6")
					logger.init(Logger::INFO);
				else if (logLevelStr == "DEBUG" || logLevelStr == "7")
					logger.init(Logger::DEBUG);
				else
				{
					std::cerr << YELLOW "Warning: Invalid log level in WS_LOG_LEVEL environment variable. Defaulting to DEBUG." RESET << std::endl;
					logger.init(Logger::DEBUG);
				}
				break;
			}
		}
		if (!logger)
		{
			std::cerr << YELLOW "Warning: WS_LOG_LEVEL environment variable not set. Defaulting to DEBUG." RESET << std::endl;
			logger.init(Logger::DEBUG);
		}
		
		// Start Event Loop
		EventLoop eventLoop(av[1]);
		eventLoop.run();
	}
	catch (const std::exception &e) {
		std::cerr << RED "Exception: " YELLOW << e.what() << RESET << std::endl;
		return 1;
	}

	return (0);
}