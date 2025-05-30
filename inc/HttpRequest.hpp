/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rpisoner <rpisoner@student.42madrid.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/27 12:07:20 by rpisoner          #+#    #+#             */
/*   Updated: 2025/05/27 12:07:28 by rpisoner         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <string>
#include <map>

class HttpRequest {
public:
	std::string method;
	std::string uri;
	std::string version;
	std::map<std::string, std::string> headers;
	std::string body;	
	
	void parse(const std::string& raw_request);
};

#endif
