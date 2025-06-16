/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jolivare <jolivare@student.42mad.com>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/07 11:46:37 by jolivare          #+#    #+#             */
/*   Updated: 2025/06/16 17:24:45 by jolivare         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Config.hpp"

Config::Config(std::string const &configPath): server_num(0)
{
	std::cout << "Reading config file: " << configPath << std::endl;
	std::string content = readConfigFile(configPath);

	std::cout << "Removing comments..." << std::endl;
	removeComments(content);
	
	std::cout << "Extracting server blocks..." << std::endl;
	extractServerConfigs(content);

	if (this->server_configs.empty())
		throw std::runtime_error("No server config found in file");
	std::cout << "Parsing servers" << std::endl;
	parseServers();
	printConfig();
}

Config::Config(Config const &copy)
{
	*this = copy;
}

Config::~Config() {}

std::string Config::readConfigFile(std::string const &path)
{
	if (path.empty())
		throw std::runtime_error("Empty file route");
	std::ifstream file(path.c_str());
	if (!file.is_open())
		throw std::runtime_error("Could not open file: " + path);
	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

void Config::removeComments(std::string &content)
{
	size_t pos = content.find('#');
	while (pos != std::string::npos)
	{
		size_t end = content.find('\n', pos);
		if (end == std::string::npos)
		{
			content.erase(pos);
			break;
		}
		else
			content.erase(pos, end - pos);
		pos = content.find('#', pos);
	}
}

void Config::extractServerConfigs(std::string &configContent)
{
	size_t pos = 0;

	while ((pos = configContent.find("server", pos)) != std::string::npos)
	{
		pos += 6;
		pos = configContent.find('{', pos);
		if (pos == std::string::npos)
			throw std::runtime_error("Incorrect syntaxt: No opening brace");
		int braceCount = 1;
		size_t startPos = pos;
		pos++;
		while (braceCount > 0 && pos < configContent.length())
		{
			if (configContent[pos] == '{')
				braceCount++;
			else if (configContent[pos] == '}')
				braceCount--;
			pos++;
		}
		if (braceCount != 0)
			throw std::runtime_error("Incorrect syntax: Disbalanced braces in server block");

		std::string serverBlock = configContent.substr(startPos - 6, pos - startPos + 6);
		this->server_configs.push_back(serverBlock);
		this->server_num++;
	}
}

void Config::parseServers()
{
	this->servers.clear();
	for (size_t i = 0; i < this->server_configs.size(); i++)
	{
		Server newServ;
		std::cout << "Parsing server block" << std::endl;
		parseServerBlock(this->server_configs[i], newServ);
		std::cout << "Server block parsed" << std::endl;

		bool found_root = false;
		for (std::vector<Location>::const_iterator it = newServ.getLocations().begin(); it < newServ.getLocations().end(); ++it)
		{
			std::cout << "Found location: " << it->getPath() << std::endl;
			if (it -> getPath() == "/")
			{
				found_root = true;
				break;
			}
		}
		if (!found_root)
			std::cout << "WARNING: could not find root location (/) in server: " << i + 1 << std::endl;
		this->servers.push_back(newServ);
		std::cout << "Added server with port " << newServ.getPort() << std::endl;
	}

	std::cout << "Total server configured: " << this->servers.size() << std::endl;
}

void Config::parseServerBlock(const std::string &serverBlock, Server &server)
{
	std::vector<std::string> lines = splitByLines(serverBlock);

	size_t i = 0;
	while (i < lines.size())
	{
		std::string line = lines[i];
		
		if (line.empty() || line.find("server {") != std::string::npos)
		{
			i++;
			continue;
		}

		if (line.find("location") == 0)
		{
			std::string locationBlock = line;
			int braceCount = 0;

			for (size_t j = 0; j < line.length(); j++)
			{
				if (line[j] == '{')
					braceCount++;
				else if (line[j] == '}')
					braceCount--;
			}
			size_t j = i + 1;
			while (braceCount > 0 && j < lines.size())
			{
				locationBlock += '\n' + lines[j];
				
				for (size_t k = 0; k < lines[j].length(); k++)
				{
					if (lines[j][k] == '{')
						braceCount++;
					else if (lines[j][k] == '}')
						braceCount--;
				}
				j++;
			}
			if (braceCount != 0)
				throw std::runtime_error("Incorrect syntax in block location: " + line);
			std::cout << "Parsing location block" << std::endl;
			std::pair<std::string, std::string> locationInfo = parseLocationBlock(locationBlock);
			std::string path = locationInfo.first;
			std::string content = locationInfo.second;

			std::vector<std::string> locationDirectives = splitByLines(content);
			for (size_t k = 0; k < locationDirectives.size(); k++)
			{
				size_t start = locationDirectives[k].find_first_not_of(" \t\r\n");
				size_t end = locationDirectives[k].find_last_not_of(" \t\r\n");
				
				if (start != std::string::npos && end != std::string::npos)
					locationDirectives[k] = locationDirectives[k].substr(start, end - start + 1);
				else
					locationDirectives[k] = "";
			}
			std::vector<std::string> filteredDirectives;
			for (size_t k = 0; k < locationDirectives.size(); k++)
			{
				if (!locationDirectives[k].empty())
					filteredDirectives.push_back(locationDirectives[k]);
			}
			std::cout << "Setting location" << std::endl;
			server.setLocation(path, filteredDirectives);
			std::cout << "Configured location " << path << " with " << filteredDirectives.size() << " directives" << std::endl;
			i = j;
		}
		else
		{
			std::vector<std::string> parts = splitDirective(line);
			if (parts.size() >= 2)
			{
				std::string directive = parts[0];
				std::string value = parts[1];

				if (!value.empty() && value[value.length() - 1] == ';')
					value = value.substr(0, value.length() - 1);

				if (directive == "listen")
					server.setPort(value);
				else if (directive == "host")
					server.setHost(value);
				else if (directive == "server_name")
					server.setServerName(value);
				else if (directive == "root")
					server.setRoot(value);
				else if (directive == "index")
					server.setIndex(value);
				else if (directive == "client_max_body_size")
					server.setClientMaxBodySize(value);
				else if (directive == "autoIndex")
					server.setAutoIndex(value);
				else if (directive == "error_page")
				{
					std::vector<std::string> errorPages;
					for (size_t j = 1; j < parts.size(); j++)
					{
						std::string val = parts[j];
						if (!val.empty() && val[val.length() - 1] == ';')
							val = val.substr(0, val.length() - 1);
						errorPages.push_back(val);
					}
					server.setErrorPages(errorPages);
				}
			}
			i++;
		}
	}
	if (server.getRoot().empty())
	{
		char *cwd = getcwd(NULL, 0);
		std::string currentDir(cwd);
		free(cwd);
		server.setRoot(currentDir);
	}
	if (server.getName().empty())
		server.setServerName("localhost");
	if (server.getPort() == 0)
		server.setPort("80");

	server.setWebErrors();
	std::cout << "Server configured: " << server.getName() << "; " << server.getPort() << std::endl;
}

std::vector<std::string> Config::splitByLines(const std::string &content)
{
	std::vector<std::string> lines;
	std::string line;
	std::istringstream stream(content);

	while(std::getline(stream, line))
	{
		size_t start = line.find_first_not_of(" \t\r\n");
		size_t end = line.find_last_not_of(" \t\r\n");
		if (start != std::string::npos && end != std::string::npos)
			lines.push_back(line.substr(start, end - start + 1));
		else if (!line.empty())
			lines.push_back("");
	}
	return lines;
}

std::vector<std::string> Config::splitDirective(const std::string &line)
{
	std::vector<std::string> parts;
	std::string part;
	std::istringstream stream(line);

	while (stream >> part)
	{
		parts.push_back(part);
	}
	return parts;
}

std::pair<std::string, std::string> Config::parseLocationBlock(const std::string &block)
{
	std::istringstream stream(block);
	std::string word;
	stream >> word;

	if (word != "location")
		throw std::runtime_error("Incorrect syntax in location block: it does not start with 'location'");
	
	stream >> word;
	std::string path = word;
	
	size_t openBrace = block.find('{');
	size_t closeBrace = block.find('}');

	if (openBrace == std::string::npos || closeBrace == std::string::npos || closeBrace <= openBrace)
		throw std::runtime_error("Incorrect syntax in location block: Braces not positioned correctly");
	std::string content = block.substr(openBrace + 1, closeBrace - openBrace - 1);

	return std::make_pair(path, content);
}

Server *Config::getServer(int i)
{
	if (i >= 0 && i < (int)this->servers.size())
		return &(this->servers[i]);
	return NULL;
}

size_t Config::getServerNum() const
{
	return this->server_num;
}

void Config::printConfig()
{
	std::cout << "=== Current Config ===" << std::endl;
	std::cout << "Server number: " << this->server_num << std::endl;

	for (size_t i = 0; i < this->servers.size(); i++)
	{
		std::cout << "Server " << i + 1 << ": "
			<< this->servers[i].getName() << ":"
			<< this->servers[i].getPort() << std::endl;
		
		std::cout << " Locations: ";
		const std::vector<Location> &locs = this->servers[i].getLocations();
		for (size_t j = 0; j < locs.size(); j++)
		{
			std::cout << locs[j].getPath() << " ";
		}
		std::cout << std::endl;
	}
	std::cout << "=======================" << std::endl;
}
