/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main_loop.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdavi-al <sdavi-al@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/16 18:48:45 by sdavi-al          #+#    #+#             */
/*   Updated: 2026/01/20 12:32:07 by sdavi-al         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "socket.hpp"
#include "Client.hpp"
#include "Config.hpp"
#include "Response.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>
#include <vector>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <map>
#include <algorithm>

std::map<int, Client*> clients;

std::vector<int> server_fds;

std::map<int, int> client_to_server;

std::map<int, std::vector<ServerConfig> > configs_map;

ServerConfig& getServerConfig(int server_fd, const Request& req)
{
    std::vector<ServerConfig>& configs = configs_map[server_fd];
    
    if (configs.size() == 1) return configs[0];

    std::string host = req.getHeader("Host");
    size_t colonPos = host.find(':');
    if (colonPos != std::string::npos)
    {
        host = host.substr(0, colonPos);
    }

    for (size_t i = 0; i < configs.size(); ++i)
    {
        if (configs[i].server_name == host)
        {
            return configs[i];
        }
    }

    return configs[0];
}

void runServer(const std::vector<ServerConfig>& servers)
{
    std::vector<struct pollfd> fds;
    std::map<int, int> port_to_fd;
    std::cout << "Initializing servers..." << std::endl;

    for (size_t i = 0; i < servers.size(); ++i)
    {
        int port = servers[i].port;
        int fd = -1;

        if (port_to_fd.count(port))
        {
            fd = port_to_fd[port];
        } else
        {
            try {
                fd = setupServer(port);
                server_fds.push_back(fd);
                port_to_fd[port] = fd;
                
                struct pollfd pfd;
                pfd.fd = fd;
                pfd.events = POLLIN;
                fds.push_back(pfd);
                
                std::cout << "Server listening on port " << port << " (FD: " << fd << ")" << std::endl;
            } catch (std::exception &e)
            {
                std::cerr << "Failed to setup server on port " << port << ": " << e.what() << std::endl;
                continue;
            }
        }

        if (fd != -1)
        {
            configs_map[fd].push_back(servers[i]);
        }
    }

    if (server_fds.empty())
        throw std::runtime_error("No servers could be initialized.");

    while (true)
    {
        int ret = poll(&fds[0], fds.size(), -1); 

        if (ret < 0)
            throw std::runtime_error("poll Error");

        for (size_t i = 0; i < fds.size(); i++)
        {
            bool is_server = false;
            for (size_t s = 0; s < server_fds.size(); ++s)
            {
                if (fds[i].fd == server_fds[s])
                {
                    is_server = true;
                    break;
                }
            }
            if (is_server && (fds[i].revents & POLLIN))
            {
                int client_fd = accept(fds[i].fd, NULL, NULL);
                if (client_fd >= 0)
                {
                    setNonBlocking(client_fd);
                    
                    struct pollfd client_pollfd;
                    client_pollfd.fd = client_fd;
                    client_pollfd.events = POLLIN; 
                    fds.push_back(client_pollfd);
                    clients[client_fd] = new Client(client_fd);
                    client_to_server[client_fd] = fds[i].fd;
                    
                    std::cout << "New connection: Client " << client_fd << " on Server FD " << fds[i].fd << std::endl;
                }
            }
            else if (!is_server && (fds[i].revents & POLLIN))
            {
                char buffer[4096];
                int bytes = recv(fds[i].fd, buffer, sizeof(buffer), 0);
                
                if (bytes <= 0)
                {
                    close(fds[i].fd);
                    if (clients.count(fds[i].fd))
                    {
                        delete clients[fds[i].fd];
                        clients.erase(fds[i].fd);
                    }
                    client_to_server.erase(fds[i].fd);
                    fds.erase(fds.begin() + i);
                    i--;
                    std::cout << "Client disconnected." << std::endl;
                } 
                else
                {
                    Client* client = clients[fds[i].fd];
                    client->feedData(buffer, bytes);

                    if (client->isRequestComplete()) 
                    {
                        client->processRequest();
                        
                        std::cout << "Processing request for: " << client->getRequest().getPath() << std::endl;

                        int s_fd = client_to_server[fds[i].fd];
                        
                        ServerConfig& config = getServerConfig(s_fd, client->getRequest());

                        Response response(client->getRequest(), config);
                        response.generateResponse();
                        
                        client->setResponse(response.getResponseString());
                        
                        fds[i].events = POLLIN | POLLOUT;
                    }
                }
            }

            else if (!is_server && (fds[i].revents & POLLOUT))
            {
                Client* client = clients[fds[i].fd];

                if (client->hasResponseToSend())
                {
                    int sent = send(fds[i].fd, client->getResponseData(), client->getResponseSize(), 0);
                    if (sent > 0) client->markBytesSent(sent);
                }

                if (!client->hasResponseToSend())
                {
                    client->clear();
                    fds[i].events = POLLIN;
                    std::cout << "Response fully sent to Client " << fds[i].fd << std::endl;
                }
            }
        }
    }
}