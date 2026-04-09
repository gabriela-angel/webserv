#include "CgiHandler.hpp"
#include "HttpUtils.hpp"
#include "RequestProcessor.hpp"


// ─── Public entry point ───────────────────────────────────────────────────────

bool CgiHandler::tryRun(
	const HttpStruct&    request,
	HttpResponse&         response,
	const LocationConfig& location,
	const std::string&    filepath
) {
	std::string interpreter = findInterpreter(location, filepath);
	if (interpreter.empty())
		return false; // not a CGI file, caller handles it

	char** env = buildEnv(request, filepath, interpreter);
	if (!env) {
		response.setStatus(HttpStatus::INTERNAL_SERVER_ERROR);
		return true;
	}

	std::string output;
	bool ok = execute(interpreter, filepath, env, request.getBody(), output);
	freeEnv(env);

	if (!ok) {
		response.setStatus(HttpStatus::INTERNAL_SERVER_ERROR);
		return true;
	}

	parseCgiOutput(output, response);
	return true;
}

// ─── Interpreter lookup ───────────────────────────────────────────────────────

std::string CgiHandler::findInterpreter(const LocationConfig& location, const std::string& filepath) {
	if (!location.getIsCgi())
		return "";

	size_t dot = filepath.rfind('.');
	if (dot == std::string::npos)
		return "";

	std::string ext = filepath.substr(dot); // e.g. ".py"
	const std::map<std::string, std::string>& cgi_map = location.getCgi();
	std::map<std::string, std::string>::const_iterator it = cgi_map.find(ext);
	if (it == cgi_map.end())
		return "";

	return it->second; // e.g. "/usr/bin/python3"
}

// ─── Environment builder ──────────────────────────────────────────────────────

static std::string intStr(int n) {
	std::ostringstream ss;
	ss << n;
	return ss.str();
}

char** CgiHandler::buildEnv(
	const HttpStruct&  request,
	const std::string&  filepath,
	const std::string&  interpreter
) {
	std::vector<std::string> env_vec;

	env_vec.push_back("GATEWAY_INTERFACE=CGI/1.1");
	env_vec.push_back("REDIRECT_STATUS=200");
	env_vec.push_back("SERVER_PROTOCOL=HTTP/1.1");
	env_vec.push_back("SERVER_SOFTWARE=webserv/1.0");
	env_vec.push_back("REQUEST_METHOD=" + request.getMethod());
	env_vec.push_back("SCRIPT_FILENAME=" + filepath);
	env_vec.push_back("SCRIPT_NAME=" + request.getUri());
	env_vec.push_back("SERVER_PORT=" + to_string(request.getPort()));
	env_vec.push_back("SERVER_NAME=" + request.getIP());

	// Query string (part of URI after '?')
	std::string uri = request.getUri();
	size_t q = uri.find('?');
	if (q != std::string::npos) {
		env_vec.push_back("PATH_INFO=" + uri.substr(0, q));
		env_vec.push_back("QUERY_STRING=" + uri.substr(q + 1));
	} else {
		env_vec.push_back("PATH_INFO=" + uri);
		env_vec.push_back("QUERY_STRING=");
	}

	// Body-related
	const std::string& body = request.getBody();
	if (!body.empty()) {
		env_vec.push_back("CONTENT_LENGTH=" + intStr(static_cast<int>(body.size())));
	} else {
		env_vec.push_back("CONTENT_LENGTH=0");
	}

	// Promote relevant HTTP headers to CGI env vars
	const Headers& headers = request.getHeaders();


	for (Headers::ConstIterator it = headers.begin(); it != headers.end(); ++it) {
		// Content-Type is a special case: no HTTP_ prefix
		if (Headers::compare(it->first, "Content-Type")) {
			env_vec.push_back("CONTENT_TYPE=" + it->second[0]);
			continue;
		}
		// All other headers: uppercase, hyphens to underscores, HTTP_ prefix
		// Note: it->first is already in uppercase
		env_vec.push_back("HTTP_" + it->first + "=" + it->second[0]);
	}

	env_vec.push_back("_INTERPRETER=" + interpreter); // informational

	// Convert to char** (null-terminated array)
	char** env = new char*[env_vec.size() + 1];
	for (size_t i = 0; i < env_vec.size(); i++) {
		env[i] = new char[env_vec[i].size() + 1];
		std::strcpy(env[i], env_vec[i].c_str());
	}
	env[env_vec.size()] = NULL;
	return env;
}

void CgiHandler::freeEnv(char** env) {
	for (int i = 0; env[i] != NULL; i++)
		delete[] env[i];
	delete[] env;
}

