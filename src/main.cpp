#include "Logger.hpp"
#include "EventLoop.hpp"

int	main(void)
{
	try
	{
		Logger &logger = Logger::getInstance();
		logger.init(Logger::DEBUG);

		EventLoop eventLoop("./config/default.conf");
		eventLoop.run();

	} catch (const std::exception &e)
	{
		std::cerr << RED "Exception: " YELLOW << e.what() << RESET << std::endl;
		return (1);
	}

	return (0);
}