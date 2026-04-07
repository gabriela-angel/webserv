// Quando request chega:
// Recebe HttpRequest - DONE
// Descobre o ServerConfig correto (host + port) - DONE
// Encontra a LocationConfig mais específica (match por prefixo) -DONE
// Verifica se há redirect - DONE
// Se sim → resposta 301/302 e para aqui
// Verifica se o método é permitido - DONE
// Se não → 405 - DONE
// Resolve root (location → server fallback) - DONE
// Constrói path no filesystem - DONE
// Verificar se é diretório
// Resolver index
// Verificar existência
// Executar GET/POST/DELETE
// Criar o HttpResponse

// Se o path final aponta para um diretório:
// Exemplo:
// GET /images/
// E o filesystem path é:
// /var/www/images/
// Você precisa verificar:
// Existe um index configurado?
// Se algum existir → serve esse.
// Se não:
// 		Se autoindex on → gera listagem
//		Senão → 403 ou 404 (dependendo da política)

#include "./http/RequestProcessor.hpp"

HttpResponse RequestProcessor::process(const HttpStruct& request, ManagerConfig& config) {
	HttpResponse response;
	ServerConfig& server = config.findServer(request.getPort(), request.getHostHeader());
	LocationConfig* location = matchLocation(request.getUri(), server);

	if (!location) {
		response.setStatus(HttpStatus::NOT_FOUND);
		handlePageErrors(response, server, location);
		return response;
	}
	if (location->hasRedirect()) {
		int code = location->getRedirectCode();
		response.setStatus(static_cast<HttpStatus::Code>(code));
		if (code >= 300 && code < 400) {
			response.addHeader("Location", location->getRedirectUrl());
		} else if (!location->getRedirectUrl().empty()) {
			// 2xx/4xx/5xx: second argument is a response body
			response.setBody(location->getRedirectUrl());
			response.addHeader("Content-Type", "text/plain");
			response.addHeader("Content-Length", itoa(static_cast<int>(location->getRedirectUrl().size())));
		}
		handlePageErrors(response, server, location);
		return response;
	}
	if (!isMethodAllowed(request.getMethod(), *location)) {
		response.setStatus(HttpStatus::METHOD_NOT_ALLOWED);
		handlePageErrors(response, server, location);
		return response;
	}
	
	if (request.getMethod() == "GET")
		handleGet(request, response, server, location);
	else if (request.getMethod() == "POST")
		handlePost(request, response, server, location);
	else
		handleDelete(request, response, server, location);

	handlePageErrors(response, server, location);
	return response;
}

// METHOD RELATED ---------------------------------------------------------------------------------

void RequestProcessor::handleGet(const HttpStruct& request, HttpResponse& response, ServerConfig& server, LocationConfig* location) {
	std::string filepath = resolveFilePath(server, location, request.getUri());

	if (isValidDirectory(filepath)) {
		if (filepath[filepath.size() - 1] != '/')
			filepath += '/';

		const std::vector<std::string>* index_files = NULL;
		if (location->hasIndexFiles())
			index_files = &location->getIndexFiles();
		else if (server.hasIndexFiles())
			index_files = &server.getIndexFiles();

		if (index_files) {
			for (size_t i = 0; i < index_files->size(); i++) {
				std::string index_path = filepath + (*index_files)[i];
				if (isValidFile(index_path)) {
					if (CgiHandler::tryRun(request, response, *location, index_path))
						return;
					serveFile(response, index_path, server, location);
					return ;
				}
			}
		}
		if (location->hasAutoindex() && location->getAutoindex()) {
			generateAutoindex(response, filepath, request.getUri());
			return ;
		}
		response.setStatus(HttpStatus::FORBIDDEN);
		return ;
	}
	
	if (!isValidFile(filepath)) {
		response.setStatus(HttpStatus::NOT_FOUND);
		return ;
	}

	if (CgiHandler::tryRun(request, response, *location, filepath))
		return;
	serveFile(response, filepath, server, location);
}


