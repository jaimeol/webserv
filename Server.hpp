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
		
		
};