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
	static void			handlePageErrors(HttpResponse& response, ServerConfig& server, LocationConfig* location);
	static LocationConfig* matchLocation(std::string request_uri, ServerConfig& server);
	static bool			isMethodAllowed(const std::string& method, const LocationConfig& location);
	static std::string		resolveFilePath(ServerConfig& server, LocationConfig* location, std::string uri);

	static void			handleGet(const HttpStruct& request, HttpResponse& response, ServerConfig& server, LocationConfig* location);
	static void			handlePost(const HttpStruct& request, HttpResponse& response, ServerConfig& server, LocationConfig* location);
	static void			handleDelete(const HttpStruct& request, HttpResponse& response, ServerConfig& server, LocationConfig* location);

	static std::string		extractHeaderAttribute(const std::string& header, const std::string& key);
	static std::string		extractFilename(const std::string& disposition);
	static bool			writeToFile(const std::string& dest, const std::string& body);
	static std::string 	resolvePostDest(const HttpStruct& request, HttpResponse& response, LocationConfig* location, const std::string& filepath, bool& file_existed);
	static bool			handleMultipart(const HttpStruct& request, HttpResponse& response, LocationConfig* location);
	static bool			canDelete(const std::string& path);
	static void			serveFile(HttpResponse& response, const std::string& path, ServerConfig& server, LocationConfig* location);
	static void			generateAutoindex(HttpResponse& response, const std::string& dirpath, const std::string& uri);
	
public:
	static HttpResponse	process(const HttpStruct& request, ManagerConfig& config);

};

#endif