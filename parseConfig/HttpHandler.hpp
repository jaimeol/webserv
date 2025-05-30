/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpHandler.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rpisoner <rpisoner@student.42madrid.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/29 17:49:17 by rpisoner          #+#    #+#             */
/*   Updated: 2025/05/29 18:09:36 by rpisoner         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPHANDLER_HPP
#define HTTPHANDLER_HPP

#include "../inc/HttpRequest.hpp"
#include "../inc/HttpResponse.hpp"
#include "Server.hpp"
#include "Location.hpp"

class HttpHandler {
public:
    static HttpResponse handleRequest(const HttpRequest& req, const Server& server);
private:
    static Location matchLocation(const std::string& uri, const Server& server);
    static HttpResponse handleGET(const HttpRequest& req, const Location& loc);
    static HttpResponse handlePOST(const HttpRequest& req, const Location& loc);
    static HttpResponse handleDELETE(const HttpRequest& req, const Location& loc);
    static std::string readFileContent(const std::string& path);
};

#endif
