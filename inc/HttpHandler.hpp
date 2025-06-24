/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpHandler.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jolivare <jolivare@student.42mad.com>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/29 17:49:17 by rpisoner          #+#    #+#             */
/*   Updated: 2025/06/24 16:52:10 by jolivare         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPHANDLER_HPP
#define HTTPHANDLER_HPP

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "SessionManager.hpp"
#include "Server.hpp"
#include "Location.hpp"
#include "WebServ.hpp"

class HttpHandler {
	public:
		static HttpResponse handleRequest(const HttpRequest& req, const std::vector<Server>& servers);
		static SessionManager sessionManager;
	private:
	    //std::vector<Server> _servers;
		static Location matchLocation(const std::string& uri, const Server& server);
		static HttpResponse handleGET(const HttpRequest& req, const Location& loc);
		static HttpResponse handlePOST(const HttpRequest& req, const Location& loc);
		static HttpResponse handleDELETE(const HttpRequest& req, const Location& loc);
		static std::string readFileContent(const std::string& path);
		static const Server& matchServer(const HttpRequest& req, const std::vector<Server>& servers);
		static std::string matchCookie(const HttpRequest& req, HttpResponse& res);
		static void handleVisits(std::map<std::string, std::string>& session, HttpResponse& res);
};

#endif