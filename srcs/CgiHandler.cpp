/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdavi-al <sdavi-al@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/18 11:37:08 by sdavi-al          #+#    #+#             */
/*   Updated: 2026/01/27 18:09:39 by sdavi-al         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CgiHandler.hpp"
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <sstream>
#include <sys/wait.h>

static std::string toString(long value) {
  std::stringstream ss;
  ss << value;
  return ss.str();
}

CgiHandler::CgiHandler(Request &request, const std::string &scriptPath,
                       const std::string &programPath)
    : _request(request), _scriptPath(scriptPath), _programPath(programPath),
      _pid(-1), _pipeOutFd(-1) {
  _setupEnv();
}

CgiHandler::~CgiHandler() {
  if (_pipeOutFd != -1)
    close(_pipeOutFd);
}

void CgiHandler::setPathInfo(const std::string &pathInfo) {
  _pathInfo = pathInfo;
  _setupEnv();
}

void CgiHandler::_setupEnv() {
  _env["REQUEST_METHOD"] = _request.getMethod();
  _env["CONTENT_LENGTH"] = toString(_request.getBody().size());
  _env["CONTENT_TYPE"] = _request.getHeader("Content-Type");
  _env["SCRIPT_NAME"] = _scriptPath;
  _env["SCRIPT_FILENAME"] = _scriptPath;
  _env["PATH_INFO"] = _pathInfo;
  _env["PATH_TRANSLATED"] = _scriptPath + _pathInfo;
  _env["SERVER_PROTOCOL"] = "HTTP/1.1";
  _env["REDIRECT_STATUS"] = "200";
  _env["GATEWAY_INTERFACE"] = "CGI/1.1";
  _env["SERVER_SOFTWARE"] = "Webserv/1.0";
  size_t questPos = _request.getPath().find('?');
  if (questPos != std::string::npos) {
    _env["QUERY_STRING"] = _request.getPath().substr(questPos + 1);
  } else {
    _env["QUERY_STRING"] = "";
  }
}

char **CgiHandler::_getEnvArray() const
{
  char **envp = new char *[_env.size() + 1];
  int i = 0;
  for (std::map<std::string, std::string>::const_iterator it = _env.begin();
       it != _env.end(); ++it) {
    std::string s = it->first + "=" + it->second;
    envp[i] = new char[s.size() + 1];
    std::strcpy(envp[i], s.c_str());
    i++;
  }
  envp[i] = NULL;
  return envp;
}

void CgiHandler::_freeEnvArray(char **envp)
{
  for (int i = 0; envp[i] != NULL; i++)
  {
    delete[] envp[i];
  }
  delete[] envp;
}

int CgiHandler::start()
{
  int pipe_in[2];
  int pipe_out[2];

  if (pipe(pipe_in) == -1 || pipe(pipe_out) == -1)
  {
    throw std::runtime_error("Failed to create pipes");
  }

  _pid = fork();
  if (_pid == -1) {
    close(pipe_in[0]);
    close(pipe_in[1]);
    close(pipe_out[0]);
    close(pipe_out[1]);
    throw std::runtime_error("Failed to fork");
  }
  if (_pid == 0) {
    dup2(pipe_in[0], STDIN_FILENO);
    close(pipe_in[1]);
    close(pipe_in[0]);

    dup2(pipe_out[1], STDOUT_FILENO);
    close(pipe_out[0]);
    close(pipe_out[1]);

    char *argv[] = {const_cast<char *>(_programPath.c_str()),
                    const_cast<char *>(_scriptPath.c_str()), NULL};
    char **envp = _getEnvArray();

    execve(_programPath.c_str(), argv, envp);

    std::cerr << "Execve failed!" << std::endl;
    _freeEnvArray(envp);
    std::exit(1);
  } else {
    close(pipe_in[0]);
    close(pipe_out[1]);

    if (!_request.getBody().empty()) {
      write(pipe_in[1], _request.getBody().c_str(), _request.getBody().size());
    }
    close(pipe_in[1]);

    _pipeOutFd = pipe_out[0];


    if (fcntl(_pipeOutFd, F_SETFL, O_NONBLOCK) == -1)
      std::cerr << "Failed to set non-blocking on CGI pipe" << std::endl;

    return _pipeOutFd;
  }
}

pid_t CgiHandler::getPid() const { return _pid; }

int CgiHandler::getOutputFd() const { return _pipeOutFd; }