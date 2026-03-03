#ifndef BASECONFIG_HPP
# define BASECONFIG_HPP

# include <algorithm>
# include <cerrno>
# include <map>
# include <stdexcept>
# include <string>
# include <sys/stat.h>
# include <vector>


# define DEFAULT_CLIENT_MAX_BODY_SIZE 1048576 // CHECK MACRO LUIZ LATER

class BaseConfig {
protected:
	bool						_has_root;
	std::string 				_root; //might be on location -- either make it mndatory on config or provide a default
	std::vector<std::string>	_methods;// create enum for methods ?
	bool						_has_autoindex;
	bool						_autoindex;
	bool						_has_index_files;
	std::vector<std::string>	_index_files;
	std::map<int, std::string>	_error_pages; //if not defined we need to return default ones
	bool						_has_redirect;
	int							_redirect_code;
	std::string					_redirect_url;
	bool						_has_client_max_body_size;
	size_t						_client_max_body_size; //currently not accepting suffixes like 1M, 500K, etc. but we can add that later

	void setRoot(const std::vector<std::string>& value);
	void setMethods(const std::vector<std::string>& value);
	void setAutoindex(const std::vector<std::string>& value);
	void setIndexFiles(const std::vector<std::string>& value);
	void setErrorPage(const std::vector<std::string>& value);
	void setRedirect(const std::vector<std::string>& value);
	void setClientMaxBodySize(const std::vector<std::string>& value);

	bool isValidDirectory(const std::string& path);
	std::string toUpper(std::string str);
public:
	BaseConfig();
	BaseConfig(const BaseConfig& copy);
	BaseConfig& operator=(const BaseConfig& other);
	virtual ~BaseConfig();

	const std::string& 					getRoot() const;
	const std::vector<std::string>& 	getMethods() const;
	const bool& getAutoindex() const;
	const std::vector<std::string>& 	getIndexFiles() const;
	const std::map<int, std::string>& 	getErrorPages() const;
	const bool& 						getHasRedirect() const;
	const int& 							getRedirectCode() const;
	const std::string& 					getRedirectUrl() const;
	const size_t& 						getClientMaxBodySize() const;
};

#endif