#pragma once

#include <string>
#include <cstdlib>
#include "ServerManager.hpp"

class HttpRequest {
	private:
		std::string request;

	public:
		HttpRequest();
		~HttpRequest();

		static void updateState(ClientData &client);
	private:
		static long _headerSize(const std::string &buffer);
		static long _contentLength(const std::string &buffer);
};