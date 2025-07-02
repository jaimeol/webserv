Made in colaboration with [rpisoner](https://github.com/rpisoner)

# Webserv

**Webserv** is a basic HTTP/1.1 web server developed in C++ as part of the 42 curriculum. The goal is to understand the fundamentals of how web servers operate by implementing one from scratch without using high-level libraries.

## 📚 Objectives

- Understand and implement the HTTP/1.1 protocol.
- Handle multiple client connections using non-blocking I/O and `poll()`.
- Serve static files and respond to HTTP requests correctly.
- Support basic configuration parsing similar to `nginx`.

## ⚙️ Features

- **HTTP Methods**: Supports `GET`, `POST`, and `DELETE`.
- **Multiple CGI Support**: Executes CGI scripts for dynamic content generation.
- **Multiple Server Blocks**: Parses configuration to handle multiple virtual servers with different ports and hostnames.
- **Request Routing**: Based on location blocks similar to Nginx.
- **Directory Listing**: Autoindex feature when no index file is present.
- **Custom Error Pages**: Configurable error pages per status code.
- **File Uploads**: Handles file upload via `POST` requests.
- **Connection Management**: Uses `poll()` to manage multiple clients efficiently.
- **Chunked Transfer Encoding**: Supports chunked responses as required by HTTP/1.1.
- **Cookies supported**: The server supports session and visit cookies.

## ✅ Test
You can test the HTTP methods by using curl or directly acceding to the website
- POST

  curl -X POST -F "file=@file_path" http://localhost:8080/uploads
- GET

  curl -X http://localhost:8080/
- DELETE

  curl -X DELETE http://localhost:8080/file_to_delete

Take into account that the methods must be allowed in the location you want to test them (you can check the allowed methods in the config file).
## 🛠️ Usage

```bash
make
./webserv config.conf
