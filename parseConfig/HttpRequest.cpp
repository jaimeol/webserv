/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jolivare <jolivare@student.42mad.com>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/27 12:08:35 by rpisoner          #+#    #+#             */
/*   Updated: 2025/06/23 18:40:32 by jolivare         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/HttpRequest.hpp"
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <cctype>

void HttpRequest::parse(const std::string& raw_request) {
	std::istringstream stream(raw_request);
	std::string line;

	// Primera línea: METHOD URI VERSION
	if (!std::getline(stream, line) || line.empty())
		throw std::runtime_error("Invalid HTTP Request Line");

	// Eliminar '\r' al final si existe
	if (!line.empty() && line[line.length() - 1] == '\r')
		line.erase(line.length() - 1);

	std::istringstream line_stream(line);
	if (!(line_stream >> method >> uri >> version))
		throw std::runtime_error("Malformed Request Line");

	// Headers
	while (std::getline(stream, line)) {
		// Fin de headers: línea vacía o solo '\r'
		if (line == "\r" || line == "" || line == "\n")
			break;
		if (!line.empty() && line[line.length() - 1] == '\r')
			line.erase(line.length() - 1);
		size_t sep = line.find(':');
		if (sep == std::string::npos)
			continue; // Ignora líneas mal formateadas (más tolerante)
		std::string key = line.substr(0, sep);
		std::string value = line.substr(sep + 1);
		while (!key.empty() && std::isspace(key[key.size() - 1]))
			key.erase(key.size() - 1);
		while (!value.empty() && std::isspace(value[0]))
			value.erase(0, 1);
		headers[key] = value;
	}

	// Body (si hay)
	std::string content_length_str = headers["Content-Length"];
	if (!content_length_str.empty())
	{
		std::istringstream ss(content_length_str);
		int content_length;
		ss >> content_length;
		
		if (content_length > 0)
		{
			std::string body_data;
			body_data.resize(content_length);
			stream.read(&body_data[0], content_length);
			body = body_data;
		}
	}
	// std::ostringstream body_stream;
	// body_stream << stream.rdbuf();
	// body = body_stream.str();
}