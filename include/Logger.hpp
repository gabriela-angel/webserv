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
		FATAL,			// For very severe error events that will presumably lead the application to abort immediately
		CRITICAL,		// For severe error events that will presumably lead the application to abort
		ERROR,			// For error events that might still allow the application to continue running
		WARNING,		// For potentially harmful situations that do not cause immediate issues but may require attention
		INFO,			// For general informational messages about the application's operation
		DEBUG,			// For development and debugging purposes
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
	void logDebug(const std::string &message);
	void logInfo(const std::string &message);
	void logWarning(const std::string &message);
	void logError(const std::string &message);
	void logCritical(const std::string &message);
	void logFatal(const std::string &message);
};