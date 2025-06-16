/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jolivare <jolivare@student.42mad.com>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/15 16:51:34 by jolivare          #+#    #+#             */
/*   Updated: 2025/06/16 17:50:30 by jolivare         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Server.hpp"

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
	if (location.getCgiExtension().empty())
		throw std::runtime_error("Empty CGI config");
	else if (location.getCgiPath().empty())
		throw std::runtime_error("Empty cgi path");
	if (access(location.getIndex().c_str(), R_OK) < 0)
	{
		std::string auxIndex = location.getRoot() + location.getPath() + "/" + location.getIndex();
		if (getPathType(location.getIndex()) != -1)
		{
			std::string rootPath = getCurrentWorkingDir();
			location.setRoot(rootPath);
			auxIndex = rootPath + location.getPath()  + location.getIndex();
		}
		if (auxIndex.empty() || getPathType(auxIndex) == -1 || access(auxIndex.c_str(), R_OK) < 0)
			throw std::runtime_error("Wrong cgi configuration: " + auxIndex);
		// validateCgiPathsAndExtensions(location);
	}

	std::vector<std::string>::const_iterator it;
	std::vector<std::string>::const_iterator itPath;
	for (it = location.getCgiPath().begin(); it != location.getCgiPath().end(); ++it)
	{
		// removeSemicolon(*it);
		if (getPathType(*it) != 1)
			throw std::runtime_error("Invalid CGI interpreter path: " + *it);
		if (access(it->c_str(), X_OK) < 0)
			throw std::runtime_error("CGI interpreter not executable: " + *it);
	}

	for (it = location.getCgiExtension().begin(); it != location.getCgiExtension().end(); ++it)
	{
		if (*it != ".py" && *it != ".sh")
			throw std::runtime_error("Unsupported CGI extension: " + *it);
		bool hasValidInterpreter = false;
		for (itPath = location.getCgiPath().begin(); itPath != location.getCgiPath().end(); ++itPath)
		{	
			if ((*it == ".py" && (*itPath).find("python") != std::string::npos) ||
			(*it == ".sh" && (*itPath).find("bash") != std::string::npos))
			{
				location.setCgiPairs(*it, *itPath);
				hasValidInterpreter = true;
				break;
			}	
		}
		if (!hasValidInterpreter)
			throw std::runtime_error("No valid interpreter found for extension: " + *it);
	}
	if (location.getCgiPath().size() != location.getCgiExtension().size())
		throw std::runtime_error("Mismatch between CGI extensions and interpreters");
}

void Server::tryStandardLocation(Location &location)
{
	if (location.getPath()[0] != '/')
		throw std::runtime_error("Error validating location path");
		
	if (location.getRoot().empty())
	{
		if (location.getPath() == "/cgi-bin")
			location.setRoot(".");
		else if (location.getPath() == "/")
			location.setRoot(".");
		else
			location.setRoot(this->_root);
	}
		
	if (fileExistAndReadable(location.getRoot() + location.getPath() + "/", location.getIndex()))
		return;
		
	if (!location.getReturnPath().empty())
		return;

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
		
	// Verifica que no haya nada después del punto y coma excepto espacios
	for (size_t i = pos + 1; i < token.size(); i++)
	{
		if (!std::isspace(token[i]))
			throw std::runtime_error("Wrong token (characters after semicolon): " + token);
	}
}

void Server::setWebErrors()
{
	std::string currentDir = getCurrentWorkingDir();
	std::string errorPath = currentDir + "/www/weberrors/";

	if (this->_web_errors[403].empty())
	{
		std::string path403 = errorPath + "403.html";
		if (access(path403.c_str(), R_OK) == -1)
			throw std::runtime_error("Could not access default error 403 page");
		this->_web_errors[403] = path403;
	}
	if (this->_web_errors[404].empty())
	{
		std::string path404 = errorPath + "404.html";
		if (access(path404.c_str(), R_OK) == -1)
			throw std::runtime_error("Could not access default error 404 page");
		this->_web_errors[404] = path404;
	}
	if (this->_web_errors[500].empty())
	{
		std::string path500 = errorPath + "500.html";
		if (access(path500.c_str(), R_OK) == -1)
			throw std::runtime_error("Could not access default error 500 page");
		this->_web_errors[500] = path500;
	}
}

void Server::setServerName(std::string name)
{
	if (!this->_name.empty())
		return ;
	this->_name = name;
}

void Server::setHost(std::string hostname)
{
    std::string cleanHostname = hostname.substr(0, hostname.find(';'));
    
    if (cleanHostname == "localhost")
	{
		std::cout << "Entra en el if" << std::endl;
        cleanHostname = "127.0.0.1";
	}
        
    if (!validHost(cleanHostname))
    {
        if (cleanHostname.find_first_not_of("0123456789.") != std::string::npos)
            throw std::runtime_error("Invalid host (contains non-IP characters): " + cleanHostname);
            
        // Verificar formato de dirección IPv4 (4 números separados por puntos)
        int parts = 0;
        std::string part;
        std::istringstream ss(cleanHostname);
        while (std::getline(ss, part, '.'))
        {
            parts++;
            try {
                int num = std::atoi(part.c_str());
                if (num < 0 || num > 255)
                    throw std::runtime_error("Invalid host (octet out of range): " + cleanHostname);
            }
            catch (const std::exception&) {
                throw std::runtime_error("Invalid host (non-numeric octet): " + cleanHostname);
            }
        }
        
        if (parts != 4)
            throw std::runtime_error("Invalid host (wrong number of octets): " + cleanHostname);
            
        // Si llegamos aquí, es un error genérico
        throw std::runtime_error("Invalid host: " + cleanHostname);
    }
    
    this->_host = inet_addr(cleanHostname.data());
}

void Server::setRoot(std::string root)
{
    // validEOL(root);  // Verifica que termine con punto y coma
    
    // Extraer la parte útil antes del punto y coma
    std::string cleanRoot = root.substr(0, root.find(';'));
    
    if (getPathType(cleanRoot) == DIR_TYPE)
        this->_root = cleanRoot;  // Guarda la versión sin punto y coma
    else
    {
        std::string dir = getCurrentWorkingDir();
        
        // Si es una ruta relativa que empieza con ./
        if (cleanRoot.substr(0, 2) == "./")
            cleanRoot = cleanRoot.substr(2);  // Quitar el "./"
            
        std::string finalroot = dir + "/" + cleanRoot;
        
        std::cout << "Checking path: " << finalroot << std::endl;
        
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
	// validEOL(port);
	
	try {
		int finalPort = std::atoi(port.c_str());
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
	// validEOL(bodySize);
	
	try {
		long finalSize = std::atol(bodySize.c_str());
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
		// validEOL(path);
		
		// Verificar que no sea un directorio
		if (getPathType(path) == DIR_TYPE)
			throw std::runtime_error("Invalid page error path (is a directory): " + path);
		
		// Asegurar que hay un root path válido
		if ((this->_root).empty())
		{
			std::string currentDir = getCurrentWorkingDir();
			this->setRoot(currentDir + ";");
		}
		
		// Validar que el archivo existe y es accesible
		std::string fullPath = this->_root + path;
		if (path[0] == '/')
			fullPath = this->_root + path;
		else
			fullPath = this->_root + "/" + path;
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

void Server::setIndex(std::string &index)
{
	// validEOL(index);
	this->_index = index;
}

void Server::setAutoIndex(std::string &autoI)
{
	// validEOL(autoI);
	if (autoI == "on;")
		this->_autoindex = true;
	else if (autoI == "off;")
		this->_autoindex = false;
	else
		throw std::runtime_error("Invalid autoindex value: " + autoI);	
}

bool Server::paramsLeft(std::string str, size_t pos)
{
	for (size_t i = pos; i < str.length(); i++)
	{
		if (!isspace(str[i]))
			return true;
	}
	return false;
}

std::string Server::returnParams(std::string str, size_t pos)
{
	if (!isspace(str[pos]))
		throw std::runtime_error("Invalid syntax in location: " + str);
	for (size_t i = pos; i < str.length(); i++)
	{
		if (!isspace(str[i]))
			return str.substr(i);
	}
	throw std::runtime_error("Invalid syntax in location: " + str);
}

std::vector<std::string> Server::splitSpaces(std::string &methods)
{
	std::vector<std::string> finalVector;
	size_t start = 0;
	size_t end;
	
	while ((end = methods.find(' ', start)) != std::string::npos)
	{
		if (end != start)
			finalVector.push_back(methods.substr(start, end - start));
		start = end + 1;
	}
	if (start < methods.size())
		finalVector.push_back(methods.substr(start));
	return finalVector;
}

void Server::parseExtensions(std::vector<std::string> &extensions, std::string &auxExt)
{
	for (size_t i = 0; i < auxExt.length(); i++)
	{
		if (!isspace(auxExt[i]))
		{
			size_t j;
			for (j = 0; auxExt[i] && !isspace(auxExt[i]); j++)
				i++;
			extensions.push_back(auxExt.substr(i - j, j));
		}
	}
}

void Server::parseCgiPath(std::vector<std::string> &paths, std::string &auxPath)
{
	for (size_t i = 0; i < auxPath.length(); i++)
	{
		if (!isspace(auxPath[i]))
		{
			size_t j;
			for (j = 0; auxPath[i] && !isspace(auxPath[i]); j++)
				i++;
			if (auxPath.substr(i - j, j).find("/python") == std::string::npos && auxPath.substr(i - j, j).find("/bash") == std::string::npos)
				throw std::runtime_error("Invalid cgi path in location: " + auxPath.substr(i - j, j));
			paths.push_back(auxPath.substr(i - j, j));
		}
	}
}

void Server::addLocation(const Location& location) {
    this->_locations.push_back(location);
}


static void removeSemicolon(std::string &src)
{
	size_t semicolonPos = src.find(';');
	if (semicolonPos != std::string::npos)
		src = src.substr(0, semicolonPos);
}
void Server::setLocation(std::string name, std::vector<std::string> &src)
{
	Location location;
	bool saved_methods = false;
	bool maxBodySize = false;

	location.setPath(name);
	for (size_t i = 0; i < src.size(); ++i)
	{
		if (src[i].substr(0, 4) == "root" && paramsLeft(src[i], 4))
		{
			if (!location.getRoot().empty())
				throw std::runtime_error("Duped root in Location: " + src[i]);
			std::string auxRoot = returnParams(src[i], 4);
			validEOL(auxRoot);
			removeSemicolon(auxRoot);
			std::cout << "AUX ROOT: " << auxRoot << std::endl;
			
			if (name == "/cgi-bin")
				location.setRoot(".");
			else if (auxRoot == "." || auxRoot == "./")
				location.setRoot(auxRoot);
			else if (getPathType(auxRoot) == 2)
				location.setRoot(auxRoot);
			else if (auxRoot[0] == '/')
				location.setRoot(auxRoot);
			else
				location.setRoot(auxRoot);
		}
		else if (src[i].substr(0, 13) == "allow_methods" && paramsLeft(src[i], 13))
		{
			if (saved_methods)
				throw std::runtime_error("Duped allow_methods location: "+ src[i]);
			std::string auxMethods = returnParams(src[i], 13);
			validEOL(auxMethods);
			removeSemicolon(auxMethods);
			std::vector<std::string> methods = splitSpaces(auxMethods);
			location.setMethods(methods);
			saved_methods = true;
		}
		else if (src[i].substr(0, 9) == "autoindex" && paramsLeft(src[i], 9))
		{
			std::string auxAutoI = returnParams(src[i], 9);
			validEOL(auxAutoI);
			removeSemicolon(auxAutoI);
			std::cout << "AutoI: " << auxAutoI << std::endl;
			location.setAutoIndex(auxAutoI);
		}
		else if (src[i].substr(0, 5) == "index" && paramsLeft(src[i], 5))
		{
			std::string auxIndex = returnParams(src[i], 5);
			validEOL(auxIndex);
			removeSemicolon(auxIndex);
			location.setIndex(auxIndex);
			std::cout << "index: " << location.getIndex() << std::endl;
		}
		else if (src[i].substr(0, 6) == "return" && paramsLeft(src[i], 6))
		{
			std::string auxReturn = returnParams(src[i], 6);
			validEOL(auxReturn);
			removeSemicolon(auxReturn);
			if (auxReturn == "/cgi-bin")
				throw std::runtime_error("Return parameter not allowed");
			if (!location.getReturnPath().empty())
				throw std::runtime_error("Duped return parameter");
			location.setReturnPath(auxReturn);
		}
		else if (src[i].substr(0, 5) == "alias" && paramsLeft(src[i], 5))
		{
			std::string auxAlias = returnParams(src[i], 5);
			validEOL(auxAlias);
			removeSemicolon(auxAlias);
			if (auxAlias == "/cgi-bin")
				throw std::runtime_error ("Alias parameter not allowed");
			if (!location.getAlias().empty())
				throw std::runtime_error("Duped alias parameter");
			location.setAlias(auxAlias);
		}
		else if (src[i].substr(0, 7) == "cgi_ext" && paramsLeft(src[i], 7))
		{
			std::string auxExt = returnParams(src[i], 7);
			validEOL(auxExt);
			removeSemicolon(auxExt);
			std::vector<std::string> extensions;
			parseExtensions(extensions, auxExt);
			location.setCgiExt(extensions);
		}
		else if (src[i].substr(0, 8) == "cgi_path" && paramsLeft(src[i], 8))
		{
			std::string auxPath = returnParams(src[i], 8);
			validEOL(auxPath);
			removeSemicolon(auxPath);
			std::vector<std::string> paths;
			parseCgiPath(paths, auxPath);
			location.setCgiPath(paths);
		}
		else if (src[i].substr(0, 20) == "client_max_body_size" && paramsLeft(src[i], 20))
		{
			if (maxBodySize)
				throw std::runtime_error("Duped client max body size parameter");
			std::string auxSize = returnParams(src[i], 20);
			validEOL(auxSize);
			removeSemicolon(auxSize);
			for (size_t i = 0; i < auxSize.length(); i++)
			{
				if (!isdigit(auxSize[i]))
					throw std::runtime_error("Illegal character in client max body size: " + auxSize[i]);
			}
			long size = atol(auxSize.c_str());
			if (size < 1)
				throw std::runtime_error("Body size out of bounds");
			location.setClientBodySize(size);
			maxBodySize = true;
		}
		else
			throw std::runtime_error("Illegan parameter in location: " + src[i]);
	}
	if (!maxBodySize)
		location.setClientBodySize(this->client_max_body_size);
	tryLocation(location);
	this->_locations.push_back(location);
}

const std::string &Server::getName()
{
	return this->_name;
}

const uint16_t &Server::getPort()
{
	return this->_port;
}

const in_addr_t &Server::getHost()
{
	return this->_host;
}
const std::string &Server::getIndex()
{
	return this->_index;
}

const std::string &Server::getRoot()
{
	return this->_root;
}

const unsigned long &Server::getClientMaxBodySize()
{
	return this->client_max_body_size;
}

const bool &Server::getAutoIndex()
{
	return this->_autoindex;
}

const std::map<unsigned int, std::string> &Server::getWebErrors()
{
	return this->_web_errors;
}

const std::string &Server::getWebErrorPath(int code)
{
	std::map<unsigned int, std::string>::const_iterator it = this->_web_errors.find(code);
	if (it == this->_web_errors.end())
	{
		std::stringstream str;
		str << code;
		throw std::runtime_error("Invalid web error page code: " + str.str());
	}
	return it->second;
}

const std::vector<Location> &Server::getLocations() const {
    return this->_locations;
}

int Server::getListenFd() const
{
	return this->listen_fd;
}

const pollfd Server::getPollFd()
{
	return this->_pollfd;
}

bool Server::emptyWeberrors()
{
	std::map<unsigned int, std::string>::const_iterator it;
	for (it = this->_web_errors.begin(); it != this->_web_errors.end(); it++)
	{
		if (!(it->second).empty())
			return false;
	}
	return true;
}

void Server::startServer()
{
	if ((this->listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		std::cerr << "Server error: Socket creation error" << std::endl;
		exit(EXIT_FAILURE);
	}
	int opt = 1;
	setsockopt(this->listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int));

	memset(&this->server_address, 0, sizeof(this->server_address));
	this->server_address.sin_family = AF_INET;
	this->server_address.sin_addr.s_addr = this->_host;
	this->server_address.sin_port = htons(this->_port);
	if (bind(this->listen_fd, (struct sockaddr *) &this->server_address, sizeof(this->server_address)) == -1)
	{
		std::cerr << "Server error: Socket: bind error" << std::endl;
		exit(EXIT_FAILURE);
	}
}

int Server::createServer()
{
	int server_fd = socket(PF_INET, SOCK_STREAM, 0);
	if (server_fd == -1)
	{
		std::cerr << "Error in socket: " <<  strerror(errno) << std::endl;
		close (server_fd);
		exit (EXIT_FAILURE);
	}
	
	int opt = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
	{
		std::cerr << "Error in set socket: " << strerror(errno) << std::endl;
		close (server_fd);
		exit (EXIT_FAILURE);
	}

	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = _host;
	server_address.sin_port = htons(_port);
	
	if (bind(server_fd, (struct sockaddr *)&server_address, sizeof(server_address)) == -1)
	{
		std::cerr << "Error in socket: " << strerror(errno) << std::endl;
		close (server_fd);
		return -1;
	}
	
	this->_pollfd.fd = server_fd;
	this->_pollfd.events = POLLIN;
	std::cout << "Server listening in port " << _port << "..." << std::endl;
	return server_fd;
}
