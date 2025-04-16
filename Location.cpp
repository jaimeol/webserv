/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jolivare <jolivare@student.42mad.com>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/11 17:15:59 by jolivare          #+#    #+#             */
/*   Updated: 2025/04/16 16:16:29 by jolivare         ###   ########.fr       */
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

void Location::setPath(std::string const &path)
{
	this->_path = path;
}

void Location::setRoot(std::string const &root)
{
	this->_root = root;
}

void Location::setIndex(std::string const &index)
{
	this->_index = index;
}

void Location::setAutoIndex(std::string const &state)
{
	if (state == "on")
		this->_autoindex = true;
	else if (state == "off")
		this->_autoindex = false;
	else
		throw std::runtime_error("Wrong autoindex parameter");
}

void Location::setMethods(std::vector<std::string> const &methods)
{
	this->_methods.clear();

	std::string valid[7] = {"GET", "POST", "DELETE", "PUT", "HEAD", "OPTIONS", "PATCH"};

	for (size_t i = 0; i < methods.size(); i++)
	{
		bool isValid = false;
		for (size_t j = 0; j < 7; j++)
		{
			if (methods[i] == valid[j])
			{
				isValid = true;
				break;
			}
			if (isValid)
				this->_methods.push_back(methods[i]);
			else
				throw std::runtime_error("Invalid HTTP method: " + methods[i]);
		}
	}
}

void Location::setReturnPath(std::string const &returnPath)
{
	this->_returnPath = returnPath;
}

void Location::setAlias(std::string const &alias)
{
	this->_alias = alias;
}

void Location::setCgiPath(std::vector<std::string> const &cgiPath)
{
	this->_cgi_path = cgiPath;
}

void Location::setCgiExt(std::vector<std::string> const &cgiExt)
{
	this->_cgi_ext = cgiExt;
}

void Location::setCgiPairs(std::string const &extension, std::string const &path)
{
	this->_cgi_pairs.insert(std::make_pair(extension, path));
}

void Location::setClientBodySize(unsigned long const &bodySize)
{
	this->client_max_body_size = bodySize;
}

void Location::setClientBodySize(std::string const &bodySize)
{
	for (size_t i = 0; i < bodySize.length(); i++)
	{
		if (!isdigit(bodySize[i]))
			throw std::runtime_error("Wrong argument in client body size: " + bodySize[i]);
	}
	this->client_max_body_size = atol(bodySize.c_str());
	if (!client_max_body_size)
		throw std::runtime_error("Error converting max body size: " + bodySize);
	
}

const std::string &Location::getPath() const
{
	return this->_path;
}

const std::string &Location::getRoot() const
{
	return this->_root;
}

const std::string &Location::getIndex() const
{
	return this->_index;
}

const bool Location::getAutoIndex() const
{
	return this->_autoindex;
}

const std::vector<std::string> &Location::getMethods() const
{
	return this->_methods;
}

const std::string &Location::getReturnPath() const
{
	return this->_returnPath;
}

const std::string &Location::getAlias() const
{
	return this->_alias;
}

const std::vector<std::string> &Location::getCgiPath() const
{
	return this->_cgi_path;
}	

const std::vector<std::string> &Location::getCgiExtension() const
{
	return this->_cgi_ext;
}

const std::map<std::string, std::string> &Location::getCgiPairs() const
{
	return this->_cgi_pairs;
}


const std::string &Location::getCgiKey(std::string const &key) const
{
	std::map<std::string, std::string>::const_iterator it = this->_cgi_pairs.find(key);
	if (it == this->_cgi_pairs.end())
		throw std::runtime_error("No cgi key found for :" + key);
	return it->second;
}



