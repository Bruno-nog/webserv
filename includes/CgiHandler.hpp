/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdavi-al <sdavi-al@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/17 18:18:47 by sdavi-al          #+#    #+#             */
/*   Updated: 2026/01/19 18:19:21 by sdavi-al         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGIHANDLER_HPP
#define CGIHANDLER_HPP

#include "Request.hpp"
#include "Config.hpp"
#include <string>
#include <map>
#include <vector>
#include <unistd.h>

class CgiHandler
{
private:
    Request         _request;
    std::string     _scriptPath;
    std::string     _programPath;
    std::map<std::string, std::string> _env;

    char** _getEnvArray() const;
    void            _setupEnv();

public:
    CgiHandler(Request& request, const std::string& scriptPath, const std::string& programPath);
    ~CgiHandler();

    std::string execute();
};

#endif