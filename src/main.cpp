#include "Logger.hpp"
#include "EventLoop.hpp"
#include <iostream>
#include <map>

int	main(int ac, char **av)
{
	Logger &logger = Logger::getInstance();
	logger.init(Logger::DEBUG);

	try {
		if (ac != 2)
			throw std::invalid_argument("Usage: ./webserv [config_file]");
				
		EventLoop eventLoop(av[1]);
		eventLoop.run();
	}
	catch (const std::exception &e) {
		std::cerr << RED "Exception: " YELLOW << e.what() << RESET << std::endl;
		return 1;
	}

	return (0);
}