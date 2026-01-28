/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdavi-al <sdavi-al@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/14 17:19:42 by sdavi-al          #+#    #+#             */
/*   Updated: 2026/01/27 18:18:13 by sdavi-al         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "CgiHandler.hpp"
#include "Request.hpp"
#include <string>
#include <sstream>

class Client {
public:
  enum ClientState {
    READING_REQUEST,
    WRITING_CGI,
    READING_CGI,
    SENDING_RESPONSE,
    FINISHED
  };

private:
  int _fd;
  std::string _requestBuffer;
  std::string _responseBuffer;
  Request _request;
  bool _isRequestReady;
  ClientState _state;
  CgiHandler *_cgiHandler;
  bool _keepAlive;

public:
  Client(int fd);
  ~Client();
  int getFd() const;
  void feedData(const char *data, int size);
  bool isRequestComplete() const;
  void processRequest();
  Request &getRequest();
  void setResponse(const std::string &response);
  bool hasResponseToSend() const;
  const char *getResponseData() const;
  size_t getResponseSize() const;
  void markBytesSent(int bytes);
  void clear();

  ClientState getState() const;
  void setState(ClientState state);
  void setCgiHandler(CgiHandler *handler);
  CgiHandler *getCgiHandler() const;
  void handleCgiOutput(const char *data, int size);
  void handleCgiFinished();
  bool wantsKeepAlive() const;
};

#endif