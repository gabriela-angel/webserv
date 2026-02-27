#include "EventLoop.hpp"

EventLoop::~EventLoop() {}
EventLoop::EventLoop(const char *configFilePath) : _logger(Logger::getInstance()), _serverManager()
{
	(void)configFilePath;
}

std::sig_atomic_t	_stopFlag = 0;
void handle_sigint(int)
{
	Logger::getInstance().logInfo("SIGINT received, shutting down gracefully...");
	_stopFlag = 1;
}

void EventLoop::run(){
	std::signal(SIGINT, handle_sigint);
	while (!_stopFlag)
	{
		// Waint for events on servers and clients
		int nfds = epoll_wait(_serverManager.getEpollFD(), _events, MAX_EVENTS, 1000); // 1000 ms timeout
		if (_stopFlag) break; // Check if SIGINT was received during epoll_wait
		if (nfds == -1) throw std::runtime_error("Failed to wait for epoll events");

		// Process events
		for (int i = 0; i < nfds; ++i)
		{
			if (_serverManager.isServerSocket(_events[i].data.fd))
			{
				_logger.logInfo("Accepting new connection on server socket: " + to_string(_events[i].data.fd));
				_serverManager.acceptConnection(_events[i].data.fd);
				continue;
			}
			
			// -------------------------------- Client socket events --------------------------------

			// 1️⃣ Check for errors first
			if (_events[i].events & EPOLLERR)
			{
				_logger.logWarning("Error on client socket: " + to_string(_events[i].data.fd));
				_serverManager.removeClient(_events[i].data.fd);
				continue;
			}

			// 2️⃣ Handle readable
			if (_events[i].events & EPOLLIN)
			{
				_logger.logInfo("Data available to read on socket: " + to_string(_events[i].data.fd));
				_serverManager.handleRead(_events[i].data.fd);
			}

			// 3️⃣ Handle writable
			if (_events[i].events & EPOLLOUT)
			{
				_logger.logInfo("Ready to write on socket: " + to_string(_events[i].data.fd));
				_serverManager.handleWrite(_events[i].data.fd);
			}
		}

		// Handle Timeout for clients
		const std::map<int, ClientData> &clientMap = _serverManager.getClientMap();
		std::vector<int> clientsToRemove;
		std::time_t currentTime = std::time(0);
		for (std::map<int, ClientData>::const_iterator it = clientMap.begin(); it != clientMap.end(); ++it)
		{
			const ClientData &client = it->second;
			const bool isFirstRead = !client.stateMachine.firstReadFlag;
			const std::time_t elapsedTime = currentTime - client.stateMachine.lastActivityTime;
			if (elapsedTime > (isFirstRead ? CLI_FIRST_READ_TIMEOUT : CLI_TIMEOUT))
			{
				if (isFirstRead)
					_logger.logInfo("Client " + to_string(client.clientSocket) + " timed out on first read");
				else
					_logger.logInfo("Client " + to_string(client.clientSocket) + " timed out after " + to_string(elapsedTime) + " seconds of inactivity");
				clientsToRemove.push_back(client.clientSocket);
				continue;
			}
		}
		for (std::vector<int>::iterator it = clientsToRemove.begin(); it != clientsToRemove.end(); ++it)
			_serverManager.removeClient(*it);
		


		// Handle Session Cleanup
		std::vector<std::string> sessionsToRemove;
		ServerManager::Sessions sessions = _serverManager.getSessions();
		currentTime = std::time(0);
		for (ServerManager::SessionIterator it = sessions.begin(); it != sessions.end(); ++it)
		{
			const Session &session = it->second;
			const std::time_t inactivityTime = currentTime - session.lastActivity;
			const std::time_t lifetime = currentTime - session.createdAt;
			if (inactivityTime > MAX_SESSION_INACTIVITY)
			{
				_logger.logInfo("Session " + session.sessionId + " expired due to inactivity (" + to_string(inactivityTime) + " seconds)");
				sessionsToRemove.push_back(session.sessionId);
				continue;
			}
			if (lifetime > MAX_SESSION_LIFETIME)
			{
				_logger.logInfo("Session " + session.sessionId + " expired due to lifetime (" + to_string(lifetime) + " seconds)");
				sessionsToRemove.push_back(session.sessionId);
				continue;
			}
		}
		for (std::vector<std::string>::iterator it = sessionsToRemove.begin(); it != sessionsToRemove.end(); ++it)
			_serverManager.removeSession(*it);
	}
}