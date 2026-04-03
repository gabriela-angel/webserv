#include "./http/HttpResponse.hpp"

HttpResponse::HttpResponse() : _statusCode(HttpStatus::OK) {}

HttpResponse::HttpResponse(const HttpResponse& copy) {
	*this = copy;
}

HttpResponse& HttpResponse::operator=(const HttpResponse& other) {
	if (this != &other) {
		_statusCode = other._statusCode;
		_headers = other._headers;
		_body = other._body;
	}
	return *this;
}

HttpResponse::~HttpResponse() {}

void HttpResponse::setStatus(HttpStatus::Code status) {
	_statusCode = status;
}

void HttpResponse::addHeader(const std::string& key, const std::string& value) {
	_headers[key] = value;
}

void HttpResponse::setBody(const std::string& body) {
	_body = body;
}

std::string HttpResponse::toString() const {
	std::stringstream ss;

	ss << HTTP_VERSION << " " << _statusCode << " " << HttpStatus::reasonPhrase(_statusCode) << CRLF;

	// check if capitalizing keys will be necessary
	for (std::map<std::string, std::string>::const_iterator it = _headers.begin(); it != _headers.end(); ++it) {
		ss << it->first << ": " << it->second << CRLF;
	}
	ss << HEADER_END;
	if (_body != "")
		ss << _body;

	std::string ret = ss.str();

	return ret;
}