/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdavi-al <sdavi-al@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/15 16:23:58 by sdavi-al          #+#    #+#             */
/*   Updated: 2026/01/20 16:30:14 by sdavi-al         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "Request.hpp"
#include "Config.hpp"
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/stat.h>

class Response {
private:
    Request         _request;
    ServerConfig    _serverConfig;
    std::string     _responseContent;
    int             _statusCode;

    std::string     _buildHeader(long bodySize, std::string contentType);
    std::string     _readFile(const std::string& path);
    std::string     _getContentType(const std::string& path);
    LocationConfig* _matchLocation(const std::string& path);
    
    bool            _isDirectory(const std::string& path);
    bool            _fileExists(const std::string& path);
    std::string     _generateAutoIndex(const std::string& path, const std::string& reqPath);

    void            _buildErrorResponse(int code);

public:
    Response(Request& request, ServerConfig& serverConfig);
    ~Response();

    void generateResponse();

    std::string getResponseString() const;
};

#endif