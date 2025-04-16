#pragma once

#include "WebServ.hpp"
#include "Location.hpp"

class Location;

class Server
{
	private:
		uint16_t	_port;
		in_addr_t	_host;
		std::string	_name;
		std::string	_index;
		std::string	_root;
		unsigned long	client_max_body_size;
		bool _autoindex;
		std::map<unsigned int, std::string> _web_errors;
		std::vector<std::string> _locations;
		struct sockaddr_in	server_address;
		int	listen_fd;
		pollfd	_pollfd;
	public:
		Server();
		Server(Server const &copy);
		~Server();

		void initWebErrors();
		static int getPathType(std::string const &path);
		void tryLocation(Location &location);
		bool validHost(std::string hostname) const;
};