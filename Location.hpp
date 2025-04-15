/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jolivare <jolivare@student.42mad.com>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/11 16:47:10 by jolivare          #+#    #+#             */
/*   Updated: 2025/04/15 16:07:00 by jolivare         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WebServ.hpp"

#define MAX_REQUEST_LENGTH 1048576

class Location
{
	private:
		std::string	_path;
		std::string	_root;
		std::string	_index;
		bool		_autoindex;
		std::vector<std::string>	_methods;
		std::string	_returnPath;
		std::string	_alias;
		std::vector<std::string>	_cgi_path;
		std::vector<std::string>	_cgi_ext;
		std::map<std::string, std::string>	_cgi_pairs;
		unsigned long	client_max_body_size;

	public:
		Location();
		~Location();

		Location(Location const &copy);

		//Setters
		void setPath(std::string const &path);
		void setRoot(std::string const &root);
		void setIndex(std::string const &index);
		void setAutoIndex(std::string const &state);
		void setMethods(std::vector<std::string> const &methods);
		void setReturnPath(std::string const &returnPath);
		void setAlias(std::string const &alias);
		void setCgiPath(std::vector<std::string> const &cgiPath);
		void setCgiExt(std::vector<std::string>  const &cgiExt);
		void setCgiPairs(std::string const &extension, std::string const &path);
		void setClientBodySize(unsigned long const &bodySize);
		void setClientBodySize(std::string const &bodySize);
		
		//Getters
		const std::string  &getPath() const;
		const std::string  &getRoot() const;
		const std::string  &getIndex() const;
		const bool getAutoIndex() const;
		const std::vector<std::string> &getMethods() const;
		const std::string &getReturnPath() const;
		const std::string &getAlias() const;
		const std::vector<std::string> &getCgiPath() const;
		const std::vector<std::string> &getCgiExtension() const;
		const std::map<std::string, std::string> &getCgiPairs() const;
		const std::string &getCgiKey(std::string const &key) const;
		
		
};