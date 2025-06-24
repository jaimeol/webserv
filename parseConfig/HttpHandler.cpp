#include "../inc/HttpHandler.hpp"
#include <ctime>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdio>

static bool isFile(const std::string& path) {
	struct stat s;
	return stat(path.c_str(), &s) == 0 && S_ISREG(s.st_mode);
}

//HttpHandler::HttpHandler(const std::vector<Server>& servers) : _servers(servers) {}

std::string HttpHandler::matchCookie(const HttpRequest& req, HttpResponse& res) {
    std::string sessionId;
    std::map<std::string, std::string>::const_iterator it = req.headers.find("Cookie");

    if (it != req.headers.end()) {
        std::string cookies = it->second;
        std::size_t pos = cookies.find("sessionId=");
        if (pos != std::string::npos) {
            sessionId = cookies.substr(pos + 10);
            std::size_t end = sessionId.find(';');
            if (end != std::string::npos)
                sessionId = sessionId.substr(0, end);
        }
    }

    if (sessionId.empty() || !sessionManager.hasSession(sessionId)) {
        sessionId = sessionManager.generateSessionId();
        sessionManager.createSession(sessionId);
        res.headers["Set-Cookie"] = "sessionId=" + sessionId + "; Path=/; HttpOnly";
    }

    return sessionId;
}

void HttpHandler::handleVisits(std::map<std::string, std::string>& session, HttpResponse& res){
    if (session.find("visits") == session.end()) {
        session["visits"] = "1";
    } else {
        int count = std::atoi(session["visits"].c_str());
        count++;
        std::ostringstream oss;
        oss << count;
        session["visits"] = oss.str();
    }

    // Añadir cookie de visitas
    std::string cookie = "visits=" + session["visits"] + "; Path=/";
    if (res.headers.find("Set-Cookie") != res.headers.end())
        res.headers["Set-Cookie"] += "; " + cookie;
    else
        res.headers["Set-Cookie"] = cookie;
}

const Server& HttpHandler::matchServer(const HttpRequest& req, const std::vector<Server>& servers) {
	std::string hostname;
	std::string portStr;

	std::string hostHeader;
	std::map<std::string, std::string>::const_iterator it = req.headers.find("Host");
	if (it != req.headers.end())
	{
		hostHeader = it->second;
	}

	std::size_t colonPos = hostHeader.find(':');
	if (colonPos != std::string::npos) {
		hostname = hostHeader.substr(0, colonPos);
		portStr = hostHeader.substr(colonPos + 1);
	} else {
		hostname = hostHeader; // no había puerto
		portStr = "80";       // valor por defecto
	}

	if (hostname == "localhost")
		hostname = "127.0.0.1";
		
	std::cout << "FURULA" << std::endl;


	in_addr_t host = inet_addr(hostname.c_str());
	if (host == INADDR_NONE) {
		throw std::runtime_error("Host inválido: " + hostname);
	}


	struct in_addr addr;
	addr.s_addr = host;

	std::cout << inet_ntoa(addr) << std::endl;

	std::istringstream portStream(portStr);
	int portInt = 80;
	portStream >> portInt;

	if (portInt < 0 || portInt > 65535) {
		throw std::runtime_error("Puerto inválido");
	}

	uint16_t port = static_cast<uint16_t>(portInt);

	std::cout << "--SEEKING--" << std::endl;
	std::cout << "Host: " << host << " Port: " << port << std::endl;
	//std::cout << "Host:" << inet_ntoa(addr) << " Port: " << port << std::endl;

	for (size_t i = 0; i < servers.size(); ++i) {
		std::cout << "Servers[" << i << "] -> Host: " << servers[i].getHost() << " Port: " << servers[i].getPort() << std::endl;
		if (servers[i].getPort() == port && servers[i].getHost() == host) {
			return servers[i];
		}
	}
	throw std::runtime_error("No matching server for host: " + host);
}

Location HttpHandler::matchLocation(const std::string& uri, const Server& server) {
	const std::vector<Location>& locs = server.getLocations();
	const Location* bestMatch = NULL;
	size_t bestLength = 0;

	for (size_t i = 0; i < locs.size(); ++i) {
		if (uri.find(locs[i].getPath()) == 0 && locs[i].getPath().length() > bestLength) {
			bestLength = locs[i].getPath().length();
			bestMatch = &locs[i];
		}
	}
	if (!bestMatch)
		throw std::runtime_error("No matching location for URI: " + uri);
	return *bestMatch;
}


