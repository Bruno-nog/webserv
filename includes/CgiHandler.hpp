/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdavi-al <sdavi-al@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/17 18:18:47 by sdavi-al          #+#    #+#             */
/*   Updated: 2026/01/27 18:16:57 by sdavi-al         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGIHANDLER_HPP
#define CGIHANDLER_HPP

#include "Config.hpp"
#include "Request.hpp"
#include <map>
#include <string>
#include <unistd.h>
#include <vector>

class CgiHandler {
private:
  Request _request;
  std::string _scriptPath;
  std::string _programPath;
  std::string _pathInfo;
  std::map<std::string, std::string> _env;

  void _setupEnv();
  char **_getEnvArray() const;
  void _freeEnvArray(char **envp);

public:
  CgiHandler(Request &request, const std::string &scriptPath,
             const std::string &programPath);
  ~CgiHandler();

  void setPathInfo(const std::string &pathInfo);
  int start();
  pid_t getPid() const;
  int getOutputFd() const;

private:
  pid_t _pid;
  int _pipeOutFd;
};

#endif