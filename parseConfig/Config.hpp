/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jolivare <jolivare@student.42mad.com>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/01 10:47:16 by jolivare          #+#    #+#             */
/*   Updated: 2025/05/03 15:11:50 by jolivare         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include "../inc/WebServ.hpp"
#include "Server.hpp"

class Server;

class Config
{
	private:
		int server_num;
		std::vector<std::string> server_configs;
		std::vector<Server> servers;
	public:
		Config(std::string const &defaultPath);
		Config(Config const &copy);
		~Config();

		std::string getConfigFile(std::string const &defaultPath);
		void	removeComments(std::string &content);
		void	saveConfigs(std::string &configContent);
		size_t	findStartPos(size_t start, std::string &configContent);
		size_t	findEndPos(size_t start, std::string &configContent);

		void	removeFirstAndLastLine(std::string &str);
		void	removeInitialSpaces(std::string &str);
		std::vector<std::string> getContentVector(std::string &config);
		void	vectorToServer(std::vector<std::string> &content, Server &server);
		std::vector<std::string> split_spaces(std::string &str);
		void	saveServers();
		
		std::vector<std::string> getArgLocations(std::string &locations);
		std::string	getNamePath(std::string &locations);
		Server	*getServer(int i);
		int getServerNum();

		void printAllConfigs();
};