std::string HttpHandler::readFileContent(const std::string& path) {
	std::ifstream f(path.c_str());
	if (!f.is_open())
		throw std::runtime_error("Cannot read file: " + path);
	std::stringstream buffer;
	buffer << f.rdbuf();
	return buffer.str();
}

HttpResponse HttpHandler::handleGET(const HttpRequest& req, const Location& loc) {
	HttpResponse res;
	res.version = "HTTP/1.1";
	std::cout << "Location root " << loc.getRoot() << std::endl;
	std::cout << "Location path " << loc.getPath() << std::endl;
	std::cout << "Location index: " << loc.getIndex() << std::endl;
	std::string fullPath;
	if (loc.getPath() == "/cgi-bin")
		fullPath = "." + req.uri;
	else
		fullPath = loc.getRoot() + req.uri;
	std::cout << "FULL PATH: " << fullPath << std::endl;
	if (isFile(fullPath)) {
		res.status_code = 200;
		res.status_text = "OK";
		res.body = readFileContent(fullPath);
	} else {
		std::string indexPath = fullPath + "/" + loc.getIndex();
		if (isFile(indexPath)) {
			res.status_code = 200;
			res.status_text = "OK";
			res.body = readFileContent(indexPath);
		} else {
			res.status_code = 404;
			res.status_text = "Not Found";
			try {
				res.body = readFileContent("www/weberrors/404.html");
			} catch (...) {
				res.body = "<h1>404 Not Found</h1>";
			}
		}
	}

	std::ostringstream len;
	len << res.body.length();
	res.headers["Content-Type"] = "text/html";
	res.headers["Content-Length"] = len.str();
	return res;
}

static bool extracFileContenandName(const std::string &body, const std::string &boundary, std::string &out_filename, std::string &out_content)
{
	std::string delimiter = "--" + boundary;
	std::cout << body << std::endl;
	size_t start = body.find("Content-Disposition");
	if (start == std::string::npos)
	{
		std::cout << "Body start" << std::endl;
		return false;
	}
	size_t filename_pos = body.find("filename=\"", start);
	if (filename_pos == std::string::npos)
		return false;
	filename_pos += 10;
	size_t filename_end = body.find("\"", filename_pos);
	if (filename_end == std::string::npos)
		return false;
	out_filename = body.substr(filename_pos, filename_end - filename_pos);
	size_t content_start = body.find("\r\n\r\n", filename_end);
	if (content_start == std::string::npos)
		return false;
	content_start += 4;
	size_t content_end = body.find(delimiter, content_start);
	if (content_end == std::string::npos)
		return false;
	if (content_end >= 2)
		out_content = body.substr(content_start, content_end - content_start - 2);
	else
		out_content = body.substr(content_start, content_end - content_start);
	return true;
}

HttpResponse HttpHandler::handlePOST(const HttpRequest& req, const Location& loc) {
	HttpResponse res;
	res.version = "HTTP/1.1";
	std::cout << req.body << std::endl;
	std::map<std::string, std::string>::const_iterator it = req.headers.find("Content-Type");
	if (it == req.headers.end())
	{
		res.status_code = 400;
		res.status_text = "Bad Request";
		res.body = "Missing Content-Type header";
		std::ostringstream len;
		len << res.body.length();
		res.headers["Content-Length"] = len.str();
		res.headers["Content-Type"] = "text/plain";
		return res;
	}
	std::string contentType = it->second;
	size_t pos = contentType.find("boundary=");
	if (pos == std::string::npos)
	{
		res.status_code = 400;
		res.status_text = "Bad Request";
		res.body = "Missing boundary";
		std::ostringstream len;
		len << res.body.length();
		res.headers["Content-Length"] = len.str();
		res.headers["Content-Type"] = "text/plain";
		return res;
	}
	
	std::string boundary = contentType.substr(pos + 9);
	std::string fileName;
	std::string content;
	if (!extracFileContenandName(req.body, boundary, fileName, content))
	{
		res.status_code = 400;
		res.status_text = "Bad Request";
		res.body = "Could not parse multipart content";
		std::ostringstream len;
		len << res.body.length();
		res.headers["Content-Length"] = len.str();
		res.headers["Content-Type"] = "text/plain";
		return res;
	}

	std::string uploadDir = loc.getRoot() + "/uploads";
	mkdir(uploadDir.c_str(), 0777);

	std::string filePath = uploadDir + "/" + fileName;
	std::ofstream file(filePath.c_str());
	if (!file.is_open())
	{
		res.status_code = 500;
		res.status_text = "Internal Server Error";
		std::ifstream file("./www/weberrors/500.html");
		if (!file.is_open())
			throw std::runtime_error("Could not open error file: 500.html");
		std::stringstream buffer;
		buffer << file.rdbuf();
		res.body = buffer.str();
		res.headers["Content-Type"] = "text/html";
		std::ostringstream len;
		len << res.body.length();
		res.headers["Content-Length"] = len.str();
	}
	else
	{
		file << content;
		file.close();
		res.status_code = 201;
		res.status_text = "Created";
		res.body = "File uploaded as " + fileName;
	}

	std::ostringstream len;
	len << res.body.length();
	res.headers["Content-Type"] = "text/plain";
	res.headers["Content-Length"] = len.str();
	return res;
}

