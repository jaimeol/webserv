/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jolivare <jolivare@student.42mad.com>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/03 10:03:22 by jolivare          #+#    #+#             */
/*   Updated: 2025/05/03 15:32:00 by jolivare         ###   ########.fr       */
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
