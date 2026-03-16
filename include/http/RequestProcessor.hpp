#ifndef REQUESTPROCESSOR_HPP
# define REQUESTPROCESSOR_HPP

// #include "HttpRequest.hpp"
# include "./http/HttpResponse.hpp"
# include "./config/ManagerConfig.hpp"
# include "./utils.hpp"

class RequestProcessor {
private:
	std::string _filepath;

	LocationConfig* matchLocation(std::string request_uri, ServerConfig& server);
	bool isMethodAllowed(const std::string& method, const LocationConfig& location);
	void processMethods(const HttpRequest& request, HttpResponse& response, LocationConfig* location, ServerConfig& server);
public:
	HttpResponse process(const HttpRequest& request, const ManagerConfig& config);

};

#endif