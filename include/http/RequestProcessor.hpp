#ifndef REQUESTPROCESSOR_HPP
# define REQUESTPROCESSOR_HPP

// #include "HttpRequest.hpp"
# include "HttpResponse.hpp"
# include "ManagerConfig.hpp"

class RequestProcessor {
private:
	LocationConfig* matchLocation(std::string request_uri, ServerConfig& server);
	bool isMethodAllowed(const std::string& method, const LocationConfig& location);
public:
	HttpResponse process(const HttpRequest& request, const ManagerConfig& config);

};

#endif