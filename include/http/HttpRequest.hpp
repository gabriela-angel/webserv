#pragma once

#include <string>
#include <cstdlib>
#include "ServerManager.hpp"
#include "Http.hpp"

class HttpRequest : public Http {
	private:
		static Logger&		_logger;
		HttpRequest();

	public:
		~HttpRequest();
		static void processClient(ClientData &client);
		static bool processClientState(ClientData &client);
};