void RequestProcessor::handlePost(const HttpStruct& request, HttpResponse& response, ServerConfig& server, LocationConfig* location) {
	size_t max_body_size = location->hasMaxBodySize()
		? location->getClientMaxBodySize()
		: server.getClientMaxBodySize();
	if (request.getBody().size() > max_body_size) {
		response.setStatus(HttpStatus::CONTENT_TOO_LARGE);
		return;
	}
 
	const Headers& headers = request.getHeaders();
	Headers::ConstIterator ct_it = headers.find("Content-Type");
	if (ct_it != headers.end() && ct_it->second[0].find("multipart/form-data") != std::string::npos) {
		if (handleMultipart(request, response, location))
			response.setStatus(HttpStatus::CREATED);
		return;
	}
 
	std::string filepath = resolveFilePath(server, location, request.getUri());
 
	if (isValidFile(filepath) && CgiHandler::tryRun(request, response, *location, filepath))
		return;
	bool file_existed = false;
	std::string dest = resolvePostDest(request, response, location, filepath, file_existed);
	if (dest.empty())
		return;
 
	if (!writeToFile(dest, request.getBody())) {
		response.setStatus(HttpStatus::INTERNAL_SERVER_ERROR);
		return;
	}
 
	if (!file_existed) {
		response.setStatus(HttpStatus::CREATED);
		response.addHeader("Location", request.getUri());
		response.setBody("File created successfully");
	} else {
		response.setStatus(HttpStatus::OK);
		response.setBody("File updated successfully");
	}
	response.addHeader("Content-Type", "text/plain");
}
 
void RequestProcessor::handleDelete(const HttpStruct& request, HttpResponse& response, ServerConfig& server, LocationConfig* location) {
	std::string filepath = resolveFilePath(server, location, request.getUri());
 
	if (!isValidFile(filepath) && !isValidDirectory(filepath)) {
		response.setStatus(HttpStatus::NOT_FOUND);
		return;
	}
 
	if (isValidFile(filepath) && CgiHandler::tryRun(request, response, *location, filepath))
		return;
 
	if (!canDelete(filepath)) {
		response.setStatus(HttpStatus::FORBIDDEN);
		return;
	}
 
	if (isValidFile(filepath)) {
		if (unlink(filepath.c_str()) == 0)
			response.setStatus(HttpStatus::NO_CONTENT);
		else
			response.setStatus(HttpStatus::INTERNAL_SERVER_ERROR);
		return;
	}
 
	if (isValidDirectory(filepath)) {
		if (!isEmptyDirectory(filepath)) {
			response.setStatus(HttpStatus::CONFLICT);
			return;
		}
		if (rmdir(filepath.c_str()) == 0)
			response.setStatus(HttpStatus::NO_CONTENT);
		else
			response.setStatus(HttpStatus::INTERNAL_SERVER_ERROR);
		return;
	}
 
	response.setStatus(HttpStatus::INTERNAL_SERVER_ERROR);
	(void)server;
}


// File serving ---------------------------------------------------------------------------------
void RequestProcessor::serveFile(HttpResponse& response, const std::string& path, ServerConfig& server, LocationConfig* location) {
	if (access(path.c_str(), R_OK) != 0) {
		response.setStatus(HttpStatus::FORBIDDEN);
		return ;
	}

	struct stat info;
	if (stat(path.c_str(), &info) != 0 || info.st_size < 0) {
		response.setStatus(HttpStatus::INTERNAL_SERVER_ERROR);
		return ;
	}
	size_t file_size = static_cast<size_t>(info.st_size);
	size_t max_body_size;
	if (location->hasMaxBodySize())
		max_body_size = location->getClientMaxBodySize();
	else
		max_body_size = server.getClientMaxBodySize();
	if (file_size > max_body_size) {
		response.setStatus(HttpStatus::CONTENT_TOO_LARGE);
		return ;
	}
	
	std::ifstream file(path.c_str(), std::ios::binary);
	if (!file.is_open()) {
		response.setStatus(HttpStatus::INTERNAL_SERVER_ERROR);
		return;
	}

	std::string body(file_size, '\0');
	file.read(&body[0], static_cast<std::streamsize>(file_size));
	if (!file && !file.eof()) {
		response.setStatus(HttpStatus::INTERNAL_SERVER_ERROR);
		return;
	}

	response.setStatus(HttpStatus::OK);
	response.setBody(body);
	response.addHeader("Content-Type", guessMimeType(path));
	response.addHeader("Content-Length", itoa(static_cast<int>(file_size)));
}


