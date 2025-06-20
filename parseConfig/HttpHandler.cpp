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

HttpResponse HttpHandler::handlePOST(const HttpRequest& req, const Location& loc) {
    HttpResponse res;
    res.version = "HTTP/1.1";

    std::string uploadDir = loc.getRoot() + "/uploads";
    mkdir(uploadDir.c_str(), 0777);

	std::ostringstream oss;
	oss << "/upload_" << time(0) << ".txt";
	std::string filename = uploadDir + oss.str();    std::ofstream file(filename.c_str());
    if (!file.is_open()) {
        res.status_code = 500;
        res.status_text = "Internal Server Error";
        res.body = "Could not save file";
    } else {
        file << req.body;
        file.close();
        res.status_code = 201;
        res.status_text = "Created";
        res.body = "File uploaded to " + filename;
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
    HttpResponse res;
    try {
		Server server = matchServer(req, servers);
 		Location loc = matchLocation(req.uri, server);
		std::vector<std::string> allowed = loc.getMethods();
        if (std::find(allowed.begin(), allowed.end(), req.method) == allowed.end()) {
			res.version = "HTTP/1.1";
            res.status_code = 405;
            res.status_text = "Method Not Allowed";
			res.body = "<h1>405 - Method Not Allowed</h1>";
            res.headers["Content-Length"] = "19";
            res.headers["Content-Type"] = "text/plain";
            return res;
        }

        if (req.method == "GET")
            return handleGET(req, loc);
        else if (req.method == "POST")
            return handlePOST(req, loc);
        else if (req.method == "DELETE")
            return handleDELETE(req, loc);

        res.version = "HTTP/1.1";
        res.status_code = 501;
        res.status_text = "Not Implemented";
        res.body = "Not Implemented";
        res.headers["Content-Length"] = "15";
        res.headers["Content-Type"] = "text/plain";
        return res;
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
