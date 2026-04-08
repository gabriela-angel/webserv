#pragma once

#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <ctime>
#include <netinet/in.h>

#include "HttpException.hpp"
#include "Headers.hpp"

#define HTTP_VERSION "HTTP/1.1"
#define CRLF "\r\n"
#define HEADER_END CRLF CRLF

#define HTTP_METHODS {"GET", "POST", "PUT", "DELETE", "HEAD", "OPTIONS", "PATCH"}

#define MAX_URI_SIZE			16384				// 16 KB
#define MAX_REQUEST_LINE_SIZE	(MAX_URI_SIZE + 20)	// Method + Version + Spaces
#define MAX_HEADER_SIZE			16384				// 16 KB
#define MAX_HEADERS				100					// Maximum number of headers allowed in a request
#define MAX_BODY_SIZE			10485760			// 10 MB
#define MAX_CHUNK_SIZE			1048576				// 1 MB
#define MAX_HOST_LABEL_SIZE		63					// Maximum size of a single label in the Host header
#define MAX_HOST_SIZE			255					// Maximum size of the entire Host header value

class	Http {
	public:
		virtual ~Http() {}
		typedef std::map<std::string, std::string>	Cookies;

};

struct	RequestLine {
	std::string method;
	std::string uri;
	std::string version;

	RequestLine() : method(""), uri(""), version("") {}
};

struct	HttpData {	
	RequestLine		requestLine;
	Headers			headers;
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
	size_t maxBodySize;

	// Session ID (if any)
	std::string sessionId;
	
	// Client State Machine for parsing HTTP requests
	StateMachine stateMachine;
	
	// Client Error ?
	HttpException exception;
	
	// Constructors
	ClientData(const struct sockaddr_in &addr, int clientSocket, int serverSocket, size_t maxBodySize = MAX_BODY_SIZE)
	: 
		clientSocket(clientSocket),
		serverSocket(serverSocket),
		client_addr(addr),
		maxBodySize(maxBodySize),
		sessionId(""),
		stateMachine(StateMachine()),
		exception()
	{}
	
	ClientData()
	: 
		clientSocket(-1),
		serverSocket(-1),
		client_addr(),
		maxBodySize(MAX_BODY_SIZE),
		sessionId(""),
		stateMachine(),
		exception()
	{}

};

#include "utils.tpp"