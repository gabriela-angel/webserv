#pragma once

#include <string>
#include <cstdlib>
#include "ServerManager.hpp"
#include "Http.hpp"

#define MAX_URI_SIZE			16384				// 16 KB
#define MAX_REQUEST_LINE_SIZE	(MAX_URI_SIZE + 20)	// Method + Version + Spaces
#define MAX_HEADER_SIZE			16384				// 16 KB
#define MAX_HEADERS				100					// Maximum number of headers allowed in a request
#define MAX_BODY_SIZE			10485760			// 10 MB
#define MAX_CHUNK_SIZE			1048576				// 1 MB
#define MAX_HOST_LABEL_SIZE		63					// Maximum size of a single label in the Host header
#define MAX_HOST_SIZE			255					// Maximum size of the entire Host header value

class HttpRequest : public Http {
	public:

	private:
		static Logger&		_logger;
		HttpRequest();

		/* Client State Machine Processing */
		static bool						_processClientState(ClientData &client);
		
		/* Parsing and Validation */
		static void						_parseCookies(ClientData &client);

		static HttpPart<RequestLine>	_parseRequestLine(const std::string &buffer);
		static void						_validateRequestLine(const RequestLine &requestLine);

		static HttpPart<HeaderMap>		_parseHeaders(const std::string &buffer);
		static void						_validateHeaders(const HeaderMap &headers);

		// This function parses the body and validates it according to the headers (Content-Length, Transfer-Encoding, etc.)
		static HttpPart<std::string>	_parseBody(struct StateMachine &stateMachine);

	public:
		~HttpRequest();

		static void processClient(ClientData &client);
};