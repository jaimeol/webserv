/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jolivare <jolivare@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/15 16:51:34 by jolivare          #+#    #+#             */
/*   Updated: 2025/04/24 00:54:38 by jolivare         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server()
{
	this->_port = 0;
	this->_host = 0;
	this->_name = "";
	this->_index = "";
	this->_root = "";
	this->client_max_body_size = MAX_REQUEST_LENGTH;
	this->_autoindex = false;
	this->listen_fd = 0;
	this->initWebErrors();
}

Server::Server(Server const &copy)
{
	this->_port = copy._port;
	this->_host = copy._host;
	this->_name = copy._name;
	this->_index = copy._index;
	this->_root = copy._root;
	this->client_max_body_size = copy.client_max_body_size;
	this->_autoindex = copy._autoindex;
	this->listen_fd = copy.listen_fd;
	this->_locations = copy._locations;
	this->server_address = copy.server_address;
	this->_web_errors = copy._web_errors;
}

Server::~Server() {}

void Server::initWebErrors()
{
	this->_web_errors.insert(std::make_pair(301, "Moved Permanently"));
	this->_web_errors.insert(std::make_pair(302, "Found"));
	this->_web_errors.insert(std::make_pair(303, "See Other"));
	this->_web_errors.insert(std::make_pair(304, "Not Modified"));

	this->_web_errors.insert(std::make_pair(400, "Bad Request"));
	this->_web_errors.insert(std::make_pair(401, "Unauthorized"));
	this->_web_errors.insert(std::make_pair(403, "Forbidden"));
	this->_web_errors.insert(std::make_pair(404, "Not Found"));
	this->_web_errors.insert(std::make_pair(405, "Method Not Allowed"));
	this->_web_errors.insert(std::make_pair(408, "Request Timeout"));
	this->_web_errors.insert(std::make_pair(409, "Conflict"));
	this->_web_errors.insert(std::make_pair(413, "Payload Too Large"));
	this->_web_errors.insert(std::make_pair(414, "URI Too Long"));

	this->_web_errors.insert(std::make_pair(500, "Internal Server Error"));
	this->_web_errors.insert(std::make_pair(501, "Not Implemented"));
	this->_web_errors.insert(std::make_pair(502, "Bad Gateway"));
	this->_web_errors.insert(std::make_pair(503, "Service Unavailable"));
	this->_web_errors.insert(std::make_pair(504, "Gateway Timeout"));
	this->_web_errors.insert(std::make_pair(505, "HTTP Version Not Supported"));
}

static int checkFile(std::string const path, int mode)
{
	return access(path.c_str(), mode);
}

int Server::getPathType(std::string const &path)
{
	struct stat buffer;
	int		result;

	result = stat(path.c_str(), &buffer);
	if (result == 0)
	{
		if (S_ISREG(buffer.st_mode))
			return 1;
		else if (S_ISDIR(buffer.st_mode))
			return 2;
		else
			return 3;
	}
	return -1;
}

bool Server::fileExistAndReadable(const std::string &path, const std::string &index)
{
    // Comprobar si el índice es un archivo legible
    if (getPathType(index) == FILE_TYPE && checkFile(path, READ_PERMISSION) == 0)
        return true;
    
    // Comprobar si la combinación de ruta e índice es un archivo legible
    if (getPathType(path + index) == FILE_TYPE && checkFile(path + index, READ_PERMISSION) == 0)
        return true;
        
    return false;
}

std::string Server::getCurrentWorkingDir()
{
    char *cwd = getcwd(NULL, 0);
    if (!cwd)
        throw std::runtime_error("Failed to get current working directory");
    
    std::string result(cwd);
    free(cwd);
    return result;
}

void Server::tryCgiLocation(Location &location)
{
    if (location.getCgiExtension().empty() || location.getCgiPath().empty())
        throw std::runtime_error("Wrong cgi configuration");
    
    if (access(location.getIndex().c_str(), 4) < 0)
    {
        std::string auxIndex = location.getRoot() + "/" + location.getIndex();
        if (getPathType(location.getIndex()) != -1)
        {
            std::string rootPath = getCurrentWorkingDir();
            location.setRoot(rootPath);
            auxIndex = rootPath + location.getPath() + "/" + location.getIndex();
        }
        
        if (auxIndex.empty() || getPathType(auxIndex) == -1 || access(auxIndex.c_str(), 4) < 0)
            throw std::runtime_error("Wrong cgi configuration");
            
        validateCgiPathsAndExtensions(location);
    }
}

void Server::validateCgiPathsAndExtensions(Location &location)
{
    std::vector<std::string>::const_iterator it;
    // Verificar que las rutas CGI existen
    for (it = location.getCgiPath().begin(); it != location.getCgiPath().end(); it++)
    {
        if (getPathType(*it) > 0)
            throw std::runtime_error("Wrong cgi configuration");
    }
    
    // Mapear extensiones con sus intérpretes
    std::vector<std::string>::const_iterator itPath;
    for (it = location.getCgiExtension().begin(); it != location.getCgiExtension().end(); it++)
    {
        if (*it != ".py" && *it != ".sh")
            throw std::runtime_error("Wrong cgi extension");
            
        for (itPath = location.getCgiPath().begin(); itPath != location.getCgiPath().end(); itPath++)
        {
            if (*it == ".py" && (*itPath).find("python") != std::string::npos)
                location.setCgiPairs(*it, *itPath);
            else if (*it == ".sh" && (*itPath).find("bash") != std::string::npos)
                location.setCgiPairs(*it, *itPath);
        }
    }
    
    if (location.getCgiPath().size() != location.getCgiExtension().size())
        throw std::runtime_error("Wrong cgi configuration");
}

void Server::tryStandardLocation(Location &location)
{
    if (location.getPath()[0] != '/')
        throw std::runtime_error("Error validating location path");
        
    if (location.getRoot().empty())
        location.setRoot(this->_root);
        
    if (fileExistAndReadable(location.getRoot() + location.getPath() + "/", location.getIndex()))
        return;
        
    if (!location.getReturnPath().empty() && 
        !fileExistAndReadable(location.getRoot(), location.getReturnPath()))
        throw std::runtime_error("Error validating return location");
        
    if (!location.getAlias().empty() && 
        !fileExistAndReadable(location.getRoot(), location.getAlias()))
        throw std::runtime_error("Error validating alias");
}

void Server::tryLocation(Location &location)
{
    if (location.getPath() == "/cgi-bin")
        tryCgiLocation(location);
    else
        tryStandardLocation(location);
}

bool Server::validHost(const std::string& hostname) const
{
    struct sockaddr_in sockaddr;
    return inet_pton(AF_INET, hostname.c_str(), &(sockaddr.sin_addr)) != 0;
}

void Server::validEOL(std::string const token)
{
	size_t pos = token.find(';');
	if (pos == std::string::npos)
		throw std::runtime_error("Token "+token+" must end with ;");
	for (size_t i = 0; i < token.size(); i++)
	{
		if (!std::isspace(token[i]))
			throw std::runtime_error("Wrong token: " + token);
	}
}

void Server::setWebErrors()
{
    std::string aux404 = getCurrentWorkingDir();

    if (this->_web_errors[403].empty())
    {
        if (access((aux404 + "/weberrors/403.html").c_str(), 4) == -1)
            throw std::runtime_error("Could not access default error 403 page");
        this->_web_errors[403] = (aux404 + "/weberrors/403.html").c_str();
    }
    if (this->_web_errors[404].empty())
    {
        if (access((aux404 + "/weberrors/404.html").c_str(), 4) == -1)
            throw std::runtime_error("Could not access default error 404 page");
        this->_web_errors[404] = (aux404 + "/weberrors/404.html").c_str();
    }
    if (this->_web_errors[500].empty())
    {
        if (access((aux404 + "/weberrors/500.html").c_str(), 4) == -1)
            throw std::runtime_error("Could not access default error 500 page");
        this->_web_errors[500] = (aux404 + "/weberrors/500.html").c_str();
    }
}

void Server::setServerName(std::string name)
{
	if (!this->_name.empty())
		return ;
	validEOL(name);
	this->_name = name;
}

void Server::setHost(std::string hostname)
{
	validEOL(hostname);
	if (hostname == "localhost")
		hostname = "127.0.0.1";
	if (!validHost(hostname))
		throw std::runtime_error("Invalid host: " + hostname);
	this->_host = inet_addr(hostname.data());
}

void Server::setRoot(std::string root)
{
    validEOL(root);

    if (getPathType(root) == DIR_TYPE)
        this->_root = root;
    else
    {
        char *cwd = getcwd(NULL, 0);
        if (!cwd)
            throw std::runtime_error("Failed to get current working directory");
            
        std::string dir(cwd);
        free(cwd);
        
        std::string finalroot = dir + root;
        if (getPathType(finalroot) != DIR_TYPE)
            throw std::runtime_error("Invalid root: " + root);
            
        this->_root = finalroot;
    }
}

void Server::setFd(int fd)
{
    this->listen_fd = fd;
}

void Server::setPort(std::string port)
{
    validEOL(port);
    
    try {
        int finalPort = std::stoi(port);
        if (finalPort < 1 || finalPort > 65535)
            throw std::runtime_error("Port value out of bounds");
        this->_port = static_cast<uint16_t>(finalPort);
    } catch (const std::invalid_argument&) {
        throw std::runtime_error("Invalid port number format");
    } catch (const std::out_of_range&) {
        throw std::runtime_error("Port number out of range");
    }
}

void Server::setClientMaxBodySize(std::string bodySize)
{
    validEOL(bodySize);
    
    try {
        long finalSize = std::stol(bodySize);
        if (finalSize < 1)
            throw std::runtime_error("Max body size value out of bounds");
        this->client_max_body_size = finalSize;
    } catch (const std::invalid_argument&) {
        throw std::runtime_error("Invalid body size format");
    } catch (const std::out_of_range&) {
        throw std::runtime_error("Body size out of range");
    }
}

void Server::setErrorPages(std::vector<std::string> web_errors)
{
    if (web_errors.empty())
        return;
        
    for (size_t i = 0; i < web_errors.size(); i++)
    {
        // Validar código de error (debe ser un número de 3 dígitos)
        const std::string& errorCode = web_errors[i];
        if (errorCode.length() != 3)
            throw std::runtime_error("Invalid page error code: length must be 3 digits");
        
        for (size_t j = 0; j < errorCode.length(); j++)
        {
            if (!isdigit(errorCode[j]))
                throw std::runtime_error("Invalid page error code: contains non-digit character");
        }
        
        int codeError = atoi(errorCode.c_str());
        
        // Validar que hay un path después del código
        i++;
        if (i >= web_errors.size())
            throw std::runtime_error("Missing error page path after error code");
        
        // Validar y preparar el path
        std::string path = web_errors[i];
        validEOL(path);
        
        // Verificar que no sea un directorio
        if (getPathType(path) == DIR_TYPE)
            throw std::runtime_error("Invalid page error path (is a directory): " + path);
        
        // Asegurar que hay un root path válido
        if ((this->_root).empty())
        {
            char *cwd = getcwd(NULL, 0);
            if (!cwd)
                throw std::runtime_error("Failed to get current working directory");
            
            std::string auxRoot(cwd);
            free(cwd);
            this->setRoot((auxRoot + ";").c_str());
        }
        
        // Validar que el archivo existe y es accesible
        std::string fullPath = this->_root + path;
        if (getPathType(fullPath) != FILE_TYPE)
            throw std::runtime_error("Invalid page error path (file not found): " + path);
        
        if (access(fullPath.c_str(), R_OK) == -1)
            throw std::runtime_error("Cannot read file: " + path + " (Permission denied)");
        
        // Guardar la configuración
        std::map<unsigned int, std::string>::iterator it = this->_web_errors.find(codeError);
        if (it != _web_errors.end())
            it->second = fullPath;
        else
            this->_web_errors.insert(std::make_pair(codeError, fullPath));
    }
}

std::string Server::getWebError(int code) const
{
    std::map<unsigned int, std::string>::const_iterator it = this->_web_errors.find(code);
    if (it != this->_web_errors.end())
        return it->second;
    return "";
}

