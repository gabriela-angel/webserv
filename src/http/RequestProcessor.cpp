
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

#include "RequestProcessor.hpp"

HttpResponse RequestProcessor::process(const HttpRequest& request, const ManagerConfig& config) {
	HttpResponse response;
	ServerConfig& server = config.findServer(request.getPort(), request.getHostHeader());
	LocationConfig* location = matchLocation(request.getUri(), server);

	if (!location) {
		// No matching location, handle with server config (e.g., return 404 or default page)
		response.setStatus(HttpStatus::NOT_FOUND);
		// should I implement error pages in here??
		return response;
	}
	if (location->hasRedirect()) {
		response.setStatus(static_cast<HttpStatus::Code>(location->getRedirectCode()));
		if (!(location->getRedirectUrl().empty()))
			response.addHeader("Location", location->getRedirectUrl());
		// should I implement error pages in here??
		return response;
	}
	if (!isMethodAllowed(request.getMethod(), *location)) {
		response.setStatus(HttpStatus::METHOD_NOT_ALLOWED);
		// should I implement error pages in here??
		return response;
	}
	if (location->hasRoot()) {
		std::string relative = request.getUri().substr(location->getPathPrefix().length());
		if (relative.empty())
			relative = "/";
		//let's make sure root never ends with a slash to avoid double slashes
		_filepath = location->getRoot() + relative;
	}
	else {
		_filepath = server.getRoot() + request.getUri();
	}
	processMethods(request, response, location, server);

	return response;
}

void RequestProcessor::processMethods(const HttpRequest& request, HttpResponse& response, LocationConfig* location, ServerConfig& server) {
	
}


	if (isValidDirectory(_filepath)) {
		if (location->hasIndexFiles())
			handleIndexFiles();
		else if (location->getAutoindex())
			handleAutoindex();
		else {
			response.setStatus(HttpStatus::FORBIDDEN);
			return response;
		}
	}
	else if (!isValidFile(_filepath)) {
		response.setStatus(HttpStatus::NOT_FOUND);
		return response;
	}

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
