#include "HttpHandler.hpp"
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

Location HttpHandler::matchLocation(const std::string& uri, const Server& server) {
    const std::vector<Location>& locs = server.getLocations();
    for (size_t i = 0; i < locs.size(); i++) {
        if (uri.find(locs[i].getPath()) == 0)
            return locs[i];
    }
    throw std::runtime_error("No matching location for URI: " + uri);
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

    std::string fullPath = loc.getRoot() + req.uri;

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

HttpResponse HttpHandler::handleRequest(const HttpRequest& req, const Server& server) {
    HttpResponse res;
    try {
        Location loc = matchLocation(req.uri, server);
        std::vector<std::string> allowed = loc.getMethods();

        if (std::find(allowed.begin(), allowed.end(), req.method) == allowed.end()) {
            res.version = "HTTP/1.1";
            res.status_code = 405;
            res.status_text = "Method Not Allowed";
            res.body = "Method Not Allowed";
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
