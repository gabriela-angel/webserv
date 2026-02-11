#pragma once

#include "colors.hpp"
#include <ctime>
#include <fstream>
#include <iostream>
#include <unistd.h>

#define LOGS_PATH "./logs/"
#define DEFAULT_LOG_FILE "webserv.log"

class Logger
{
  public:
	// LOG LEVELS
	enum		LogLevel
	{
		INFO,
		ERROR,
		DEBUG,
	};

  private:
	// Files
	LogLevel	_logLevel;
	std::ofstream _logFile;
	static bool	_initialized;

	// Private Constructors (Singleton Pattern)
	Logger(){};
	Logger(const Logger &){};
	Logger &operator=(const Logger &)
	{
		return (*this);
	};

	/* Private Methods */
	static bool _isTerminal(std::ostream &os);
	static std::string _currentDateTime(void);

  public:
	// Destructor
	~Logger();

	// Initializer
	void init(LogLevel level);
	void init(LogLevel level, const std::string &filePath);

	// Get the singleton instance of the Logger
	static Logger &getInstance();

	// Logging Methods
	void logInfo(const std::string &message);
	void logError(const std::string &message);
	void logDebug(const std::string &message);
};