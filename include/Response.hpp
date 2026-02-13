#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include <string>
# include <map>

enum HttpStatus {
	OK = 200,
	MOVED_PERMANENTLY = 301,
	MOVED_TEMPORARILY = 302,
	NOT_MODIFIED = 304,
	BAD_REQUEST = 400,
	UNAUTHORIZED = 401,
	FORBIDDEN = 403,
	NOT_FOUND = 404,
	METHOD_NOT_ALLOWED = 405,
	PAYLOAD_TOO_LARGE = 413,
	INTERNAL_SERVER_ERROR = 500,
	NOT_IMPLEMENTED = 501,
	SERVICE_UNAVAILABLE = 503
};

class Response {
private:
	HttpStatus							_statusCode;
	std::string							_reasonPhrase;
	std::string							_httpVersion; // WE CAN PROBABLY JUST TAKE IT FROM REQUEST
	std::map<std::string, std::string>	_headers;
	std::string							_body;
public:
	Response();
	//other constructors
	Response(const Response& copy);
	Response& operator=(const Response& other);
	~Response();

	// methods tbd, BUT PROBABLY:
	void setStatus(HttpStatus status);
	void setHeader(const std::string& key, const std::string& value);
	void setBody(const std::string& body);
	
	std::string buildResponse() const;
};

#endif