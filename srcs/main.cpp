/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdavi-al <sdavi-al@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/16 14:31:56 by brunogue          #+#    #+#             */
/*   Updated: 2026/01/27 15:13:37 by sdavi-al         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include "ConfigParser.hpp"

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


// curl -X POST -H "Content-Type: text/plain" --data "Este e um arquivo de teste" http://localhost:8080/uploads/teste.txt
// curl -X DELETE http://localhost:8080/uploads/upload_
// siege -b -t 10S http://localhost:8080/