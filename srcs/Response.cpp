/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdavi-al <sdavi-al@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/17 16:28:03 by sdavi-al          #+#    #+#             */
/*   Updated: 2026/01/20 12:10:25 by sdavi-al         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"
#include "CgiHandler.hpp"
#include <sstream>
#include <fstream>
#include <iostream>
#include <sys/stat.h>
#include <dirent.h>
#include <algorithm>
#include <cstdio>

static std::string intToString(int value)
{
    std::stringstream ss;
    ss << value;
    return ss.str();
}

Response::Response(Request& request, ServerConfig& serverConfig) 
    : _request(request), _serverConfig(serverConfig), _statusCode(200) {}

Response::~Response() {}

bool Response::_fileExists(const std::string& path)
{
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0);
}

bool Response::_isDirectory(const std::string& path)
{
    struct stat buffer;
    if (stat(path.c_str(), &buffer) != 0) return false;
    return S_ISDIR(buffer.st_mode);
}

std::string Response::_getContentType(const std::string& path)
{
    size_t dotPos = path.find_last_of('.');
    if (dotPos == std::string::npos) return "text/plain";

    std::string ext = path.substr(dotPos);
    if (ext == ".html" || ext == ".htm") return "text/html";
    if (ext == ".css") return "text/css";
    if (ext == ".js") return "application/javascript";
    if (ext == ".jpg" || ext == ".jpeg") return "image/jpeg";
    if (ext == ".png") return "image/png";
    if (ext == ".gif") return "image/gif";
    if (ext == ".pdf") return "application/pdf";
    if (ext == ".txt") return "text/plain";
    return "application/octet-stream";
}

std::string Response::_readFile(const std::string& path)
{
    std::ifstream file(path.c_str(), std::ios::binary);
    if (!file.is_open()) return "";

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

LocationConfig* Response::_matchLocation(const std::string& path)
{
    LocationConfig* bestMatch = NULL;
    size_t bestMatchLen = 0;

    for (size_t i = 0; i < _serverConfig.locations.size(); ++i)
    {
        std::string locPath = _serverConfig.locations[i].path;
        
        if (path.find(locPath) == 0)
        {
            if (locPath.length() > bestMatchLen)
            {
                bestMatch = &_serverConfig.locations[i];
                bestMatchLen = locPath.length();
            }
        }
    }
    return bestMatch;
}

void Response::_buildErrorResponse(int code)
{
    _statusCode = code;
    std::string body;

    if (_serverConfig.error_pages.count(code))
    {
        std::string errorPath = _serverConfig.root + _serverConfig.error_pages[code];
        if (_fileExists(errorPath))
        {
            body = _readFile(errorPath);
        }
    }

    if (body.empty())
    {
        body = "<html><head><title>Error</title></head><body>";
        body += "<h1>Error " + intToString(code) + "</h1>";
        body += "</body></html>";
    }

    _responseContent = _buildHeader(body.size(), "text/html") + body;
}

std::string Response::_buildHeader(long bodySize, std::string contentType)
{
    std::string statusMsg;
    switch (_statusCode)
    {
        case 200: statusMsg = "OK"; break;
        case 201: statusMsg = "Created"; break;
        case 204: statusMsg = "No Content"; break;
        case 301: statusMsg = "Moved Permanently"; break;
        case 400: statusMsg = "Bad Request"; break;
        case 403: statusMsg = "Forbidden"; break;
        case 404: statusMsg = "Not Found"; break;
        case 405: statusMsg = "Method Not Allowed"; break;
        case 413: statusMsg = "Payload Too Large"; break;
        case 500: statusMsg = "Internal Server Error"; break;
        default:  statusMsg = "Unknown"; break;
    }

    std::stringstream h;
    h << "HTTP/1.1 " << _statusCode << " " << statusMsg << "\r\n";
    h << "Content-Length: " << bodySize << "\r\n";
    h << "Content-Type: " << contentType << "\r\n";
    h << "Connection: keep-alive\r\n";
    h << "\r\n";
    return h.str();
}

std::string Response::_generateAutoIndex(const std::string& path, const std::string& reqPath)
{
    std::string html = "<html><head><title>Index of " + reqPath + "</title></head><body>";
    html += "<h1>Index of " + reqPath + "</h1><hr><ul>";

    DIR* dir = opendir(path.c_str());
    if (dir)
    {
        struct dirent* entry;
        while ((entry = readdir(dir)) != NULL)
        {
            std::string name = entry->d_name;
            if (name == ".") continue;

            std::string link = name;
            if (entry->d_type == DT_DIR) link += "/";
            
            html += "<li><a href=\"" + link + "\">" + name + "</a></li>";
        }
        closedir(dir);
    }
    html += "</ul><hr></body></html>";
    return html;
}

void Response::generateResponse()
{
    LocationConfig* location = _matchLocation(_request.getPath());
    
    std::string root = _serverConfig.root; 
    if (location && !location->root.empty())
    {
        root = location->root;
    }

    if (location && !location->return_url.empty())
    {
        _statusCode = 301;
        std::stringstream h;
        h << "HTTP/1.1 301 Moved Permanently\r\n";
        h << "Location: " << location->return_url << "\r\n\r\n";
        _responseContent = h.str();
        return;
    }

    if (location && !location->allow_methods.empty())
    {
        bool allowed = false;
        for (size_t i = 0; i < location->allow_methods.size(); ++i)
        {
            if (location->allow_methods[i] == _request.getMethod())
            {
                allowed = true;
                break;
            }
        }
        if (!allowed)
        {
            _buildErrorResponse(405);
            return;
        }
    }

    std::string fullPath = root + _request.getPath();

    if (_isDirectory(fullPath)) {
        std::string indexFile = "index.html"; 
        if (location && !location->index.empty()) indexFile = location->index;

        std::string indexPath = fullPath;
        if (indexPath[indexPath.size()-1] != '/') indexPath += "/";
        indexPath += indexFile;

        if (_fileExists(indexPath))
        {
            fullPath = indexPath;
        } 
        else
        {
            if (location && location->autoindex)
            {
                std::string html = _generateAutoIndex(fullPath, _request.getPath());
                _statusCode = 200;
                _responseContent = _buildHeader(html.size(), "text/html") + html;
                return;
            } else
            {
                _buildErrorResponse(403);
                return;
            }
        }
    }

    std::string extension = "";
    size_t dotPos = fullPath.find_last_of('.');
    if (dotPos != std::string::npos)
    {
        extension = fullPath.substr(dotPos);
    }

    if (extension == ".py" || extension == ".php")
    {
        
        if (!_fileExists(fullPath))
        {
            _buildErrorResponse(404);
            return;
        }

        std::string interpreter = "/usr/bin/python3"; 
        if (extension == ".php") interpreter = "/usr/bin/php-cgi";

        try
        {
            CgiHandler cgi(_request, fullPath, interpreter);
            std::string cgiOutput = cgi.execute();
            
            if (cgiOutput.find("HTTP/1.1") == std::string::npos)
            {
                _statusCode = 200;
                _responseContent = "HTTP/1.1 200 OK\r\n" + cgiOutput;
            } else
            {
                _responseContent = cgiOutput;
            }
            return;

        } catch (std::exception &e) {
            std::cerr << "CGI Execution Error: " << e.what() << std::endl;
            _buildErrorResponse(500);
            return;
        }
    }

    if (_request.getMethod() == "GET")
    {
        if (_fileExists(fullPath))
        {
            std::string body = _readFile(fullPath);
            _statusCode = 200;
            std::string type = _getContentType(fullPath);
            _responseContent = _buildHeader(body.size(), type) + body;
        } else
        {
            _buildErrorResponse(404);
        }
    }
    else if (_request.getMethod() == "DELETE")
    {
        if (_fileExists(fullPath))
        {
            if (remove(fullPath.c_str()) == 0)
            {
                _statusCode = 204; 
                _responseContent = "HTTP/1.1 204 No Content\r\n\r\n";
            } else
            {
                _buildErrorResponse(500);
            }
        } else
        {
            _buildErrorResponse(404);
        }
    }
    else if (_request.getMethod() == "POST")
    {
        if (_request.getBody().size() > _serverConfig.client_max_body_size)
        {
            _buildErrorResponse(413);
            return;
        }

        std::string uploadDir = root;
        if (location && !location->upload_path.empty())
        {
            uploadDir = location->upload_path;
        }
        
        std::string filename = uploadDir + "/upload_" + intToString(clock());
        
        std::ofstream outfile(filename.c_str(), std::ios::binary);
        if (outfile.is_open())
        {
            outfile << _request.getBody();
            outfile.close();
            
            std::string msg = "File uploaded successfully to " + filename;
            _statusCode = 201;
            _responseContent = _buildHeader(msg.size(), "text/plain") + msg;
        } else
        {
            _buildErrorResponse(500);
        }
    }
    else
    {
        _buildErrorResponse(501);
    }
}

std::string Response::getResponseString() const
{
    return _responseContent;
}