// Autoindex ---------------------------------------------------------------------------------
void RequestProcessor::generateAutoindex(HttpResponse& response, const std::string& dirpath, const std::string& uri) {
	DIR* dir = opendir(dirpath.c_str());
	if (!dir) {
		response.setStatus(HttpStatus::INTERNAL_SERVER_ERROR);
		return;
	}
 
	std::ostringstream html;
	html << "<!DOCTYPE html>\n<html>\n<head><meta charset=\"UTF-8\">"
	     << "<title>Index of " << uri << "</title></head>\n"
	     << "<body>\n<h1>Index of " << uri << "</h1>\n<hr>\n<pre>\n";
 
	struct dirent* entry;
	while ((entry = readdir(dir)) != NULL) {
		std::string name = entry->d_name;
		if (name == ".")
			continue;
 
		std::string fullpath = dirpath + name;
		struct stat info;
		std::string display = name;
		std::string size_str = "-";
 
		if (stat(fullpath.c_str(), &info) == 0) {
			if (S_ISDIR(info.st_mode))
				display += "/";
			else
				size_str = itoa(static_cast<int>(info.st_size));
		}
 
		std::string href = uri;
		if (href[href.size() - 1] != '/')
			href += '/';
		href += name;
		if (S_ISDIR(info.st_mode))
			href += '/';
 
		html << "<a href=\"" << href << "\">" << display << "</a>"
		     << "\t\t" << size_str << "\n";
	}
	closedir(dir);
 
	html << "</pre>\n<hr>\n</body>\n</html>\n";
 
	std::string body = html.str();
	response.setStatus(HttpStatus::OK);
	response.setBody(body);
	response.addHeader("Content-Type", "text/html");
	response.addHeader("Content-Length", itoa(static_cast<int>(body.size())));
}

// Method utils ---------------------------------------------------------------------------------
 
// Extracts an attribute value from a header field, e.g. "boundary" from Content-Type or "filename" from Content-Disposition
std::string RequestProcessor::extractHeaderAttribute(const std::string& header, const std::string& key) {
	size_t pos = header.find(key + "=");
	if (pos == std::string::npos)
		return "";
	pos += key.size() + 1;
	if (pos < header.size() && header[pos] == '"') {
		pos++;
		size_t end = header.find('"', pos);
		if (end != std::string::npos)
			return header.substr(pos, end - pos);
	} else {
		size_t end = header.find_first_of(" ;\r\n", pos);
		return header.substr(pos, end == std::string::npos ? end : end - pos);
	}
	return "";
}

// Returns the filename from Content-Disposition header, or a timestamp-based fallback
std::string RequestProcessor::extractFilename(const std::string& disposition) {
	std::string filename = extractHeaderAttribute(disposition, "filename");
	if (!filename.empty())
		return filename;
	std::ostringstream ss;
	ss << "upload_" << time(NULL);
	return ss.str();
}

// Writes body to path, returns false on failure
bool RequestProcessor::writeToFile(const std::string& dest, const std::string& body) {
	std::ofstream out(dest.c_str(), std::ios::binary | std::ios::trunc);
	if (!out.is_open())
		return false;
	out.write(body.c_str(), static_cast<std::streamsize>(body.size()));
	return out.good();
}

