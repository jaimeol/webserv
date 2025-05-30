/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rpisoner <rpisoner@student.42madrid.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/27 12:15:29 by rpisoner          #+#    #+#             */
/*   Updated: 2025/05/27 12:16:45 by rpisoner         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include <string>
#include <map>

class HttpResponse {
public:
    std::string version;
    int status_code;
    std::string status_text;
    std::map<std::string, std::string> headers;
    std::string body;
	std::string toString() const;

    void parse(const std::string& raw_response);
};

#endif
