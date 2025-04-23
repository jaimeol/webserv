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
		void validEOL(std::string const token);
		void setWebErrors();
		void setServerName(std::string name);
		void setHost(std::string hostname);
		void setRoot(std::string root);
		void setFd(int fd);
		void setPort(std::string port);
		void setClientMaxBodySize(std::string bodySize);
		void setErrorPages(std::vector<std::string> web_errors);


		std::string getWebError(int code) const;
		// class ServerError: public std::exception
		// {
		// 	private:
		// 		std::string _message;
		// 	public:
		// 		ServerError
		// }
};