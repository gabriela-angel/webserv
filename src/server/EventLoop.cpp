#include "EventLoop.hpp"

EventLoop::~EventLoop() {}
EventLoop::EventLoop(const char *configFilePath) : _logger(Logger::getInstance()), _epoll(EpollManager::getInstance())
{
	(void)configFilePath;
}

void EventLoop::run(){
	while (true)
	{
		// Waint for events on servers and clients
		int nfds = epoll_wait(_epoll.getEpollFD(), _events, MAX_EVENTS, -1);
		if (nfds == -1) throw std::runtime_error("Failed to wait for epoll events");

		for (int i = 0; i < nfds; ++i)
		{
			if (_epoll.isServerSocket(_events[i].data.fd))
			{
				_logger.logInfo("Accepting new connection on server socket: " + to_string(_events[i].data.fd));
				_serverManager.acceptConnection(_events[i].data.fd);
				continue;
			}
			


			if (_events[i].events & EPOLLIN)
			{
				_logger.logInfo("Data available to read on socket: " + to_string(_events[i].data.fd));
				_serverManager.handleRead(_events[i].data.fd);
				// TESTING
				const std::map<int, ClientData> &clientMap = _serverManager.getClientMap();
				for (std::map<int, ClientData>::const_iterator it = clientMap.begin(); it != clientMap.end(); ++it)
				{
					const ClientData &client = it->second;
					_logger.logDebug("Client " + to_string(client.clientSocket) + " read buffer: " + client.readBuffer);
				}
			}
			// else if (_events[i].events & EPOLLOUT)
			// {
			// 	_logger.logInfo("Ready to write on socket: " + to_string(_events[i].data.fd));
			// 	_serverManager.handleWrite(_events[i].data.fd);
			// }
			
		}
	}
}