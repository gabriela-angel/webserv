#ifndef CGIHANDLER_HPP
# define CGIHANDLER_HPP

# include "HttpRequest.hpp"
# include "HttpResponse.hpp"
# include "LocationConfig.hpp"
# include <string>
# include <map>
# include <cstdlib>
# include <cstring>
# include <sstream>
# include <vector>
# include <unistd.h>
# include <sys/wait.h>
# include <fcntl.h>
# include <errno.h>

class CgiHandler {
public:
	// Returns true and fills response if the file is a CGI script.
	// Returns false if the extension is not registered as CGI (caller serves statically).
	static bool tryRun(
		const struct HttpStruct&   request,
		HttpResponse&        response,
		const LocationConfig& location,
		const std::string&   filepath
	);

private:
	// Returns the interpreter for this file's extension, or "" if not CGI
	static std::string findInterpreter(const LocationConfig& location, const std::string& filepath);

	// Builds the CGI environment vector (caller must free with freeEnv)
	static char** buildEnv(
		const struct HttpStruct&  request,
		const std::string&  filepath,
		const std::string&  interpreter
	);
	static void freeEnv(char** env);

	// Runs the interpreter, returns the raw CGI output (headers + body)
	static bool execute(
		const std::string&  interpreter,
		const std::string&  filepath,
		char**              env,
		const std::string&  body,
		std::string&        output
	);

	// Parses raw CGI output into response headers and body
	static void parseCgiOutput(const std::string& output, HttpResponse& response);
};

#endif
