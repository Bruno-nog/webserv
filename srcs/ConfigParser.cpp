/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdavi-al <sdavi-al@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/15 11:12:20 by sdavi-al          #+#    #+#             */
/*   Updated: 2026/01/27 18:02:11 by sdavi-al         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigParser.hpp"
#include <sstream>
#include <stdexcept>

ConfigParser::ConfigParser(const std::string &filePath)
    : _filePath(filePath), _currentTokenIndex(0) {}

ConfigParser::~ConfigParser() {}

void ConfigParser::tokenize() {
  std::ifstream file(_filePath.c_str());
  if (!file.is_open())
    throw std::runtime_error("Could not open config file");

  std::string line;
  while (std::getline(file, line)) {
    size_t commentPos = line.find('#');
    if (commentPos != std::string::npos)
      line = line.substr(0, commentPos);
    std::string buffer;
    for (size_t i = 0; i < line.length(); i++) {
      if (line[i] == '{' || line[i] == '}' || line[i] == ';') {
        buffer += ' ';
        buffer += line[i];
        buffer += ' ';
      } else {
        buffer += line[i];
      }
    }
    std::stringstream ss(buffer);
    std::string token;
    while (ss >> token) {
      _tokens.push_back(token);
    }
  }
  file.close();
}

std::string ConfigParser::nextToken() {
  if (_currentTokenIndex >= _tokens.size())
    return "";
  return _tokens[_currentTokenIndex++];
}

std::string ConfigParser::peekToken() {
  if (_currentTokenIndex >= _tokens.size())
    return "";
  return _tokens[_currentTokenIndex];
}

std::vector<ServerConfig> ConfigParser::parse() {
  tokenize();
  std::vector<ServerConfig> servers;

  while (_currentTokenIndex < _tokens.size()) {
    std::string token = nextToken();
    if (token == "server") {
      if (nextToken() != "{")
        throw std::runtime_error("Expected '{' after server");
      ServerConfig server;
      parseServer(server);
      servers.push_back(server);
    } else {
      throw std::runtime_error("Unexpected token: " + token);
    }
  }
  return servers;
}

void ConfigParser::parseServer(ServerConfig &server) {
  while (_currentTokenIndex < _tokens.size()) {
    std::string token = nextToken();
    if (token == "}")
      return;

    if (token == "listen") {
      server.port = std::atoi(nextToken().c_str());
      if (nextToken() != ";")
        throw std::runtime_error("Expected ';' after listen");
    } else if (token == "host") {
      server.host = nextToken();
      if (nextToken() != ";")
        throw std::runtime_error("Expected ';' after host");
    } else if (token == "server_name") {
      server.server_name = nextToken();
      if (nextToken() != ";")
        throw std::runtime_error("Expected ';' after server_name");
    } else if (token == "root") {
      server.root = nextToken();
      if (nextToken() != ";")
        throw std::runtime_error("Expected ';' after root");
    } else if (token == "error_page") {
      int code = std::atoi(nextToken().c_str());
      std::string page = nextToken();
      server.error_pages[code] = page;
      if (nextToken() != ";")
        throw std::runtime_error("Expected ';' after error_page");
    } else if (token == "client_max_body_size") {
      server.client_max_body_size = std::atol(nextToken().c_str());
      if (nextToken() != ";")
        throw std::runtime_error("Expected ';' after client_max_body_size");
    } else if (token == "location") {
      parseLocation(server);
    }
  }
  throw std::runtime_error("Unexpected end of file inside server block");
}

void ConfigParser::parseLocation(ServerConfig &server) {
  LocationConfig loc;
  loc.path = nextToken();

  if (nextToken() != "{")
    throw std::runtime_error("Expected '{' after location path");

  while (_currentTokenIndex < _tokens.size()) {
    std::string token = nextToken();

    if (token == "}") {
      server.locations.push_back(loc);
      return;
    }

    if (token == "root")
    {
      loc.root = nextToken();
      if (nextToken() != ";")
        throw std::runtime_error("Expected ';' after root");
    } else if (token == "index") 
    {
      loc.index = nextToken();
      if (nextToken() != ";")
        throw std::runtime_error("Expected ';' after index");
    } else if (token == "autoindex")
    {
      std::string val = nextToken();
      loc.autoindex = (val == "on");
      if (nextToken() != ";")
        throw std::runtime_error("Expected ';' after autoindex");
    } else if (token == "allow_methods")
    {
      while (peekToken() != ";")
      {
        loc.allow_methods.push_back(nextToken());
      }
      nextToken();
    } else if (token == "upload_path")
    {
      loc.upload_path = nextToken();
      if (nextToken() != ";")
        throw std::runtime_error("Expected ';' after upload_path");
    } else if (token == "cgi_extension")
    {
      std::string ext = nextToken();
      std::string prog = nextToken();
      loc.cgi_extensions[ext] = prog;
      if (nextToken() != ";")
        throw std::runtime_error("Expected ';' after cgi_extension");
    }
  }
}
