#pragma once

#include <sys/epoll.h>
#include <stdexcept>
#include <unistd.h>

class EpollManager {
	private:
		int	_epoll_fd;

		// Private Constructors (Singleton Pattern)
		EpollManager();
		EpollManager(const EpollManager &){};
		EpollManager &operator=(const EpollManager &){ return (*this); };

	public:
		// Destructor
		~EpollManager();

		// Get the singleton instance of the EpollManager
		static EpollManager &getInstance();

		void	addToEpoll( int sockfd, uint32_t events );
		void	modifyEpoll( int sockfd, uint32_t events );
		void	removeFromEpoll( int sockfd );

		// Getters
		int		getEpollFD() const { return _epoll_fd; }
};