/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdavi-al <sdavi-al@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/16 17:25:03 by sdavi-al          #+#    #+#             */
/*   Updated: 2026/01/16 19:09:28 by sdavi-al         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"
#include <cstdlib>

Request::Request() {}
Request::~Request() {}

std::string Request::getMethod() const { return _method; }
std::string Request::getPath() const { return _path; }
std::string Request::getHttpVersion() const { return _httpVersion; }
std::string Request::getBody() const { return _body; }

std::string Request::getHeader(const std::string& key) const
{
    std::map<std::string, std::string>::const_iterator it = _headers.find(key);
    if (it != _headers.end())
    {
        return it->second;
    }
    return "";
}

void Request::parse(const std::string& rawData)
{
    std::stringstream ss(rawData);
    std::string line;

    if (std::getline(ss, line))
    {
        if (!line.empty() && line[line.size() - 1] == '\r')
        {
            line.erase(line.size() - 1);
        }
        parseRequestLine(line);
    }

    parseHeaders(ss);
    std::string bodyPart;
    while (std::getline(ss, bodyPart))
    {
        _body += bodyPart + "\n";
    }
}

void Request::parseRequestLine(const std::string& line)
{
    std::stringstream ss(line);
    ss >> _method >> _path >> _httpVersion;
    if (_path.empty()) _path = "/";
}

void Request::parseHeaders(std::stringstream& ss)
{
    std::string line;
    
    while (std::getline(ss, line))
    {
        if (!line.empty() && line[line.size() - 1] == '\r')
        {
            line.erase(line.size() - 1);
        }
        if (line.empty())
        {
            break;
        }
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos)
        {
            std::string key = line.substr(0, colonPos);
            std::string value = line.substr(colonPos + 1);
            _headers[key] = trim(value);
        }
    }
}

std::string Request::trim(const std::string& str)
{
    size_t first = str.find_first_not_of(" \t");
    if (std::string::npos == first)
    {
        return str;
    }
    size_t last = str.find_last_not_of(" \t");
    return str.substr(first, (last - first + 1));
}

void Request::debugPrint() const {
    std::cout << "--- REQUEST PARSED ---" << std::endl;
    std::cout << "Method: " << _method << std::endl;
    std::cout << "Path: " << _path << std::endl;
    std::cout << "Version: " << _httpVersion << std::endl;
    std::cout << "Headers:" << std::endl;
    for (std::map<std::string, std::string>::const_iterator it = _headers.begin(); it != _headers.end(); ++it) {
        std::cout << "  " << it->first << ": " << it->second << std::endl;
    }
    std::cout << "Body Preview (first 50 chars): " << _body.substr(0, 50) << std::endl;
    std::cout << "----------------------" << std::endl;
}