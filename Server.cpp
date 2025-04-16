/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jolivare <jolivare@student.42mad.com>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/15 16:51:34 by jolivare          #+#    #+#             */
/*   Updated: 2025/04/16 17:14:45 by jolivare         ###   ########.fr       */
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

static bool fileExistAndReadable(std::string const &path, std::string const &index)
{
	if (Server::getPathType(index) == 1 && checkFile(path, 4) == 0)
		return true;
	else if (Server::getPathType(path + index) == 1	&& checkFile(path + index, 4) == 0)
		return true;
	return false;
}
void Server::tryLocation(Location &location)
{
	if (location.getPath() == "/cgi-bin")
	{
		if (location.getCgiExtension().empty() || location.getCgiPath().empty())
			throw std::runtime_error("Wrong cgi configuration");
		if (access(location.getIndex().c_str(), 4) < 0)
		{
			std::string auxIndex = location.getRoot() + "/" + location.getIndex();
			if (getPathType(location.getIndex()) != -1)
			{
				char *cwd = getcwd(NULL, 0);
				std::string rootPath(cwd);
				free(cwd);
				location.setRoot(rootPath);
				auxIndex = rootPath + location.getPath() + "/" + location.getIndex();
			}
			if (auxIndex.empty() || getPathType(auxIndex) == -1 || access(auxIndex.c_str(), 4) < 0)
				throw std::runtime_error ("Wrong cgi configuration");
			std::vector<std::string>::const_iterator it;
			for (it = location.getCgiPath().begin(); it != location.getCgiPath().end(); it++)
			{
				if (getPathType(*it) > 0)
					throw std::runtime_error("Wrong cgi configuration");
			}
			std::vector<std::string>::const_iterator itPath;
			for (it = location.getCgiExtension().begin(); it != location.getCgiExtension().end(); it++)
			{
				if (*it != ".py" && *it != ".sh")
					throw std::runtime_error("Wrong cgi extension");
				for (itPath = location.getCgiPath().begin(); itPath != location.getCgiPath().end(); itPath++)
				{
					if (*it == ".py")
					{
						if ((*itPath).find("python") != std::string::npos)
							location.setCgiPairs(*it, *itPath);
					}
					else if (*it == ".sh")
					{
						if ((*itPath).find("bash") != std::string::npos)
							location.setCgiPairs(*it, *itPath);
					}
				}
			}
			if (location.getCgiPath().size() != location.getCgiExtension().size())
				throw std::runtime_error("Wrong cgi configuration");
		}
	}
	else
	{
		if (location.getPath()[0] != '/')
			throw std::runtime_error("Error validating location path");
		if (location.getRoot().empty())
			location.setRoot(this->_root);
		if (fileExistAndReadable(location.getRoot() + location.getPath() + "/", location.getIndex()))
			return ;
		if (!location.getReturnPath().empty())
		{
			if (fileExistAndReadable(location.getRoot(), location.getReturnPath()))
				throw std::runtime_error("Error validating return location");
		}
		if (!location.getAlias().empty())
		{
			if (fileExistAndReadable(location.getRoot(), location.getAlias()))
				throw std::runtime_error("Error validating alias");
		}
	}
}
