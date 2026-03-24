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

HttpResponse RequestProcessor::process(const HttpRequest& request, const ManagerConfig& config) {
	HttpResponse response;
	ServerConfig& server = config.findServer(request.getPort(), request.getHostHeader());
	LocationConfig* location = matchLocation(request.getUri(), server);

	if (!location) {
		response.setStatus(HttpStatus::NOT_FOUND);
		// 	processError(?);
		// No matching location, handle with server config (e.g., return 404 or default page)
		return response;
	}
	if (location->hasRedirect()) {
		int code  = location->getRedirectCode();
		if (code >= 300 && code < 400)
		// processRedirect(?);
			response.setStatus(static_cast<HttpStatus::Code>(code));
			response.addHeader("Location", location->getRedirectUrl());
		// else
		// 	processError(?);
		return response;
	}
	if (!isMethodAllowed(request.getMethod(), *location)) {
		response.setStatus(HttpStatus::METHOD_NOT_ALLOWED);
		//processError(?);
		return response;
	}
	
	if (request.getMethod() == "GET") {
		handleGet(request, response, server, location);
	}
	else if (request.getMethod() == "POST") {
		handlePost(request, response, server, location);
	}
	else {
		handleDelete(request, response, server, location);
	}

	return response;
}

// METHOD RELATED -----------------------------------------------------------------------------------

void RequestProcessor::handleGet(const HttpRequest& request, HttpResponse& response, ServerConfig& server, LocationConfig* location) {
	std::string filepath = resolveFilePath(server, location, request.getUri());

	if (isValidDirectory(filepath)) {
		if (location->hasIndexFiles()) {
			std::vector<std::string> index_files = location->getIndexFiles();
			for (size_t i = 0; i < index_files.size(); i++) {
				std::string index_path;
				index_path = filepath + index_files[i];
				//DEBUG
				std::cout << index_path;
				if (isValidFile(index_path)) {
					//check if it's cgi
					//if not
					serveStatic(response,index_path);
					return ;
				}
			}
		}
		if (location->getAutoindex()) {
			generateAutoindex();
			return ;
		}
		response.setStatus(HttpStatus::FORBIDDEN);
		return ;
	}
	else if (!isValidFile(filepath)) {
		response.setStatus(HttpStatus::NOT_FOUND);
		return ;
	}
	//check if it's cgi
	//if not
	serveStatic(response, filepath);
}

void RequestProcessor::serveStatic(HttpResponse& response, const std::string& path) {
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
		max_body_size = server->getClientMaxBodySize();
	if (file_size > max_body_size) {
		response.setStatus(HttpStatus::CONTENT_TOO_LARGE);
		return ;
	}
	
	std::ifstream file(path.c_str(), std::ios::binary);
	std::vector<char> buffer(file_size);
	file.read(buffer.data(), file_size);
	if (!file){
		response.setStatus(HttpStatus::INTERNAL_SERVER_ERROR);
		return ;
	}

	response.setBody(std::string(buffer.begin(), buffer.end()));
	response.addHeader("Content-Type", FileUtils::guessMimeType(path));	
	response.setStatus(HttpStatus::OK);
}




// -------------------------------------------------------------------------------------


LocationConfig* RequestProcessor::matchLocation(std::string request_uri, ServerConfig& server) {
	unsigned long match_length = 0;
	LocationConfig* best_match = nullptr;

	if (request_uri.length() > 1 && request_uri[request_uri.length() - 1] == '/')
		request_uri.erase(request_uri.length() - 1);
	for (size_t i = 0; i < server.getLocations().size(); i++) {
		const std::string& prefix = server.getLocations()[i].getPathPrefix();
		if (prefix.length() > request_uri.length())
			continue;
		if (request_uri.compare(0, prefix.length(), prefix) == 0) {
			if (request_uri.length() > prefix.length() && request_uri[prefix.length()] != '/') {
				continue;
			}
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