/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdavi-al <sdavi-al@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/17 11:05:23 by sdavi-al          #+#    #+#             */
/*   Updated: 2026/01/17 11:05:40 by sdavi-al         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>
#include <vector>
#include <map>

//defines how the server must respond to requests for a specific location
struct LocationConfig
{
    std::string                 path;
    std::string                 root;
    std::string                 index;
    bool                        autoindex;
    std::vector<std::string>    allow_methods;
    std::string                 return_url;
    std::string                 cgi_path;
    std::string                 upload_path;
    LocationConfig() : autoindex(false) {}
};

struct ServerConfig
{
    int                         port;
    std::string                 host;
    std::string                 server_name;
    std::string                 root;
    std::string                 index;
    unsigned long               client_max_body_size;
    std::map<int, std::string>  error_pages;
    std::vector<LocationConfig> locations;

    ServerConfig() : port(80), host("127.0.0.1"), client_max_body_size(1000000) {}
};

#endif