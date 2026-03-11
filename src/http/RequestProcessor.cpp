
// Quando request chega:
// Recebe HttpRequest - DONE
// Descobre o ServerConfig correto (host + port) - DONE
// Encontra a LocationConfig mais específica (match por prefixo) -DONE
// Verifica se há redirect
// Se sim → resposta 301/302 e para aqui
// Verifica se o método é permitido
// Se não → 405
// Resolve root (location → server fallback)
// Constrói path no filesystem
// Continua fluxo (index, autoindex, etc.)
// Verificar se é diretório
// Resolver index
// Verificar existência
// Executar GET/POST/DELETE
// Criar o HttpResponse

// to create file path -> root + best match location
// if best match location ahs root, use it, otherwise use server root

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

// Depois que você sabe qual location vale:
// request.method ∈ allowed_methods ?
// Se não:
// → retornar 405

// Redirecionamento (301 / 302)
// Se a location tiver algo como:
// return 301 /new-path;

// Você:
// Não tenta acessar filesystem

// Gera Response com:
// status 301 ou 302
// header Location: novo_path
// body opcional

#include "RequestProcessor.hpp"

HttpResponse RequestProcessor::process(const HttpRequest& request, const ManagerConfig& config) {
	HttpResponse response();
	ServerConfig& server = config.findServer(request.getPort(), request.getHostHeader());
	LocationConfig* location = matchLocation(request.getUri(), server);

	if (!location) {
		// No matching location, handle with server config (e.g., return 404 or default page)
		response.setStatus(NOT_FOUND);
		return response;
	}
	if (location->getHasRedirect()) {
		response.setStatus(location->getRedirectCode());
		// THIS MIGHT CHANGE, CHECK HOW REDIREECT USUALLY APPEARS IN REPONSE
		response.addHeader("Location", location->getRedirectUrl());
		return response;
	}
	if (!isMethodAllowed(request.getMethod(), *location)) {
		response.setStatus(METHOD_NOT_ALLOWED);
		// check if that's all we need to set for 405 response
		return response;
	}
	//resolver root
	//send to method functions
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
