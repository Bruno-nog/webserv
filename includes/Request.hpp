/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdavi-al <sdavi-al@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/16 17:15:55 by sdavi-al          #+#    #+#             */
/*   Updated: 2026/01/17 11:06:31 by sdavi-al         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <string>
#include <map>
#include <iostream>
#include <sstream>

class Request
{
private:
    std::string _method;
    std::string _path;
    std::string _httpVersion;
    std::map<std::string, std::string> _headers;
    std::string _body;
    void parseRequestLine(const std::string& line);
    void parseHeaders(std::stringstream& ss);
    std::string trim(const std::string& str);

public:
    Request();
    ~Request();
    void parse(const std::string& rawData);
    std::string getMethod() const;
    std::string getPath() const;
    std::string getHttpVersion() const;
    std::string getBody() const;
    std::string getHeader(const std::string& key) const;
    void debugPrint() const;
};

#endif