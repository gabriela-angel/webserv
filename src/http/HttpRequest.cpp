// EXAMPLE FOR TESTING PURPOSES

#include "./http/HttpRequest.hpp"

HttpRequest::HttpRequest()
	: _method("GET"), _uri("/"), _version("HTTP/1.1"), _port(80) {}

HttpRequest::HttpRequest(
	const std::string& method,
	const std::string& uri,
	const std::string& version,
	const std::map<std::string, std::string>& headers,
	const std::string& body,
	const std::string& host,
	int port
)
	: _method(method), _uri(uri), _version(version),
	  _headers(headers), _body(body), _host(host), _port(port) {}

HttpRequest::HttpRequest(const HttpRequest& copy) {
	*this = copy;
}

HttpRequest& HttpRequest::operator=(const HttpRequest& other) {
	if (this != &other) {
		_method  = other._method;
		_uri     = other._uri;
		_version = other._version;
		_headers = other._headers;
		_body    = other._body;
		_host    = other._host;
		_port    = other._port;
	}
	return *this;
}

HttpRequest::~HttpRequest() {}

const std::string& HttpRequest::getMethod() const {
	return _method;
}

const std::string& HttpRequest::getUri() const {
	return _uri;
}

const std::string& HttpRequest::getVersion() const {
	return _version;
}

const std::map<std::string, std::string>& HttpRequest::getHeaders() const {
	return _headers;
}

const std::string& HttpRequest::getBody() const {
	return _body;
}

const std::string& HttpRequest::getHostHeader() const {
	return _host;
}

int HttpRequest::getPort() const {
	return _port;
}