// ─── Execution ────────────────────────────────────────────────────────────────

bool CgiHandler::execute(
	const std::string&  interpreter,
	const std::string&  filepath,
	char**              env,
	const std::string&  body,
	std::string&        output
) {
	// Two pipes: parent→child (stdin) and child→parent (stdout)
	int stdin_pipe[2];
	int stdout_pipe[2];

	if (pipe(stdin_pipe) != 0 || pipe(stdout_pipe) != 0)
		return false;

	pid_t pid = fork();
	if (pid < 0) {
		close(stdin_pipe[0]);  close(stdin_pipe[1]);
		close(stdout_pipe[0]); close(stdout_pipe[1]);
		return false;
	}

	if (pid == 0) {
		// Child: wire stdin and stdout to the pipes
		close(stdin_pipe[1]);
		close(stdout_pipe[0]);

		if (dup2(stdin_pipe[0], STDIN_FILENO) < 0)  _exit(1);
		if (dup2(stdout_pipe[1], STDOUT_FILENO) < 0) _exit(1);

		close(stdin_pipe[0]);
		close(stdout_pipe[1]);

		// Build argv: { interpreter, filepath, NULL }
		char* argv[3];
		argv[0] = const_cast<char*>(interpreter.c_str());
		argv[1] = const_cast<char*>(filepath.c_str());
		argv[2] = NULL;

		execve(interpreter.c_str(), argv, env);
		_exit(1); // execve failed
	}

	// Parent: close unused ends
	close(stdin_pipe[0]);
	close(stdout_pipe[1]);

	// Write request body to CGI stdin
	if (!body.empty()) {
		size_t total = 0;
		while (total < body.size()) {
			ssize_t written = write(stdin_pipe[1], body.c_str() + total, body.size() - total);
			if (written < 0) break;
			total += static_cast<size_t>(written);
		}
	}
	close(stdin_pipe[1]); // signal EOF to the script

	// Read CGI stdout
	char buf[4096];
	ssize_t n;
	while ((n = read(stdout_pipe[0], buf, sizeof(buf))) > 0)
		output.append(buf, static_cast<size_t>(n));
	close(stdout_pipe[0]);
	// Wait for child and check exit status
	int status;
	waitpid(pid, &status, 0);

	return (WIFEXITED(status) && WEXITSTATUS(status) == 0);
}

// ─── CGI output parser ────────────────────────────────────────────────────────

// CGI output format:
//   Header-Name: value\r\n
//   Header-Name: value\r\n
//   \r\n
//   <body>
// Status defaults to 200 unless a "Status:" header is present.

void CgiHandler::parseCgiOutput(const std::string& output, HttpResponse& response) {
	size_t header_end = output.find("\r\n\r\n");
	// Also accept \n\n (some scripts don't emit \r\n)
	size_t header_end_lf = output.find("\n\n");

	bool use_crlf;
	size_t split;
	size_t body_start;

	if (header_end != std::string::npos &&
	    (header_end_lf == std::string::npos || header_end < header_end_lf)) {
		use_crlf   = true;
		split      = header_end;
		body_start = header_end + 4;
	} else if (header_end_lf != std::string::npos) {
		use_crlf   = false;
		split      = header_end_lf;
		body_start = header_end_lf + 2;
	} else {
		// No header section at all — treat everything as body
		response.setStatus(HttpStatus::OK);
		response.setBody(output);
		return;
	}

	std::string header_block = output.substr(0, split);
	std::string body         = output.substr(body_start);

	// Parse headers line by line
	HttpStatus::Code status_code = HttpStatus::OK;
	std::string line;
	std::istringstream stream(header_block);

	while (std::getline(stream, line)) {
		// Strip trailing \r if present
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);
		if (line.empty())
			continue;

		size_t colon = line.find(':');
		if (colon == std::string::npos)
			continue;

		std::string key   = line.substr(0, colon);
		std::string value = line.substr(colon + 1);
		// Trim leading space from value
		size_t start = value.find_first_not_of(" \t");
		if (start != std::string::npos)
			value = value.substr(start);

		// "Status: 404 Not Found" → set HTTP status code
		if (key == "Status") {
			int code = std::atoi(value.c_str());
			if (code > 0)
				status_code = static_cast<HttpStatus::Code>(code);
		} else {
			response.addHeader(key, value);
		}
	}

	response.setStatus(status_code);
	response.setBody(body);

	// Ensure Content-Length reflects the actual body
	std::ostringstream ss;
	ss << body.size();
	response.addHeader("Content-Length", ss.str());

	(void)use_crlf;
}
