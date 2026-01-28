/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdavi-al <sdavi-al@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/16 17:25:03 by sdavi-al          #+#    #+#             */
/*   Updated: 2026/01/17 11:06:02 by sdavi-al         ###   ########.fr       */
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
            line.erase(line.size() - 1);
        parseRequestLine(line);
    }

    parseHeaders(ss);

    size_t headerEnd = rawData.find("\r\n\r\n");
    if (headerEnd != std::string::npos)
    {
        std::string rawBody = rawData.substr(headerEnd + 4);
        parseBody(rawBody);
    }
}

void Request::parseBody(const std::string& rawBody)
{
    if (getHeader("Transfer-Encoding").find("chunked") != std::string::npos)
    {
        _body = decodeChunked(rawBody);
    }
    else
    {
        _body = rawBody;
    }
}

std::string Request::decodeChunked(const std::string& rawBody)
{
    std::string decoded;
    size_t i = 0;

    while (i < rawBody.size())
    {
        size_t lineEnd = rawBody.find("\r\n", i);
        if (lineEnd == std::string::npos) break;

        std::string sizeStr = rawBody.substr(i, lineEnd - i);
        long chunkSize = std::strtol(sizeStr.c_str(), NULL, 16);

        if (chunkSize == 0) break;

        i = lineEnd + 2;

        if (i + chunkSize <= rawBody.size())
        {
            decoded.append(rawBody.substr(i, chunkSize));
            i += chunkSize;
        }

        if (rawBody.substr(i, 2) == "\r\n")
        {
            i += 2;
        }
    }
    return decoded;
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
            line.erase(line.size() - 1);
        if (line.empty()) break;
        
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
    if (std::string::npos == first) return str;
    size_t last = str.find_last_not_of(" \t");
    return str.substr(first, (last - first + 1));
}

void Request::debugPrint() const
{
    std::cout << "--- REQUEST PARSED ---" << std::endl;
    std::cout << "Method: " << _method << std::endl;
    std::cout << "Path: " << _path << std::endl;
    std::cout << "Chunked: " << (getHeader("Transfer-Encoding").find("chunked") != std::string::npos ? "Yes" : "No") << std::endl;
    std::cout << "Body Size: " << _body.size() << std::endl;
    std::cout << "----------------------" << std::endl;
}