HttpResponse HttpHandler::handleDELETE(const HttpRequest& req, const Location& loc) {
	HttpResponse res;
	res.version = "HTTP/1.1";

	std::string path = loc.getRoot() + req.uri;
	if (isFile(path)) {
		if (remove(path.c_str()) == 0) {
			res.status_code = 200;
			res.status_text = "OK";
			res.body = "Deleted: " + req.uri;
		} else {
			res.status_code = 403;
			res.status_text = "Forbidden";
			res.body = "Cannot delete: " + req.uri;
		}
	} else {
		res.status_code = 404;
		res.status_text = "Not Found";
		res.body = "File not found: " + req.uri;
	}

	std::ostringstream len;
	len << res.body.length();
	res.headers["Content-Type"] = "text/plain";
	res.headers["Content-Length"] = len.str();
	return res;
}

HttpResponse HttpHandler::handleRequest(const HttpRequest& req, const std::vector<Server>& servers) {
    HttpResponse res; // usado para capturar Set-Cookie desde matchCookie
    try {
        Server server = matchServer(req, servers);
        Location loc = matchLocation(req.uri, server);
        std::vector<std::string> allowed = loc.getMethods();

		std::cout << "PATH\n" << std::endl;
		std::cout << loc.getPath() << std::endl;
        if (std::find(allowed.begin(), allowed.end(), req.method) == allowed.end()) {
            res.version = "HTTP/1.1";
            res.status_code = 405;
            res.status_text = "Method Not Allowed";
            res.body = "<h1>405 - Method Not Allowed</h1>";
            res.headers["Content-Type"] = "text/plain";
            std::ostringstream len;
            len << res.body.length();
            res.headers["Content-Length"] = len.str();
            return res;
        }

        // --- Manejo de cookies y sesión
        std::string sessionId = matchCookie(req, res);
        std::map<std::string, std::string>& session = sessionManager.getSession(sessionId);
        handleVisits(session, res);

        // --- Construir respuesta en una nueva variable
        HttpResponse response;
        if (req.method == "GET")
            response = handleGET(req, loc);
        else if (req.method == "POST")
            response = handlePOST(req, loc);
        else if (req.method == "DELETE")
            response = handleDELETE(req, loc);
        else {
            response.version = "HTTP/1.1";
            response.status_code = 501;
            response.status_text = "Not Implemented";
            response.body = "Not Implemented";
            std::ostringstream len;
            len << response.body.length();
            response.headers["Content-Type"] = "text/plain";
            response.headers["Content-Length"] = len.str();
        }

        // --- Preservar Set-Cookie si fue seteada
        if (res.headers.find("Set-Cookie") != res.headers.end()) {
            response.headers["Set-Cookie"] = res.headers["Set-Cookie"];
        }

        return response;
    } catch (const std::exception& e) {
        res.version = "HTTP/1.1";
        res.status_code = 404;
        res.status_text = "Not Found";
        res.body = "<h1>404 Not Found</h1>";
        std::ostringstream len;
        len << res.body.length();
        res.headers["Content-Type"] = "text/html";
        res.headers["Content-Length"] = len.str();
        return res;
    }
}
