#pragma once

#include <string>
#include <cstdlib>
#include "ServerManager.hpp"
#include "Http.hpp"

class HttpRequest : public Http {
	public:

	private:
		static Logger&					_logger;
		HttpRequest();

		/* Client State Machine Processing */
		static bool						_processClientState(ClientData &client, const ServerManager &manager);
		
		/* Parsing and Validation */
		static void						_parseCookies(ClientData &client);

		static HttpPart<RequestLine>	_parseRequestLine(const std::string &buffer);
		static void						_validateRequestLine(const RequestLine &requestLine);

		static HttpPart<Headers>		_parseHeaders(const std::string &buffer);
		static void						_validateHeaders(const Headers &headers);
		static void						_validateMaxBodySize(const ServerManager &manager, const ClientData &client, const Headers &headers);

		// This function parses the body and validates it according to the headers (Content-Length, Transfer-Encoding, etc.)
		static HttpPart<std::string>	_parseBody(struct StateMachine &stateMachine);

	public:
		~HttpRequest();

		static void processClient(ClientData &client, const ServerManager &manager);
};