#pragma once

#include "EpollManager.hpp"
#include "ServerManager.hpp"
#include "Logger.hpp"

#define MAX_EVENTS 64

class EventLoop
{
  private:
	Logger& _logger;
	EpollManager& _epoll;
	ServerManager _serverManager;
	struct epoll_event _events[MAX_EVENTS];

  private:
  	EventLoop();
	
  public:
	EventLoop(const char *configFilePath);
	~EventLoop();

	void run(void);
};