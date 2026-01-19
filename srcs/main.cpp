/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdavi-al <sdavi-al@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/16 14:31:56 by brunogue          #+#    #+#             */
/*   Updated: 2026/01/18 16:29:50 by sdavi-al         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include "ConfigParser.hpp"

#include "ConfigParser.hpp"
#include <iostream>

void runServer(const std::vector<ServerConfig>& servers);

int main(int argc, char **argv) {
    if (argc != 2)
    {
        std::cerr << "Usage: ./webserv [config_file]" << std::endl;
        return 1;
    }

    try
    {
        ConfigParser parser(argv[1]);
        std::vector<ServerConfig> servers = parser.parse();
        std::cout << "Config parsed. Starting server..." << std::endl;
        runServer(servers);

    } catch (std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}