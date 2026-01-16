/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdavi-al <sdavi-al@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/16 17:19:42 by sdavi-al          #+#    #+#             */
/*   Updated: 2026/01/16 18:33:55 by sdavi-al         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include "Request.hpp"

class Client
{
private:
    int         _fd;
    std::string _requestBuffer;
    std::string _responseBuffer;
    Request     _request;
    bool        _isRequestReady;

public:
    Client(int fd);
    ~Client();
    int getFd() const;
    void feedData(const char* data, int size);
    bool isRequestComplete() const;
    void processRequest();
    Request& getRequest();    
    void setResponse(const std::string& response);
    bool hasResponseToSend() const;
    const char* getResponseData() const;
    size_t getResponseSize() const;
    void markBytesSent(int bytes);
    void clear();
};

#endif