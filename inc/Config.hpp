/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rpisoner <rpisoner@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/01 10:47:16 by jolivare          #+#    #+#             */
/*   Updated: 2025/06/19 14:42:15 by rpisoner         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include "../inc/WebServ.hpp"
#include "Server.hpp"

class Server;

class Config
{
	private:
		size_t server_num;
		std::vector<std::string> server_configs;
		std::vector<Server> servers;
	public:
		Config(std::string const &defaultPath);
		Config(Config const &copy);
		~Config();

		std::string readConfigFile(std::string const &path);
		void	removeComments(std::string &content);
		void	extractServerConfigs(std::string &configContent);
		
		void	parseServers();
		void	parseServerBlock(const std::string &serverBlock, Server &server);
		
		std::vector<std::string> splitByLines(const std::string &content);
		std::vector<std::string> splitDirective(const std::string &line);
		std::pair<std::string, std::string> parseLocationBlock(const std::string &block);
		
		Server *getServer(int i);
		size_t getServerNum() const;
		std::vector<Server> getServers() const;
		
		void printConfig();
};