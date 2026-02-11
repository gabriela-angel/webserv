
#include "Logger.hpp"

// Singleton instance
Logger &Logger::getInstance()
{
	static Logger instance;
	return instance;
}

Logger::~Logger()
{
	if (_logFile.is_open()) _logFile.close();
}

bool Logger::_initialized = false;
void Logger::init(LogLevel level, const std::string &filePath)
{
	if (_initialized)
		throw std::runtime_error("Logger has already been initialized.");
	_initialized = true;
	_logLevel = level;
	std::string fullLogPath = LOGS_PATH + filePath;
	_logFile.open(fullLogPath.c_str(), std::ios::app | std::ios::out);

	if (!_logFile.is_open()) throw std::runtime_error("Failed to open " + fullLogPath);
	std::string levelStr;
	switch (level)
	{
	case INFO:
		levelStr = "[INFO]";
		break;
	case ERROR:
		levelStr = "[ERROR]";
		break;
	case DEBUG:	
		levelStr = "[DEBUG]";
		break;
	default:
		levelStr = "[UNKNOWN]";
		break;
	}
	logInfo("Logger initialized with log level: " + levelStr);
}

void Logger::init(LogLevel level)
{
	init(level, DEFAULT_LOG_FILE);
}

bool Logger::_isTerminal(std::ostream &os)
{
	if (&os == &std::cout)
		return (isatty(fileno(stdout)));
	if (&os == &std::cerr)
		return (isatty(fileno(stderr)));
	return false;
}

std::string Logger::_currentDateTime(void)
{
	time_t now = time(NULL);
	char buf[20];
	std::strftime(buf, sizeof(buf), "%Y-%m-%d %X", localtime(&now));
	return std::string(buf);
}

inline static std::string colorizeMessage(const std::string &dateTime, const std::string &severity, const std::string &message)
{
	std::string coloredDateTime = COLORIZE(GRAY, dateTime);
	std::string coloredSeverity;

	if (severity == "[DEBUG]")
		coloredSeverity = COLORIZE(MAGENTA, severity);
	else if (severity == "[INFO]")
		coloredSeverity = COLORIZE(CYAN, severity);
	else if (severity == "[ERROR]")
		coloredSeverity = COLORIZE(RED, severity);
	else
		coloredSeverity = severity;

	return coloredDateTime + " - " + coloredSeverity + " " + message;
}

void Logger::logInfo(const std::string &message)
{
	if (!_initialized) throw std::runtime_error("Logger is not initialized. Call init() before logging.");
	if (_logLevel < INFO) return;
	std::string dateTime = _currentDateTime();
	std::string severity = "[INFO]";
	std::string logMessage = dateTime + " - " + severity + " " + message;
	std::string coloredMessage = colorizeMessage(dateTime, severity, message);
	if (_isTerminal(std::cout))
		std::cout << coloredMessage << std::endl;
	_logFile << logMessage << std::endl;
}

void Logger::logError(const std::string &message)
{
	if (!_initialized) throw std::runtime_error("Logger is not initialized. Call init() before logging.");
	if (_logLevel < ERROR) return;
	std::string dateTime = _currentDateTime();
	std::string severity = "[ERROR]";
	std::string logMessage = dateTime + " - " + severity + " " + message;
	std::string coloredMessage = colorizeMessage(dateTime, severity, message);
	if (_isTerminal(std::cerr))
		std::cerr << coloredMessage << std::endl;
	_logFile << logMessage << std::endl;
}

void Logger::logDebug(const std::string &message)
{
	if (!_initialized) throw std::runtime_error("Logger is not initialized. Call init() before logging.");
	if (_logLevel < DEBUG) return;
	std::string dateTime = _currentDateTime();
	std::string severity = "[DEBUG]";
	std::string logMessage = dateTime + " - " + severity + " " + message;
	std::string coloredMessage = colorizeMessage(dateTime, severity, message);
	if (_isTerminal(std::cout))
		std::cout << coloredMessage << std::endl;
	_logFile << logMessage << std::endl;
}