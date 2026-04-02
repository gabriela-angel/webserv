#include "./config/ParseConfig.hpp"
 
ParseConfig::ParseConfig(const std::string& filename) : _filename(filename), _line_counter(0), _context(GLOBAL) { }
 
ParseConfig::ParseConfig(const ParseConfig& copy){
	*this = copy;
}
 
ParseConfig& ParseConfig::operator=(const ParseConfig& other){
	if (this != &other) {
		_filename = other._filename;
		_line_counter = other._line_counter;
		_context = other._context;
	}
	return *this;
}
 
ParseConfig::~ParseConfig(){}
 
std::vector<ServerConfig> ParseConfig::parse() {
	std::vector<ServerConfig> servers;
 
	if (_filename.length() < 5 || _filename.substr(_filename.length() - 5) != ".conf")
		throw std::invalid_argument("Invalid file extension: \"" + _filename + "\". Expected a .conf file.");
	std::fstream file(_filename.c_str(), std::ios::in);
	if (!file.is_open())
		throw std::invalid_argument("Unable to open file: \"" + _filename + "\"");
	if (file.peek() == std::ifstream::traits_type::eof()) {
		file.close();
		throw std::invalid_argument("Empty configuration file: \"" + _filename + "\"");
	}
	std::string line;
	std::string remainder;
 
	while (getline(file, remainder)) {
		_line_counter++;
 
		cutComments(remainder);
		remainder = trim(remainder);
		while (!remainder.empty()) {
			size_t pos = remainder.find_first_of(";{}");
			if (pos != (remainder.length() - 1)) {
				line = remainder.substr(0, pos + 1);
				remainder = remainder.substr(pos + 1);
			}
			else {
				line = remainder;
				remainder = "";
			}
 
			line = trim(line);
 
			std::string key;
			std::vector<std::string> value;
			if(!setKeyValue(line, key, value))
				continue;
 
			if (setContext(key, value, servers))
				continue;
			parseDirective(key, value, servers);
		}
	}
	
	if (!file.eof()) {
		file.close();
		throw std::runtime_error("Failed to read data from file: \"" + _filename + "\"");
	}
 
	file.close();
	if (servers.empty())
		throw std::runtime_error("At least one server block must be defined in the configuration file");
 
	for (size_t i = 0; i < servers.size(); i++) {
		if (servers[i].getPortPair().empty())
			throw std::runtime_error("Each server block must have at least one listen directive");
		if (!servers[i].hasRoot()) {
			// Root is not required if every location has a return directive
			const std::vector<LocationConfig>& locs = servers[i].getLocations();
			bool all_have_return = !locs.empty();
			for (size_t j = 0; j < locs.size(); j++) {
				if (!locs[j].hasRedirect()) {
					all_have_return = false;
					break;
				}
			}
			if (!all_have_return)
				throw std::runtime_error("Each server block must have a root directive");
		}
	}
 
	return servers;
}
 
void ParseConfig::parseDirective(const std::string& key, const std::vector<std::string>& value, std::vector<ServerConfig>& servers) {
	switch (_context) {
		case GLOBAL:
			throw std::runtime_error("Syntax error: line " + itoa(_line_counter) + ": Directive '" + key + "' is not allowed in global context");
		case SERVER:
			servers[servers.size() - 1].setDirective(key, value);
			break;
		case LOCATION:
			LocationConfig* currentLocation = &servers[servers.size() - 1].getLocations().back();
			currentLocation->setDirective(key, value);
			break;
	}
}
 
bool ParseConfig::setContext(const std::string& key, const std::vector<std::string>& value, std::vector<ServerConfig>& servers) {
	if (key == "server") {
		if (_context != GLOBAL)
			throw std::runtime_error("Syntax error: line " + itoa(_line_counter) + ": 'server' block cannot be nested");
		_context = SERVER;
		servers.push_back(ServerConfig());
		return true;
	} else if (key == "location") {
		if (_context != SERVER)
			throw std::runtime_error("Syntax error: line " + itoa(_line_counter) + ": 'location' block must be inside a 'server' block");
		_context = LOCATION;
		servers[servers.size() - 1].addLocation(LocationConfig(value));
		return true;
	} else if (key == "}") {
		if (_context == LOCATION)
			_context = SERVER;
		else if (_context == SERVER)
			_context = GLOBAL;
		else
			throw std::runtime_error("Syntax error: line " + itoa(_line_counter) + ": Unexpected '}'");
		return true;
	}
	return false;
}
 
bool ParseConfig::setKeyValue(const std::string& line, std::string& key, std::vector<std::string>& value) {
	std::istringstream iss(line);
	std::string token;
 
	if (line.empty())
		return false;
 
	iss >> key;
 
	if (key != "server" && key != "location" && key != "}" && line[line.size() - 1] != ';')
		throw std::runtime_error("Syntax error: line " + itoa(_line_counter) + ": Missing ';'");
 
	while (iss >> token) {
		if (token[token.size() - 1] == ';')
			token.erase(token.size() - 1);
		if (!token.empty())
			value.push_back(token);
	}
 
	return true;
}
 
void ParseConfig::cutComments(std::string& line) {
	bool in_single = false;
	bool in_double = false;
 
	for (size_t i = 0; i < line.length(); i++)
	{
		if (line[i] == '\'' && !in_double)
			in_single = !in_single;
		else if (line[i] == '"' && !in_single)
			in_double = !in_double;
		else if (line[i] == '#' && !in_single && !in_double)
		{
			line = line.substr(0, i);
			break;
		}
	}
	if (in_single || in_double) {
		throw std::runtime_error("Invalid syntax: line " + itoa(_line_counter) + ": Unclosed quote");
	}
}
 
// Arquivo .conf
//         ↓
// ConfigParser
//         ↓
// Estruturas em memória
//         ↓
// ServerManager (guarda todos os servers)
//         ↓
// Request chega
//         ↓
// Você usa as estruturas para decidir o que fazer
 
// Webserv
//  ├── vários ServerConfig
//  │     ├── várias LocationConfig
//  │
//  └── (opcional) Config global
 
// - [ ] Suporte inicial:
// 	- porta
// 	- root
// 	- index 
// 	- error_page  
// - [ ] Validação mínima (config inválido → erro)
 
// Se não houver root definido em lugar nenhum:
// 	erro de configuração.
// 	500 Internal Server Error
// if (!server.hasRoot())
//     throw ConfigError("Server must define a root");
 
//VALIDAR:
// Existe pelo menos 1 server?
// Cada server tem listen?
// Cada server tem root? (se você decidiu que é obrigatório)
// Não há duplicação absurda? -> exemplo:
	// dois servers com mesma porta e name
	// dois servers escutando na mesma porta e mesmo host sem lógica clara
	// duas locations iguais dentro do mesmo server
	// diretivas repetidas que não deveriam repetir (como dois roots)
 
// MANTER EM MENTE QUE O PORT PODE SER:
// listen 127.0.0.1:8080;
// OU
// listen 8080;