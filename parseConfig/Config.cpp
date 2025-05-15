/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jolivare <jolivare@student.42mad.com>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/03 10:03:22 by jolivare          #+#    #+#             */
/*   Updated: 2025/05/15 11:15:23 by jolivare         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"

Config::Config(std::string const &defaultPath): server_num(0)
{
	std::string content = getConfigFile(defaultPath);
	if (content.empty())
		throw std::runtime_error("Empty file");
	removeComments(content);
	saveConfigs(content);
	if (this->server_configs.size() == 0 || this->server_num == 0)
		throw std::runtime_error("No servers in file: " + defaultPath);
}

Config::Config(Config const &copy)
{
	this->server_num = copy.server_num;
	for (std::vector<std::string>::const_iterator it = copy.server_configs.begin(); it != copy.server_configs.end(); it++)
		this->server_configs.push_back(*it);
}

Config::~Config()
{}

std::string Config::getConfigFile(std::string const &defaultPath)
{
	if (defaultPath.empty() || defaultPath.length() == 0)
		throw std::runtime_error("File does not exist");
	std::ifstream configFd(defaultPath.c_str());
	if (!configFd || !configFd.is_open())
		throw std::runtime_error("File does not exist");
	std::string content, line;
	while(std::getline(configFd, line))
		content += line + "\n";
	return content;
}

void Config::removeComments(std::string &content)
{
	size_t pos = content.find('#');
	while (pos != std::string::npos)
	{
		size_t finalPos = content.find('\n', pos);
		content.erase(pos, finalPos - pos);
		pos = content.find('#');
	}
}

void Config::saveConfigs(std::string &configContent)
{
	size_t start;

	for (start = 0; configContent[start]; start++)
	{
		if (!isspace)
		{
			configContent = configContent.substr(start);
			break;
		}
	}
	if (configContent.find("server", 0) == std::string::npos)
		throw std::runtime_error("No server config defined in file");
	start = 0;
	size_t end = 1;
	while (start != end && start < configContent.length())
	{
		start = findStartPos(start, configContent);
		if (start == std::string::npos)
			break;
		end = findEndPos(start, configContent);
		if (end == std::string::npos)
			throw std::runtime_error("Problem with scope");
		this->server_configs.push_back(configContent.substr(start, end - start + 1));
		this->server_num++;
		start = end + 1;
	}
}

size_t Config::findStartPos(size_t start, std::string &configContent)
{
	size_t i;
	
	for (i = start; configContent[i]; i++)
	{
		if (configContent[i] == 's')
			break ;
		if (!isspace(configContent[i]))
			throw std::runtime_error("Wrong character out of server scope{}");
	}
	if (!configContent[i])
		return std::string::npos;
	if (configContent.compare(i, 6, "server") != 0)
		throw std::runtime_error("Wrong character out of server scope{}");
	i += 6;
	while (configContent[i] && isspace(configContent[i]))
		i++;
	if (configContent[i] == '{')
		return i;
	else
		throw std::runtime_error("Wrong character out of server scope{}");
}

size_t Config::findEndPos(size_t start, std::string &configContent)
{
	size_t i;
	size_t scope;
	
	scope = 0;
	for (i = start + 1; configContent[i]; i++)
	{
		if (configContent[i] == '{')
			scope++;
		if (configContent[i] == '}')
		{
			if (!scope)
				return i;
			scope--;
		}
	}
	return std::string::npos;
}

void Config::removeFirstAndLastLine(std::string &str)
{
	size_t pos = str.find('\n');
	if (pos == std::string::npos)
		throw std::runtime_error("Wrong server configuration");
	str = str.substr(pos + 1);
	pos = str.rfind('\n');
	if (pos == std::string::npos)
		throw std::runtime_error("Wrong server configuration");
	str = str.substr(0, pos);
}

void Config::removeInitialSpaces(std::string &str)
{
	for (size_t i = 0; i < str.length(); i++)
	{
		size_t iniPos = i;
		while (isspace(str[i]))
			i++;
		str.erase(iniPos, i - iniPos);
		i = iniPos;
		while (str[i] && str[i] != '\n')
			i++;
	}
}

std::vector<std::string> Config::getContentVector(std::string &config)
{
	std::vector<std::string> content;
	for (size_t i = 0; i < config.length(); i++)
	{
		if (config.substr(i, 8) == "location")
		{
			
			size_t finalPos = config.find('}', i);
			if (finalPos != std::string::npos)
			{
				size_t auxPos = config.find('\n', finalPos);
				if (auxPos == std::string::npos)
					auxPos = config.size() - 1;
				while (isspace(config[auxPos]))
					auxPos--;
				if (auxPos != finalPos)
					throw std::runtime_error("Location error: problem after scope {}");
				content.push_back(config.substr(i, finalPos - i + 1));
			}
			else
				throw std::runtime_error("Location error: problem with scope {}");
			i = finalPos;
		}
		else
		{
			size_t finalPos = config.find('\n', i);
			if (finalPos != std::string::npos)
			{
				while (isspace(config[finalPos - 1]))
					finalPos--;
				content.push_back(config.substr(i, finalPos - 1));
			}
			else
			{
				finalPos = config.length();
				while (isspace(config[finalPos - 1]))
					finalPos--;
				content.push_back(config.substr(i, finalPos - i));
			}
		}
		while (config[i] != '\n')
			i++;
	}
	return content;
}

std::vector<std::string> Config::split_spaces(std::string &str)
{
	std::vector<std::string> res;
	std::istringstream iss(str);
	std::string word;
	
	while (iss >> word)
		res.push_back(word);
	return res;
}

std::string Config::getNamePath(std::string &locations)
{
	size_t i = 0;
	while (locations[i] != ' ')
		i++;
	while (locations[i] == ' ')
		i++;
	size_t finalPos = i;
	while (locations[finalPos] != ' ' &&  locations[finalPos] != '\n' && locations[finalPos] != '{')
		finalPos++;
	return locations.substr(i, finalPos - i);
}

std::vector<std::string> Config::getArgLocations(std::string &locations)
{
	size_t i = locations.find(' ');
	while (locations[i] && isspace(locations[i]))
		i++;
	while (locations[i] && !isspace(locations[i]) && locations[i] != '{')
		i++;
	while (locations[i] && isspace(locations[i]))
		i++;
	if (locations[i] == '{')
		throw std::runtime_error("Wrong location config");
	i++;
	while (isspace(locations[i]))
		i++;
	std::vector<std::string> args;
	for (size_t z = i; locations[z]; z++)
	{
		while (locations[z] != '\n')
		{
			if (locations[z] == '}')
				break;
			z++;
		}
		if (locations[z] == '}')
			break;
		if (z == std::string::npos)
		{
			args.push_back(locations.substr(i));
			break ;
		}
		args.push_back(locations.substr(i, z - i));
		i = z + 1;
	}
	return args;
}

void Config::vectorToServer(std::vector<std::string> &content, Server &server)
{
	std::vector<std::string>::iterator it;
	for (it = content.begin(); it != content.end(); it++)
	{
		std::string param = *it;
		if (param.substr(0, 6) == "listen" && param[6] == ' ')
		{
			std::string auxPort = param.substr(7);
			removeInitialSpaces(auxPort);
			server.setPort(auxPort);
		}
		else if (param.substr(0, 11) == "server_name" && param[11] == ' ')
		{
			std::string auxName = param.substr(12);
			removeInitialSpaces(auxName);
			server.setServerName(auxName);
		}
		else if (param.substr(0, 4) == "host" && param[4] == ' ')
		{
			std::string auxHost = param.substr(5);
			removeInitialSpaces(auxHost);
			server.setHost(auxHost);
		}
		else if (param.substr(0, 4) == "root" && param[4] == ' ')
		{
			std::string auxRoot = param.substr(5);
			removeInitialSpaces(auxRoot);
			server.setRoot(auxRoot);
		}
		else if (param.substr(0, 20) == "client_max_body_size" && param[20] == ' ')
		{
			std::string auxSize = param.substr(21);
			removeInitialSpaces(auxSize);
			server.setClientMaxBodySize(auxSize);
		}
		else if (param.substr(0, 5) == "index" && param[5] == ' ')
		{
			std::string auxIndex = param.substr(6);
			removeInitialSpaces(auxIndex);
			server.setIndex(auxIndex);
		}
		else if (param.substr(0, 9) == "autoindex" && param[9] == ' ')
		{
			std::string auxAutoI = param.substr(10);
			removeInitialSpaces(auxAutoI);
			server.setAutoIndex(auxAutoI);
		}
		else if (param.substr(0, 10) == "error_page" && param[10] == ' ')
		{
			std::string auxErrorPages = param.substr(11);
			removeInitialSpaces(auxErrorPages);
			std::vector<std::string> auxErrors = split_spaces(auxErrorPages); 
			server.setErrorPages(auxErrors);
		}
		else if (param.substr(0, 8) == "location" && param[8] == ' ')
		{
			std::string auxLocation = param.substr(9);
			std::vector<std::string> locations = getArgLocations(param);
			server.setLocation(auxLocation, locations);
		}
		else
			throw std::runtime_error("Undefined parameter: " + param);
	}
	if (server.getRoot().empty())
	{
		char *cwd = getcwd(NULL, 0);
		std::string auxRoot(cwd);
		free (cwd);
		server.setRoot((auxRoot + ";").c_str());
	}
	if (server.getName().empty())
		server.setServerName("localhost;");
	if (server.getPort() == 0)
		server.setPort("80;");
	server.setWebErrors();
}

void Config::saveServers()
{
	std::vector<std::string>::iterator it;
	std::vector<short> allPorts;
	for (it = this->server_configs.begin(); it != this->server_configs.end(); it++)
	{
		std::string servConf = *it;
		removeFirstAndLastLine(servConf);
		removeInitialSpaces(servConf);
		std::vector<std::string> content = getContentVector(servConf);
		
		Server newServ;
		vectorToServer(content, newServ);
		bool found_root = false;
		std::vector<Location>::const_iterator it;
		allPorts.push_back(newServ.getPort());
		for (it = newServ.getLocations().begin(); it != newServ.getLocations().end(); it++)
		{
			if (it->getPath() == "/")
				found_root = true;
		}
		if (found_root == false)
			throw std::runtime_error("Could not find location");
	}
	std::vector<short>::const_iterator it2;
	for (it2 = allPorts.begin(); it2 != allPorts.end(); it2++)
	{
		std::vector<short>::const_iterator it3;
		for (it3 = it2 + 1; it2 != allPorts.end(); it2++)
		{
			if (*it3 == *it2)
				throw std::runtime_error("There can not be the same two ports in config file");
		}
	}
}

void Config::printAllConfigs()
{
	for(std::vector<std::string>::iterator it = this->server_configs.begin(); it != server_configs.end(); ++it)
	{
		removeFirstAndLastLine(*it);
		removeInitialSpaces(*it);
		std::cout << *it << std::endl;
	}
}

Server *Config::getServer(int i)
{
	if (i < this->server_num)
		return &(this->servers[i]);
	return NULL;
}

int Config::getServerNum()
{
	return this->server_num;
}
