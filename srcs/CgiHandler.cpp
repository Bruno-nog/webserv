/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdavi-al <sdavi-al@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/18 11:37:08 by sdavi-al          #+#    #+#             */
/*   Updated: 2026/01/20 11:47:09 by sdavi-al         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CgiHandler.hpp"
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sys/wait.h>
#include <fcntl.h>

static std::string toString(long value)
{
    std::stringstream ss;
    ss << value;
    return ss.str();
}


CgiHandler::CgiHandler(Request& request, const std::string& scriptPath, const std::string& programPath)
    : _request(request), _scriptPath(scriptPath), _programPath(programPath) {
    _setupEnv();
}

CgiHandler::~CgiHandler() {}

void CgiHandler::_setupEnv()
{
    _env["REQUEST_METHOD"] = _request.getMethod();
    _env["CONTENT_LENGTH"] = toString(_request.getBody().size());
    _env["CONTENT_TYPE"] = _request.getHeader("Content-Type");
        _env["SCRIPT_NAME"] = _scriptPath;
    _env["SCRIPT_FILENAME"] = _scriptPath;
    _env["PATH_INFO"] = _scriptPath;
        _env["SERVER_PROTOCOL"] = "HTTP/1.1";
    _env["REDIRECT_STATUS"] = "200";
    _env["GATEWAY_INTERFACE"] = "CGI/1.1";
    _env["SERVER_SOFTWARE"] = "Webserv/1.0";
    size_t questPos = _request.getPath().find('?');
    if (questPos != std::string::npos)
    {
        _env["QUERY_STRING"] = _request.getPath().substr(questPos + 1);
    } else
    {
        _env["QUERY_STRING"] = "";
    }
}

char** CgiHandler::_getEnvArray() const
{
    char** envp = new char*[_env.size() + 1];
    int i = 0;
    for (std::map<std::string, std::string>::const_iterator it = _env.begin(); it != _env.end(); ++it)
    {
        std::string s = it->first + "=" + it->second;
        envp[i] = new char[s.size() + 1];
        std::strcpy(envp[i], s.c_str());
        i++;
    }
    envp[i] = NULL;
    return envp;
}

void CgiHandler::_freeEnvArray(char** envp)
{
    for (int i = 0; envp[i] != NULL; i++)
    {
        delete[] envp[i];
    }
    delete[] envp;
}

std::string CgiHandler::execute()
{
    int pipe_in[2];
    int pipe_out[2];

    if (pipe(pipe_in) == -1 || pipe(pipe_out) == -1)
    {
        throw std::runtime_error("Failed to create pipes");
    }

    pid_t pid = fork();
    if (pid == -1)
    {
        throw std::runtime_error("Failed to fork");
    }
    if (pid == 0)
    {
        dup2(pipe_in[0], STDIN_FILENO);
        close(pipe_in[1]);
        close(pipe_in[0]);

        dup2(pipe_out[1], STDOUT_FILENO);
        close(pipe_out[0]);
        close(pipe_out[1]);

        char* argv[] =
        {
            const_cast<char*>(_programPath.c_str()), 
            const_cast<char*>(_scriptPath.c_str()), 
            NULL
        };
        char** envp = _getEnvArray();

        execve(_programPath.c_str(), argv, envp);

        std::cerr << "Execve failed!" << std::endl;
        _freeEnvArray(envp);
        exit(1);
    }
    else
    {
        close(pipe_in[0]);
        close(pipe_out[1]);

        if (!_request.getBody().empty())
        {
            write(pipe_in[1], _request.getBody().c_str(), _request.getBody().size());
        }
        close(pipe_in[1]);

        std::string result;
        char buffer[4096];
        ssize_t bytes;

        while ((bytes = read(pipe_out[0], buffer, sizeof(buffer))) > 0)
        {
            result.append(buffer, bytes);
        }
        close(pipe_out[0]);

        int status;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
        {
            throw std::runtime_error("CGI script exited with error");
        }

        return result;
    }
}