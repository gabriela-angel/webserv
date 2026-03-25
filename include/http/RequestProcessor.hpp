#ifndef REQUESTPROCESSOR_HPP
# define REQUESTPROCESSOR_HPP

# include "./http/HttpRequest.hpp"
# include "./http/HttpResponse.hpp"
# include "./config/ManagerConfig.hpp"
# include "./utils.hpp"

class RequestProcessor {
private:
	LocationConfig* matchLocation(std::string request_uri, ServerConfig& server);
	bool			isMethodAllowed(const std::string& method, const LocationConfig& location);
	std::string		resolveFilePath(ServerConfig& server, LocationConfig* location, std::string uri);

	void			handleGet(const HttpRequest& request, HttpResponse& response, ServerConfig& server, LocationConfig* location);
	void			handlePost(const HttpRequest& request, HttpResponse& response, ServerConfig& server, LocationConfig* location);
	void			handleDelete(const HttpRequest& request, HttpResponse& response, ServerConfig& server, LocationConfig* location);

	void serveStatic(HttpResponse& response, const std::string& path);
	

	
public:
	HttpResponse process(const HttpRequest& request, const ManagerConfig& config);

};

#endif