// Resolves the destination path for a POST and sets file_existed.
// Returns an empty string and sets the response status on error.
std::string RequestProcessor::resolvePostDest(const HttpStruct& request, HttpResponse& response, LocationConfig* location, const std::string& filepath, bool& file_existed) {
	if (isValidDirectory(filepath)) {
		std::string upload_dir = location->getUpload();
		if (upload_dir.empty() || !isValidDirectory(upload_dir) || access(upload_dir.c_str(), W_OK) != 0) {
			response.setStatus(HttpStatus::FORBIDDEN);
			return "";
		}
		if (upload_dir[upload_dir.size() - 1] != '/')
			upload_dir += '/';
		const Headers& hdrs = request.getHeaders();
		Headers::ConstIterator cd_it = hdrs.find("Content-Disposition");
		std::string disposition = (cd_it != hdrs.end()) ? cd_it->second[0] : "";
		std::string dest = upload_dir + extractFilename(disposition);
		file_existed = isValidFile(dest);
		return dest;
	}
 
	if (isValidFile(filepath)) {
		if (access(filepath.c_str(), W_OK) != 0) {
			response.setStatus(HttpStatus::FORBIDDEN);
			return "";
		}
		file_existed = true;
		return filepath;
	}
 
	// Path doesn't exist: create it if upload is configured and parent is writable
	if (location->getUpload().empty()) {
		response.setStatus(HttpStatus::FORBIDDEN);
		return "";
	}
	size_t slash = filepath.rfind('/');
	std::string parent = (slash != std::string::npos) ? filepath.substr(0, slash) : ".";
	if (!isValidDirectory(parent)) {
		response.setStatus(HttpStatus::NOT_FOUND);
		return "";
	}
	if (access(parent.c_str(), W_OK) != 0) {
		response.setStatus(HttpStatus::FORBIDDEN);
		return "";
	}
	file_existed = false;
	return filepath;
}
 

// Handles multipart/form-data uploads: parses each part and saves files to upload_dir
bool RequestProcessor::handleMultipart(const HttpStruct& request, HttpResponse& response, LocationConfig* location) {
	const Headers& headers = request.getHeaders();
 
	std::string upload_dir = location->getUpload();
	if (upload_dir.empty() || !isValidDirectory(upload_dir) || access(upload_dir.c_str(), W_OK) != 0) {
		response.setStatus(HttpStatus::FORBIDDEN);
		return false;
	}
 
	Headers::ConstIterator ct_it = headers.find("Content-Type");
	if (ct_it == headers.end()) {
		response.setStatus(HttpStatus::BAD_REQUEST);
		return false;
	}
	std::string boundary = extractHeaderAttribute(ct_it->second[0], "boundary");
	if (boundary.empty()) {
		response.setStatus(HttpStatus::BAD_REQUEST);
		return false;
	}
 
	std::string sep = "--" + boundary;
	const std::string& body = request.getBody();
 
	size_t pos = body.find(sep);
	if (pos == std::string::npos) {
		response.setStatus(HttpStatus::BAD_REQUEST);
		return false;
	}
	pos += sep.size();
 
	while (true) {
		if (pos + 2 <= body.size() && body.compare(pos, 2, "--") == 0)
			break;
 
		if (pos + 2 <= body.size() && body.compare(pos, 2, CRLF) == 0)
			pos += 2;
 
		size_t headers_end = body.find("\r\n\r\n", pos);
		if (headers_end == std::string::npos) {
			response.setStatus(HttpStatus::BAD_REQUEST);
			return false;
		}
 
		std::string part_headers = body.substr(pos, headers_end - pos);
		size_t file_start = headers_end + 4;
 
		size_t next_boundary = body.find(sep, file_start);
		if (next_boundary == std::string::npos) {
			response.setStatus(HttpStatus::BAD_REQUEST);
			return false;
		}
 
		size_t cd_pos = part_headers.find("Content-Disposition:");
		if (cd_pos != std::string::npos) {
			size_t cd_end = part_headers.find(CRLF, cd_pos);
			std::string cd_value = part_headers.substr(cd_pos, cd_end == std::string::npos ? cd_end : cd_end - cd_pos);
			std::string filename = extractFilename(cd_value);
 
			if (!filename.empty()) {
				size_t file_end = next_boundary;
				if (file_end >= 2 && body.compare(file_end - 2, 2, CRLF) == 0)
					file_end -= 2;
 
				if (upload_dir[upload_dir.size() - 1] != '/')
					upload_dir += '/';
				std::string out_path = upload_dir + filename;
				size_t data_size = (file_end > file_start) ? file_end - file_start : 0;
 
				if (!writeToFile(out_path, body.substr(file_start, data_size))) {
					response.setStatus(HttpStatus::INTERNAL_SERVER_ERROR);
					return false;
				}
			}
		}
		pos = next_boundary + sep.size();
	}
 
	response.setBody("Files uploaded successfully");
	response.addHeader("Content-Type", "text/plain");
	return true;
}

