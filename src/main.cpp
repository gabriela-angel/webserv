#include "Logger.hpp"
#include "Server.hpp"
#include "EpollManager.hpp"

std::string inetNtop(uint32_t binary_ip)
{
	uint32_t host_ip = ntohl(binary_ip);

	int byte1 = (host_ip >> 24) & 0xFF;
	int byte2 = (host_ip >> 16) & 0xFF;
	int byte3 = (host_ip >> 8) & 0xFF;
	int byte4 = host_ip & 0xFF;
	std::ostringstream ip_stream;
	ip_stream << byte1 << "." << byte2 << "." << byte3 << "." << byte4;
	return ip_stream.str();
}

void _acceptConnection( int serverSocket ){
	Logger &logger = Logger::getInstance();
	// Accept the new connection
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);
	int client_socket = accept(serverSocket, (struct sockaddr *)&client_addr, &client_len);
	if (client_socket < 0)
	{
		logger.logError("Error accepting new connection");
		logger.logDebug("error code: " + to_string(errno));
		logger.logDebug("error message: " + std::string(strerror(errno)));
		return;
	}
	logger.logInfo("Accepted new connection from " + std::string(inetNtop(client_addr.sin_addr.s_addr)) + ":" + to_string(ntohs(client_addr.sin_port)));
	EpollManager::getInstance().addToEpoll(client_socket, EPOLLIN);
}


int	main(void)
{
	try
	{
		Logger &logger = Logger::getInstance();
		logger.init(Logger::DEBUG);




		// ----------------------
		EpollManager &em = EpollManager::getInstance();

		Server *server = new Server();
		if (!server->init())
		{
			delete server;
			throw std::runtime_error("Failed to initialize the server");
		}
		em.addToEpoll(server->getServerSocket(), EPOLLIN);
		while (true)
		{
			const int MAX_EVENTS = 64;
			struct epoll_event events[MAX_EVENTS];
			logger.logDebug("Waiting for epoll events...");
			int nfds = epoll_wait(em.getEpollFD(), events, MAX_EVENTS, -1);
			logger.logDebug("Epoll wait returned with " + to_string(nfds) + " events");
			if (nfds == -1)
			{
				delete server;
				throw std::runtime_error("Failed to wait for epoll events");
			}
			for (int i = 0; i < nfds; ++i)
			{
				if (events[i].data.fd == server->getServerSocket())
				{
					// Handle new incoming connection
					logger.logInfo("Accepting new connection on server socket: " + to_string(events[i].data.fd));
					_acceptConnection(events[i].data.fd);
				}
				else
				{
					if (events[i].events & EPOLLIN)
					{
						logger.logInfo("Data available to read on socket: " + to_string(events[i].data.fd));
						std::string allData;
						char buffer[1024];

						ssize_t bytesRead = recv(events[i].data.fd, buffer, sizeof(buffer), 0);
						logger.logDebug("recv returned with " + to_string(bytesRead) + " bytes read");
						if (bytesRead < 0)							{
							if (errno == EAGAIN || errno == EWOULDBLOCK)
							{
								// No more data to read
								break;
							}
							else
							{
								logger.logError("Error reading from socket: " + to_string(events[i].data.fd));
								logger.logDebug("error code: " + to_string(errno));
								logger.logDebug("error message: " + std::string(strerror(errno)));
								break;
							}
						}
						else if (bytesRead == 0)
						{
							// Connection closed by client
							logger.logInfo("Connection closed by client on socket: " + to_string(events[i].data.fd));
							break;
						}
						else
						{
							allData.append(buffer, bytesRead);
						}
						logger.logInfo("Received data on socket " + to_string(events[i].data.fd) + ": " + allData);
					}
					else
					{
						logger.logWarning("Received unexpected event on socket: " + to_string(events[i].data.fd));
					}
				}
			}


		}
		// ----------------------

	} catch (const std::exception &e)
	{
		std::cerr << RED "Exception: " YELLOW << e.what() << RESET << std::endl;
		return (1);
	}

	return (0);
}