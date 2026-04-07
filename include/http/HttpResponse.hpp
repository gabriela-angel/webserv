#ifndef HTTPRESPONSE_HPP
# define HTTPRESPONSE_HPP

# include "HttpUtils.hpp"
# include "Headers.hpp"
# include <string>
# include <map>
# include <sstream>

class HttpResponse {
private:
	HttpStatus::Code		_statusCode;
	Headers					_headers;
	std::string				_body;
public:
	HttpResponse();
	HttpResponse(const HttpResponse& copy);
	HttpResponse& operator=(const HttpResponse& other);
	~HttpResponse();

	// methods tbd, BUT PROBABLY:
	void					setStatus(HttpStatus::Code status);
	void					addHeader(const std::string& key, const std::string& value);
	void					setBody(const std::string& body);
	
	const HttpStatus::Code&	getStatus() const;
	const Headers&			getHeaders() const;
	const std::string&		getBody() const;
	std::string				toString() const;
};

#endif