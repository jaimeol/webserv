/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rpisoner <rpisoner@student.42madrid.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/27 12:15:41 by rpisoner          #+#    #+#             */
/*   Updated: 2025/05/27 12:17:14 by rpisoner         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/HttpResponse.hpp"
#include <sstream>
#include <stdexcept>
#include <cstdlib>
#include <cctype>
#include <algorithm>

std::string HttpResponse::toString() const {
    std::ostringstream oss;
    oss << version << " " << status_code << " " << status_text << "\r\n";
    for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it) {
        oss << it->first << ": " << it->second << "\r\n";
    }
    oss << "\r\n" << body;
    return oss.str();
}

void HttpResponse::parse(const std::string& raw_response) {
    std::istringstream stream(raw_response);
    std::string line;

    // Status line
    if (!std::getline(stream, line) || line.empty())
        throw std::runtime_error("Invalid HTTP status line");

    if (!line.empty() && line[line.length() - 1] == '\r')
        line.erase(line.length() - 1);

    std::istringstream status_stream(line);
    status_stream >> version;
    std::string code_str;
    status_stream >> code_str;
    std::getline(status_stream, status_text);
    while (!status_text.empty() && std::isspace(status_text[0]))
        status_text.erase(0, 1);

    status_code = std::atoi(code_str.c_str());

    // Headers
    while (std::getline(stream, line) && line != "\r") {
        if (!line.empty() && line[line.length() - 1] == '\r')
            line.erase(line.length() - 1);

        size_t sep = line.find(':');
        if (sep == std::string::npos)
            throw std::runtime_error("Malformed header: " + line);

        std::string key = line.substr(0, sep);
        std::string value = line.substr(sep + 1);

        while (!key.empty() && std::isspace(key[key.size() - 1]))
            key.erase(key.size() - 1);
        while (!value.empty() && std::isspace(value[0]))
            value.erase(0, 1);

        headers[key] = value;
    }

    // Body
    std::ostringstream body_stream;
    body_stream << stream.rdbuf();
    body = body_stream.str();
}
