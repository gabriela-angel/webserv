// EXAMPLE FOR TESTING PURPOSES

#ifndef HTTPREQUEST_HPP
# define HTTPREQUEST_HPP

# include <map>
# include <string>

class HttpRequest {
private:
	std::string							_method;
	std::string							_uri;
	std::string							_version;
	std::map<std::string, std::string>	_headers;
	std::string							_body;
	std::string							_host;
	int									_port;

public:
	HttpRequest();
	HttpRequest(
		const std::string& method,
		const std::string& uri,
		const std::string& version,
		const std::map<std::string, std::string>& headers,
		const std::string& body,
		const std::string& host,
		int port
	);
	HttpRequest(const HttpRequest& copy);
	HttpRequest& operator=(const HttpRequest& other);
	~HttpRequest();

	const std::string&							getMethod()		const;
	const std::string&							getUri()		const;
	const std::string&							getVersion()	const;
	const std::map<std::string, std::string>&	getHeaders()	const;
	const std::string&							getBody()		const;
	const std::string&							getHostHeader()	const;
	int											getPort()		const;
};

#endif
