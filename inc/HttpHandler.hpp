/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpHandler.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rpisoner <rpisoner@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/29 17:49:17 by rpisoner          #+#    #+#             */
/*   Updated: 2025/06/19 15:43:37 by rpisoner         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPHANDLER_HPP
#define HTTPHANDLER_HPP

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "Server.hpp"
#include "Location.hpp"

class HttpHandler {
	public:
		static HttpResponse handleRequest(const HttpRequest& req, const std::vector<Server>& servers);
	private:
	    //std::vector<Server> _servers;
		static Location matchLocation(const std::string& uri, const Server& server);
		static HttpResponse handleGET(const HttpRequest& req, const Location& loc);
		static HttpResponse handlePOST(const HttpRequest& req, const Location& loc);
		static HttpResponse handleDELETE(const HttpRequest& req, const Location& loc);
		static std::string readFileContent(const std::string& path);
		static const Server& matchServer(const HttpRequest& req, const std::vector<Server>& servers);

};

#endif
