/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdavi-al <sdavi-al@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/18 14:31:56 by brunogue          #+#    #+#             */
/*   Updated: 2026/01/17 11:16:58 by sdavi-al         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include "ConfigParser.hpp"

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        std::cerr << "Usage: ./webserv [config_file]" << std::endl;
        return 1;
    }

    try
    {
        ConfigParser parser(argv[1]);
        std::vector<ServerConfig> servers = parser.parse();
        
        std::cout << "Config parsed successfully!" << std::endl;
        std::cout << "Server 1 listens on port: " << servers[0].port << std::endl;
    } catch (std::exception &e)
    {
        std::cerr << "Config Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}