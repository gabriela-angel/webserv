#pragma once

#include <string>
#include <cstdlib>
#include "ServerManager.hpp"
#include "Http.hpp"

class HttpRequest : public Http {
	private:
		HttpRequest();

	public:
		~HttpRequest();
		static void processClient(ClientData &client);
};