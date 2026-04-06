#ifndef REQUESTPROCESSOR_HPP
# define REQUESTPROCESSOR_HPP

# include "HttpResponse.hpp"
# include "ManagerConfig.hpp"
# include "utils.hpp"
# include "CgiHandler.hpp"
#include "Http.hpp"

# include <fstream>
# include <sstream>
# include <sys/stat.h>
# include <dirent.h>
# include <unistd.h>
# include <cstring>

struct HttpStruct {
	std::string method;
	std::string HTTPVersion;
	std::string uri;
	std::string host;
	Headers		headers;
	std::string body;
	
	HttpException exception;

	int port;

	// Getters
	const std::string& getMethod() const { return method; }
	const std::string& getHTTPVersion() const { return HTTPVersion; }
	const std::string& getUri() const { return uri; }
	const std::string& getHostHeader() const { return host; }
	const Headers& getHeaders() const { return headers; }
	const std::string& getBody() const { return body; }
	const int& getPort() const { return port; }

};

class RequestProcessor {
private:
	LocationConfig* matchLocation(std::string request_uri, ServerConfig& server);
	bool			isMethodAllowed(const std::string& method, const LocationConfig& location);
	std::string		resolveFilePath(ServerConfig& server, LocationConfig* location, std::string uri);

	void			handleGet(const HttpStruct& request, HttpResponse& response, ServerConfig& server, LocationConfig* location);
	void			handlePost(const HttpStruct& request, HttpResponse& response, ServerConfig& server, LocationConfig* location);
	void			handleDelete(const HttpStruct& request, HttpResponse& response, ServerConfig& server, LocationConfig* location);

	std::string		extractHeaderAttribute(const std::string& header, const std::string& key);
	std::string		extractFilename(const std::string& disposition);
	bool			writeToFile(const std::string& dest, const std::string& body);
	std::string 	resolvePostDest(const HttpStruct& request, HttpResponse& response, LocationConfig* location, const std::string& filepath, bool& file_existed);
	bool			handleMultipart(const HttpStruct& request, HttpResponse& response, LocationConfig* location);
	bool			canDelete(const std::string& path);
	void			serveFile(HttpResponse& response, const std::string& path, ServerConfig& server, LocationConfig* location);
	void			generateAutoindex(HttpResponse& response, const std::string& dirpath, const std::string& uri);
	
public:
	HttpResponse	process(const HttpStruct& request, ManagerConfig& config);

};

#endif