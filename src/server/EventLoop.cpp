#include "EventLoop.hpp"

EventLoop::~EventLoop() {}
EventLoop::EventLoop(const char *configFilePath) : _logger(Logger::getInstance())
{
	(void)configFilePath;
}

void EventLoop::run(){
	while (true)
	{
		// Waint for events on servers and clients
		int nfds = epoll_wait(_serverManager.getEpollFD(), _events, MAX_EVENTS, 1000); // 1000 ms timeout
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

			if (_events[i].events & EPOLLIN)
			{
				_logger.logInfo("Data available to read on socket: " + to_string(_events[i].data.fd));
				_serverManager.handleRead(_events[i].data.fd);
			}
			// else if (_events[i].events & EPOLLOUT)
			// {
			// 	_logger.logInfo("Ready to write on socket: " + to_string(_events[i].data.fd));
			// 	_serverManager.handleWrite(_events[i].data.fd);
			//  // DISCONECT CLIENT AFTER WRITING RESPONSE (IN ERROR CASES)
			// }
			
		}




		// TESTING
		const std::map<int, ClientData> &clientMap = _serverManager.getClientMap();
		for (std::map<int, ClientData>::const_iterator it = clientMap.begin(); it != clientMap.end(); ++it)
		{
			const ClientData &client = it->second;
			_logger.logDebug("Client " + to_string(client.clientSocket) + " read buffer: \n" + client.stateMachine.buffer);
		}






		// Handle Timeout for clients
		// const std::map<int, ClientData> &clientMap = _serverManager.getClientMap();
		std::time_t currentTime = std::time(0);
		for (std::map<int, ClientData>::const_iterator it = clientMap.begin(); it != clientMap.end(); ++it)
		{
			const ClientData &client = it->second;
			const std::time_t elapsedTime = currentTime - client.lastActivityTime;
			if (elapsedTime > (client.firstRead ? CLI_FIRST_READ_TIMEOUT : CLI_TIMEOUT))
			{
				if (client.firstRead)
					_logger.logInfo("Client " + to_string(client.clientSocket) + " timed out on first read");
				else
					_logger.logInfo("Client " + to_string(client.clientSocket) + " timed out after " + to_string(elapsedTime) + " seconds of inactivity");
				// Send a 408 Request Timeout response before closing the connection
				// _serverManager.sendErrorResponse(client.clientSocket, 408);
				_serverManager.removeClient(client.clientSocket);
				continue;
			}
		}

	}
}