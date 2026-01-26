/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdavi-al <sdavi-al@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/16 18:48:39 by sdavi-al          #+#    #+#             */
/*   Updated: 2026/01/21 17:25:30 by sdavi-al         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

/* ************************************************************************** */
/* */
/* :::      ::::::::   */
/* Client.cpp                                         :+:      :+:    :+:   */
/* +:+ +:+         +:+     */
/* By: sdavi-al <sdavi-al@student.42.fr>          +#+  +:+       +#+        */
/* +#+#+#+#+#+   +#+           */
/* Created: 2026/01/16 18:48:39 by sdavi-al          #+#    #+#             */
/* Updated: 2026/01/21 17:30:00 by sdavi-al         ###   ########.fr       */
/* */
/* ************************************************************************** */

#include "Client.hpp"
#include <cstdlib>
#include <iostream>

Client::Client(int fd) : _fd(fd), _isRequestReady(false) {}

Client::~Client() {}

int Client::getFd() const
{
    return _fd;
}

void Client::feedData(const char* data, int size)
{
    _requestBuffer.append(data, size);
}

bool Client::isRequestComplete() const
{
    size_t headerEnd = _requestBuffer.find("\r\n\r\n");
    
    if (headerEnd == std::string::npos)
    {
        return false;
    }

    size_t clPos = _requestBuffer.find("Content-Length:", 0);
    
    if (clPos != std::string::npos && clPos < headerEnd)
    {
        size_t valStart = clPos + 15;
        long contentLength = std::atol(_requestBuffer.c_str() + valStart);
        
        size_t totalRequired = headerEnd + 4 + contentLength;
        
        if (_requestBuffer.size() >= totalRequired)
        {
            return true;
        } else
        {
            return false;
        }
    }
    return true;
}

void Client::processRequest()
{
    if (isRequestComplete())
    {
        _request.parse(_requestBuffer);
        _isRequestReady = true;
    }
}

Request& Client::getRequest()
{
    return _request;
}


void Client::setResponse(const std::string& response)
{
    _responseBuffer = response;
}

bool Client::hasResponseToSend() const
{
    return !_responseBuffer.empty();
}

const char* Client::getResponseData() const
{
    return _responseBuffer.c_str();
}

size_t Client::getResponseSize() const
{
    return _responseBuffer.size();
}

void Client::markBytesSent(int bytes)
{
    if (bytes > 0 && (size_t)bytes <= _responseBuffer.size())
    {
        _responseBuffer.erase(0, bytes);
    }
}

void Client::clear()
{
    _requestBuffer.clear();
    _responseBuffer.clear();
    _isRequestReady = false;
}