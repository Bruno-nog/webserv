/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdavi-al <sdavi-al@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/16 18:48:39 by sdavi-al          #+#    #+#             */
/*   Updated: 2026/01/16 19:09:40 by sdavi-al         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

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
    return _requestBuffer.find("\r\n\r\n") != std::string::npos;
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