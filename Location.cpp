/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jolivare <jolivare@student.42mad.com>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/11 17:15:59 by jolivare          #+#    #+#             */
/*   Updated: 2025/04/11 17:24:02 by jolivare         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Location.hpp"

Location::Location()
{
	this->_path = "";
	this->_root = "";
	this->_index = "";
	this->_autoindex = false;
	this->_returnPath = "";
	this->_alias = "";
	this->client_max_body_size = MAX_REQUEST_LENGTH;
}

Location::~Location() {}

Location::Location(Location const &copy)
{
	this->_path = copy._path;
	this->_root = copy._root;
	this->_index = copy._index;
	this->_autoindex = copy._autoindex;
	this->_methods = copy._methods;
	this->_returnPath = copy._returnPath;
	this->_alias = copy._alias;
	this->_cgi_path = copy._cgi_path;
	this->_cgi_ext = copy._cgi_ext;
	this->_cgi_pairs = copy._cgi_pairs;
	this->client_max_body_size = copy.client_max_body_size;
}