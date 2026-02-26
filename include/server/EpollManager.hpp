#pragma once

#include <sys/epoll.h>
#include <stdexcept>
#include <unistd.h>
#include <vector>

class EpollManager {
	private:
		int					_epoll_fd;
		std::vector<int>	_servers_fds; // Store server socket fds for easy access
		
		// Private Constructors (Singleton Pattern)
		EpollManager();
		EpollManager(const EpollManager &){};
		EpollManager &operator=(const EpollManager &){ return (*this); };

	public:
		// Destructor
		~EpollManager();

		// Getters
		int		getEpollFD() const { return _epoll_fd; }

		// Get the singleton instance of the EpollManager
		static EpollManager &getInstance();

		// Public methods to manage epoll
		void	addClient(int sockfd, uint32_t events);
		void	addServer(int sockfd);
		void	removeClient(int sockfd);
		void	modifyClient(int sockfd, uint32_t events);
		bool	isServerSocket(int sockfd) const;

		
	
	private:
		void	_addToEpoll( int sockfd, uint32_t events );
		void	_modifyEpoll( int sockfd, uint32_t events );
		void	_removeFromEpoll( int sockfd );
};