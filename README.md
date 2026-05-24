*This project has been created as part of the 42 curriculum by lhenriqu, gangel-a*

# **webserv**

Small, educational HTTP web server implemented in C++ (42 project).

## **Description**
Implement a minimal but realistic HTTP server to learn how web servers work.

### **Overview**
- Virtual hosts and multiple server configs
- Static files, directory listing, and custom error pages
- CGI handling for dynamic content (Python, Go examples in `data/cgi/`)
- Basic PHP support via `php-cgi`
- Non-blocking I/O using `epoll`
- Configurable locations, redirects and methods

**Instructions**

- Prerequisites:
  - A Linux system with a C++ compiler (g++), `make`, and standard build tools.
  - `php-cgi` installed if you want to run PHP tests.

- Build:

```bash
make
```

- Run:

```bash
./webserv config/multiservers.conf
```

- Notes:
  - The server reads configuration files from the `config/` directory. Use `config/multiservers.conf`, `config/defaultserver.conf` or your own config file.
  - Document roots and test sites are available under `data/` (e.g., `data/oficial_curl.sh`, `data/ninjas_server/`, `data/php_server/`).
  - Logs are written to the `logs/` directory when enabled by the configuration.

**Usage examples**

- Request a static file:

```bash
curl -i http://localhost:8080/
```

- Test CGI script (example):

```bash
curl -i http://localhost:8080/cgi/cadet_list.py
```

**Configuration**
- Check the `config/` directory for sample configurations. Files use a simple nginx-like syntax with `server` and `location` blocks. Ports, server_names, root paths, error pages and CGI handlers are configured there.

**Resources**
- HTTP/1.1 and related RFCs: RFC 7230, RFC 7231
- CGI specification: https://www.w3.org/CGI/
- Linux `epoll` man page and socket programming resources
- MDN Web Docs — HTTP: https://developer.mozilla.org/en-US/docs/Web/HTTP
- C++ reference and STL documentation

**AI usage**
- AI assistance was used to draft this `README.md` (structuring, phrasing and examples). All code in the repository was implemented by the author; AI did not write or modify source files beyond documentation.

**Troubleshooting & Tips**
- If the server fails to bind to a port <1024, run it as root or choose a non-privileged port (e.g., 8080).
- Ensure `php-cgi` is in your PATH before testing PHP pages.
- Use `logs/` and the provided `data/` test sites to reproduce common behaviors.

**Contact**
- For questions about this project, open an issue in the repository or contact the author.