// Checks that the parent directory (and the target itself if a directory)
// grant write+execute permission, which is required to remove an entry
bool RequestProcessor::canDelete(const std::string& path) {
	size_t pos = path.find_last_of('/');
	std::string parent;
	if (pos == std::string::npos)
		parent = ".";
	else if (pos == 0)
		parent = "/";
	else
		parent = path.substr(0, pos);
 
	if (access(parent.c_str(), W_OK | X_OK) != 0)
		return false;
	if (isValidDirectory(path) && access(path.c_str(), W_OK | X_OK) != 0)
		return false;
	return true;
}

// Routing helpers ---------------------------------------------------------------------------------


LocationConfig* RequestProcessor::matchLocation(std::string request_uri, ServerConfig& server) {
	unsigned long match_length = 0;
	LocationConfig* best_match = NULL;

	// Strip query string before prefix matching
	size_t q = request_uri.find('?');
	if (q != std::string::npos)
		request_uri = request_uri.substr(0, q);

	if (request_uri.length() > 1 && request_uri[request_uri.length() - 1] == '/')
		request_uri.erase(request_uri.length() - 1);

	for (size_t i = 0; i < server.getLocations().size(); i++) {
		const std::string& prefix = server.getLocations()[i].getPathPrefix();
		if (prefix.length() > request_uri.length())
			continue;
		if (request_uri.compare(0, prefix.length(), prefix) == 0) {
			if (request_uri.length() > prefix.length() && request_uri[prefix.length()] != '/' && prefix.length() > 1)
				continue;
			if (prefix.length() > match_length) {
				match_length = prefix.length();
				best_match = &server.getLocations()[i];
			}
		}
	}
	return best_match;
}

bool RequestProcessor::isMethodAllowed(const std::string& method, const LocationConfig& location) {
	const std::vector<std::string>& allowed_methods = location.getMethods();
	for (size_t i = 0; i < allowed_methods.size(); i++) {
		//verificar se Luiz ja ta me enviando tudo em uppercase
		if (allowed_methods[i] == method)
			return true;
	}
	return false;
}


std::string RequestProcessor::resolveFilePath(ServerConfig& server, LocationConfig* location, std::string uri) {
	std::string filepath;

	// Strip query string — it's for CGI env, not the filesystem path
	size_t q = uri.find('?');
	if (q != std::string::npos)
		uri = uri.substr(0, q);

	if (location->hasRoot()) {
		std::string relative = uri.substr(location->getPathPrefix().length());
		if (relative.empty())
			relative = "/";
		if (relative[0] != '/')
			relative = "/" + relative;
		filepath = location->getRoot() + relative;
	}
	else {
		if (uri[0] != '/')
			uri = "/" + uri;
		filepath = server.getRoot() + uri;
	}
	return filepath;
}

void RequestProcessor::handlePageErrors(HttpResponse& response, ServerConfig& server, LocationConfig* location) {
	const int &status = response.getStatus();
	
	typedef std::map<int, std::string>	ErrorPage;
	typedef ErrorPage::const_iterator	ErrorPageIt;


	const ErrorPage&	location_pages = location->getErrorPages();
	for (ErrorPageIt it = location_pages.begin(); it != location_pages.end(); ++it) {
		if (it->first == status) {
			serveFile(response, it->second, server, location);
			return;
		}
	}

	const ErrorPage&	server_pages = server.getErrorPages();
	for (ErrorPageIt it = server_pages.begin(); it != server_pages.end(); ++it) {
		if (it->first == status) {
			serveFile(response, it->second, server, location);
			return;
		}
	}
		
}