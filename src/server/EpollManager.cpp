#include "EpollManager.hpp"

EpollManager &EpollManager::getInstance() {
	static EpollManager instance;
	return instance;
}

EpollManager::EpollManager() {
	_epoll_fd = epoll_create1(0);
	if (_epoll_fd == -1) {
		throw std::runtime_error("Failed to create epoll instance");
	}
}

EpollManager::~EpollManager() {
	if (_epoll_fd != -1) {
		close(_epoll_fd);
	}
}

void EpollManager::_addToEpoll(int sockfd, uint32_t events) {
	struct epoll_event ev = {};
	ev.events = events;
	ev.data.fd = sockfd;
	if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, sockfd, &ev) == -1) {
		throw std::runtime_error("Failed to add file descriptor to epoll");
	}
}

void EpollManager::_modifyEpoll(int sockfd, uint32_t events) {
	struct epoll_event ev = {};
	ev.events = events;
	ev.data.fd = sockfd;
	if (epoll_ctl(_epoll_fd, EPOLL_CTL_MOD, sockfd, &ev) == -1) {
		throw std::runtime_error("Failed to modify file descriptor in epoll");
	}
}

void EpollManager::_removeFromEpoll(int sockfd) {
	if (epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, sockfd, NULL) == -1) {
		throw std::runtime_error("Failed to remove file descriptor from epoll");
	}
}

// ------------------------------ Public Methods ------------------------------

void EpollManager::addClient(int sockfd, uint32_t events) {
	_addToEpoll(sockfd, events);
}

void EpollManager::addServer(int sockfd) {
	_addToEpoll(sockfd, EPOLLIN);
	_servers_fds.push_back(sockfd); // Store server socket fd for easy access
}

void EpollManager::removeClient(int sockfd) {
	if (isServerSocket(sockfd)) {
		throw std::runtime_error("Attempted to remove a server socket as a client");
	}
	_removeFromEpoll(sockfd);
}

void EpollManager::modifyClient(int sockfd, uint32_t events) {
	if (isServerSocket(sockfd)) {
		throw std::runtime_error("Attempted to modify a server socket as a client");
	}
	_modifyEpoll(sockfd, events);
}

bool EpollManager::isServerSocket(int sockfd) const {
	for (size_t i = 0; i < _servers_fds.size(); ++i) {
		if (_servers_fds[i] == sockfd) {
			return true;
		}
	}
	return false;
}
