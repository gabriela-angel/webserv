#include "Logger.hpp"
#include "./config/ManagerConfig.hpp"
#include "./config/ParseConfig.hpp"
#include "./http/RequestProcessor.hpp"
#include "./http/HttpRequest.hpp"
#include <iostream>
#include <map>

// HEKPER PARA PRINTAR OS TESTES
static void runTest(
	const std::string&    label,
	const std::string&    method,
	const std::string&    uri,
	const std::map<std::string, std::string>& headers,
	const std::string&    body,
	const std::string&    host,
	int                   port,
	ManagerConfig&        config)
{
	std::cout << "\n========================================\n";
	std::cout << "TEST: " << label << "\n";
	std::cout << method << " " << uri << " HTTP/1.1\n";
	std::cout << "Host: " << host << ":" << port << "\n";
	if (!body.empty())
		std::cout << "Body: " << body.substr(0, 80)
		          << (body.size() > 80 ? "..." : "") << "\n";
	std::cout << "----------------------------------------\n";
 
	HttpRequest request(method, uri, "HTTP/1.1", headers, body, host, port);
	RequestProcessor processor;
	HttpResponse response = processor.process(request, config);
 
	// Print only the status line + headers (body can be large)
	std::string raw = response.toString();
	size_t body_start = raw.find("\r\n\r\n");
	if (body_start != std::string::npos) {
		std::cout << raw.substr(0, body_start) << "\r\n\r\n";
		std::string resp_body = raw.substr(body_start + 4);
		if (!resp_body.empty())
			std::cout << resp_body.substr(0, 200)
			          << (resp_body.size() > 200 ? "\n[...truncated]" : "") << "\n";
	} else {
		std::cout << raw;
	}
}

int	main(int ac, char **av)
{
	(void)av;
	// try
	// {
	// 	Logger &logger = Logger::getInstance();
	// 	logger.init(Logger::DEBUG);

	// 	logger.logInfo("Hello, World!");
	// 	logger.logError("This is an error message.");
	// 	logger.logDebug("This is a debug message.");
	// } catch (const std::exception &e)
	// {
	// 	std::cerr << RED "Exception: " YELLOW << e.what() << RESET << std::endl;
	// }

	Logger &logger = Logger::getInstance();
	logger.init(Logger::DEBUG);

	try {
		if (ac != 2)
			throw std::invalid_argument("Usage: ./webserv [config_file]");

		ParseConfig parser(av[1]);
		std::vector<ServerConfig> servers = parser.parse();
		ManagerConfig config(servers);

		// ── TESTES COM O ARQUIVO DE CONFIG OFFICIAL.CONF (pode apagar) -----------------------------------------------------------------
 
		std::map<std::string, std::string> base_headers;
		base_headers["Host"]       = "42sp";
		base_headers["Connection"] = "close";
 
		// 1. GET static index page
		runTest("GET static index", "GET", "/", base_headers, "", "42sp", 8085, config);
 
		// 2. GET autoindex on /cadets_uploads
		runTest("GET autoindex", "GET", "/cadets_uploads", base_headers, "", "42sp", 8085, config);
 
		// 3. GET CGI — cadet list (runs cadet_list.py)
		runTest("GET CGI cadet_list", "GET", "/cadets_list", base_headers, "", "42sp", 8085, config);
 
		// 4. POST to create a plain text file via upload
		{
			std::map<std::string, std::string> h = base_headers;
			h["Content-Type"] = "text/plain";
			runTest("POST create file", "POST", "/cadet", h,
				"hello from POST", "42sp", 8085, config);
		}
 
		// 5. POST CGI — toggle silent mode (silent.py reads POST body from stdin)
		{
			std::map<std::string, std::string> h;
			h["Host"]           = "ninjas-server";
			h["Connection"]     = "close";
			h["Content-Type"]   = "application/x-www-form-urlencoded";
			h["Content-Length"] = "8";
			runTest("POST CGI toggle silent", "POST", "/silent/silent.py",
				h, "toggle=1", "ninjas-server", 8084, config);
		}
 
		// 6. DELETE a file directly (uses /cgi location which allows DELETE)
		runTest("DELETE file via /cgi", "DELETE",
			"/cgi/cadet_delete.py", base_headers, "", "42sp", 8085, config);
 
		// 7. DELETE CGI — cadet_delete.py via query string
		{
			std::map<std::string, std::string> h = base_headers;
			runTest("DELETE CGI cadet", "DELETE",
				"/cgi/cadet_delete.py?name=kevin&file=kevin.jpg",
				h, "", "42sp", 8085, config);
		}
 
		// 8. GET a non-existent file → 404
		runTest("GET 404", "GET", "/does_not_exist.html",
			base_headers, "", "42sp", 8085, config);
 
		// 9. POST on a GET-only location → 405
		runTest("POST on GET-only location → 405", "POST", "/",
			base_headers, "body", "42sp", 8085, config);
 
		// 10. GET redirect
		{
			std::map<std::string, std::string> h = base_headers;
			h["Host"] = "ninjas-server";
			runTest("GET redirect /fast → 302", "GET", "/fast",
				h, "", "ninjas-server", 8084, config);
		}
	}
	catch (const std::exception &e) {
		logger.logError(e.what());
		return 1;
	}
	

	return (0);
}