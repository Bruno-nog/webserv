/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdavi-al <sdavi-al@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/16 18:48:39 by sdavi-al          #+#    #+#             */
/*   Updated: 2026/01/27 18:28:56 by sdavi-al         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include <cstdlib>
#include <iostream>

Client::Client(int fd) : _fd(fd), _isRequestReady(false), _state(READING_REQUEST), 
      _cgiHandler(NULL), _keepAlive(true) {}


Client::~Client() 
{
    if (_cgiHandler) delete _cgiHandler;
}

bool Client::wantsKeepAlive() const {
    return _keepAlive;
}

static std::string intToString(int value)
{
    std::stringstream ss;
    ss << value;
    return ss.str();
}

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

    std::string headers = _requestBuffer.substr(0, headerEnd);

    if (headers.find("Transfer-Encoding: chunked") != std::string::npos)
    {
        if (_requestBuffer.find("0\r\n\r\n") != std::string::npos)
        {
            return true;
        }
        return false;
    }

    size_t clPos = headers.find("Content-Length:");
    if (clPos != std::string::npos)
    {
        size_t valStart = clPos + 15;
        long contentLength = std::atol(headers.c_str() + valStart);
        
        size_t totalRequired = headerEnd + 4 + contentLength;
        
        if (_requestBuffer.size() >= totalRequired)
        {
            return true;
        }
        else
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
    _state = SENDING_RESPONSE;
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

void Client::clear() {
    _requestBuffer.clear();
    _responseBuffer.clear();
    _isRequestReady = false;
    _state = READING_REQUEST;
    _keepAlive = true;
    if (_cgiHandler)
    {
        delete _cgiHandler;
        _cgiHandler = NULL;
    }
}

Client::ClientState Client::getState() const
{
    return _state;
}

void Client::setState(ClientState state)
{
    _state = state;
}

void Client::setCgiHandler(CgiHandler* handler)
{
    _cgiHandler = handler;
    _state = READING_CGI;
}

CgiHandler* Client::getCgiHandler() const
{
    return _cgiHandler;
}

void Client::handleCgiOutput(const char* data, int size)
{
    _responseBuffer.append(data, size);
}

void Client::handleCgiFinished()
{
    if (_responseBuffer.find("HTTP/1.1") == std::string::npos)
    {
         std::string headers = "HTTP/1.1 200 OK\r\n";
         
         size_t bodyStart = _responseBuffer.find("\r\n\r\n");
         
         if (bodyStart != std::string::npos)
         {
             size_t bodySize = _responseBuffer.size() - (bodyStart + 4);
             
             std::stringstream ss;
             ss << "Content-Length: " << bodySize << "\r\n";
             
             headers += ss.str();
         }
         else
         {
             headers += "Content-Length: " + intToString(_responseBuffer.size()) + "\r\n";
         }
         
         _responseBuffer.insert(0, headers);
    }
    
    _state = SENDING_RESPONSE;
}