#pragma once

#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <ctime>
#include <netinet/in.h>

#include "HttpException.hpp"

#define HTTP_VERSION "HTTP/1.1"
#define CRLF "\r\n"
#define HEADER_END CRLF CRLF

#define HTTP_METHODS {"GET", "POST", "PUT", "DELETE", "HEAD", "OPTIONS", "PATCH"}

// Main Headers
#define HOST "HOST"
#define CONTENT_LENGTH "CONTENT-LENGTH"
#define TRANSFER_ENCODING "TRANSFER-ENCODING"
#define CONNECTION "CONNECTION"
#define EXPECT "EXPECT"
#define CONTENT_TYPE "CONTENT-TYPE"
#define COOKIE "COOKIE"

class	Http {
	public:
		virtual ~Http() {}
		typedef std::map<std::string, std::string>	Cookies;
		typedef std::string							HeaderKey;
		typedef std::vector<std::string>			HeaderValues;
		typedef std::map<HeaderKey, HeaderValues>	HeaderMap;
		typedef HeaderMap::const_iterator			ConstHeaderIterator;
		typedef HeaderMap::iterator					HeaderIterator;
		typedef HeaderValues::const_iterator		ConstHeaderValueIterator;
		typedef HeaderValues::iterator				HeaderValueIterator;
};

struct	RequestLine {
	std::string method;
	std::string uri;
	std::string version;

	RequestLine() : method(""), uri(""), version("") {}
};

struct	HttpData {	
	RequestLine		requestLine;
	Http::HeaderMap	headers;
	std::string		body;

	// Main Info
	std::string		host;
	bool			chunkedTransferEncoding;
	size_t			contentLength;
	bool			keepAlive;
	bool			expectContinue;

	// Cookies
	Http::Cookies	cookies;

	HttpData()
	:
		requestLine(),
		headers(),
		body(""),
		host(""),
		chunkedTransferEncoding(false),
		contentLength(0),
		keepAlive(false),
		expectContinue(false),
		cookies()
	{}
};

struct	StateMachine
{
	// ----- Types -----
	enum ReadState
	{
		READING_REQUEST_LINE,
		READING_HEADERS,
		READING_BODY,
		DONE,
		ERROR
	};


	// ----- Data -----
	ReadState		state;
	std::string		buffer;
	struct HttpData	httpData;
	bool			firstReadFlag;
	std::time_t		lastActivityTime;


	// ----- Functions -----
	StateMachine() 
	: 
		state(READING_REQUEST_LINE),
		buffer(""),
		httpData(),
		firstReadFlag(false),
		lastActivityTime(std::time(0))
	{}

	void updateActivity() {
		if (!firstReadFlag) firstReadFlag = true;
		lastActivityTime = std::time(0);
	}

	std::string getStateString() const {
		switch (state) {
			case READING_REQUEST_LINE: return "READING_REQUEST_LINE";
			case READING_HEADERS: return "READING_HEADERS";
			case READING_BODY: return "READING_BODY";
			case DONE: return "DONE";
			case ERROR: return "ERROR";
			default: return "UNKNOWN";
		}
	}
};

struct	ClientData
{
	// Client information
	int clientSocket;
	int	serverSocket;
	struct sockaddr_in client_addr;

	// Session ID (if any)
	std::string sessionId;
	
	// Client State Machine for parsing HTTP requests
	StateMachine stateMachine;
	
	// Client Error ?
	HttpException exception;
	
	// Constructors
	ClientData(const struct sockaddr_in &addr, int clientSocket, int serverSocket)
	: 
		clientSocket(clientSocket),
		serverSocket(serverSocket),
		client_addr(addr),
		sessionId(""),
		stateMachine(StateMachine()),
		exception()
	{}
	
	ClientData()
	: 
		clientSocket(-1),
		serverSocket(-1),
		client_addr(),
		sessionId(""),
		stateMachine(),
		exception()
	{}

};

#include "utils.tpp"