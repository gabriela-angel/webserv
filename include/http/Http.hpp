#pragma once

#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include "HttpException.hpp"


#define HTTP_VERSION "HTTP/1.1"
#define CRLF "\r\n"
#define HEADER_END CRLF CRLF

#define MAX_URI_SIZE			16384				// 16 KB
#define MAX_REQUEST_LINE_SIZE	(MAX_URI_SIZE + 20)	// Method + Version + Spaces
#define MAX_HEADER_SIZE			16384				// 16 KB
#define MAX_HEADERS				100					// Maximum number of headers allowed in a request
#define MAX_BODY_SIZE			10485760			// 10 MB
#define MAX_HOST_LABEL_SIZE		63					// Maximum size of a single label in the Host header
#define MAX_HOST_SIZE			255					// Maximum size of the entire Host header value

#define HTTP_METHODS {"GET", "POST", "PUT", "DELETE", "HEAD", "OPTIONS", "PATCH"}

template <typename T>
struct HttpPart
{
	T value;
	size_t size;

	HttpPart() : value(), size(0) {}

};

struct RequestLine {
	std::string method;
	std::string uri;
	std::string version;

	RequestLine() : method(""), uri(""), version("") {}
};

class Http {
	public:
		virtual ~Http() {}
		typedef std::string HeaderKey;
		typedef std::vector<std::string> HeaderValues;
		typedef std::map<HeaderKey, HeaderValues> HeaderMap;
		typedef HeaderMap::const_iterator ConstHeaderIterator;
		typedef HeaderMap::iterator HeaderIterator;
		typedef HeaderValues::const_iterator ConstHeaderValueIterator;
		typedef HeaderValues::iterator HeaderValueIterator;
	
	protected:
		static HttpPart<RequestLine>	_parseRequestLine(const std::string &buffer);
		static void						_validateRequestLine(const RequestLine &requestLine);

		static HttpPart<HeaderMap>		_parseHeaders(const std::string &buffer);
		static void						_validateHeaders(const HeaderMap &headers);
};

struct HttpData {	
	RequestLine requestLine;
	Http::HeaderMap headers;
	std::string body;

	HttpData() : requestLine(), headers(), body("") {}
};

#include "utils.tpp"