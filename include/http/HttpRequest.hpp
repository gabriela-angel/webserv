#pragma once

#include <string>
#include <cstdlib>
#include "ServerManager.hpp"
#include "Http.hpp"

class HttpRequest : public Http {
	public:

	private:
		static Logger&					_logger;
		static const ManagerConfig&		_config;
		HttpRequest();

		/* Client State Machine Processing */
		static bool						_processClientState(ClientData &client);
		
		/* Parsing and Validation */
		static void						_parseCookies(ClientData &client);

		static HttpPart<RequestLine>	_parseRequestLine(const std::string &buffer);
		static void						_validateRequestLine(const RequestLine &requestLine);

		static HttpPart<Headers>		_parseHeaders(const std::string &buffer);
		static void						_validateHeaders(const Headers &headers);
		static void						_validateMaxBodySize(const Headers &headers, const ManagerConfig &config, const std::string &uri);
		// This function parses the body and validates it according to the headers (Content-Length, Transfer-Encoding, etc.)
		static HttpPart<std::string>	_parseBody(struct StateMachine &stateMachine);

	public:
		~HttpRequest();

		static void processClient(ClientData &client);
};