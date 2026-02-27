#pragma once

#include <string>
#include <cstdlib>
#include "ServerManager.hpp"
#include "Http.hpp"

class HttpRequest : public Http {
	public:

	private:
		static Logger&		_logger;
		HttpRequest();
		static bool _processClientState(ClientData &client);
		static void _parseCookies(ClientData &client);

	public:
		~HttpRequest();

		static void processClient(ClientData &client);
};