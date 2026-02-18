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

void EpollManager::addToEpoll(int sockfd, uint32_t events) {
	struct epoll_event ev = {};
	ev.events = events;
	ev.data.fd = sockfd;
	if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, sockfd, &ev) == -1) {
		throw std::runtime_error("Failed to add file descriptor to epoll");
	}
}

void EpollManager::modifyEpoll(int sockfd, uint32_t events) {
	struct epoll_event ev = {};
	ev.events = events;
	ev.data.fd = sockfd;
	if (epoll_ctl(_epoll_fd, EPOLL_CTL_MOD, sockfd, &ev) == -1) {
		throw std::runtime_error("Failed to modify file descriptor in epoll");
	}
}

void EpollManager::removeFromEpoll(int sockfd) {
	if (epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, sockfd, NULL) == -1) {
		throw std::runtime_error("Failed to remove file descriptor from epoll");
	}
}