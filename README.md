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

- Build:

```bash
make
```

- Run:

```bash
./webserv <path_to_file.conf>
```

- Notes:
  - config files and test sites are available under `config/` and `data/`.
  - Logs are written to the `logs/` directory.

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
- Linux `epoll` man page and socket programming resources
- MDN Web Docs — HTTP: https://developer.mozilla.org/en-US/docs/Web/HTTP

**AI usage**
- AI assistance was used to draft this `README.md` (structuring, phrasing and examples) and to explain concepts studied for this project.