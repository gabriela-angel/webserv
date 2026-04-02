#ifndef REQUESTPROCESSOR_HPP
# define REQUESTPROCESSOR_HPP

# include "./http/HttpRequest.hpp"
# include "./http/HttpResponse.hpp"
# include "./config/ManagerConfig.hpp"
# include "./utils.hpp"
# include "./http/CgiHandler.hpp"
# include <fstream>
# include <sstream>
# include <sys/stat.h>
# include <dirent.h>
# include <unistd.h>
# include <cstring>

class RequestProcessor {
private:
	LocationConfig* matchLocation(std::string request_uri, ServerConfig& server);
	bool			isMethodAllowed(const std::string& method, const LocationConfig& location);
	std::string		resolveFilePath(ServerConfig& server, LocationConfig* location, std::string uri);

	void			handleGet(const HttpRequest& request, HttpResponse& response, ServerConfig& server, LocationConfig* location);
	void			handlePost(const HttpRequest& request, HttpResponse& response, ServerConfig& server, LocationConfig* location);
	void			handleDelete(const HttpRequest& request, HttpResponse& response, ServerConfig& server, LocationConfig* location);

	std::string		extractHeaderAttribute(const std::string& header, const std::string& key);
	std::string		extractFilename(const std::string& disposition);
	bool			writeToFile(const std::string& dest, const std::string& body);
	std::string 	resolvePostDest(const HttpRequest& request, HttpResponse& response, LocationConfig* location, const std::string& filepath, bool& file_existed);
	bool			handleMultipart(const HttpRequest& request, HttpResponse& response, LocationConfig* location);
	bool			canDelete(const std::string& path);
	void			serveFile(HttpResponse& response, const std::string& path, ServerConfig& server, LocationConfig* location);
	void			generateAutoindex(HttpResponse& response, const std::string& dirpath, const std::string& uri);
	
public:
	HttpResponse	process(const HttpRequest& request, ManagerConfig& config);

};

#endif