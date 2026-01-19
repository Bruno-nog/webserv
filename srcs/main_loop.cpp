/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main_loop.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdavi-al <sdavi-al@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/16 18:48:45 by sdavi-al          #+#    #+#             */
/*   Updated: 2026/01/19 15:10:51 by sdavi-al         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "socket.hpp"
#include "Client.hpp"
#include "Config.hpp"
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

void runServer(const std::vector<ServerConfig>& servers)
{
    std::vector<struct pollfd> fds;
    std::vector<int> ports_bound;
    std::cout << "Initializing servers..." << std::endl;

    for (size_t i = 0; i < servers.size(); ++i)
    {
        int port = servers[i].port;
        bool already_bound = false;
        for (size_t j = 0; j < ports_bound.size(); ++j)
        {
            if (ports_bound[j] == port)
            {
                already_bound = true;
                break;
            }
        }
        if (!already_bound)
        {
            try 
            {
                int fd = setupServer(port);
                server_fds.push_back(fd);
                struct pollfd pfd;
                pfd.fd = fd;
                pfd.events = POLLIN;
                fds.push_back(pfd);
                ports_bound.push_back(port);
                std::cout << "Server listening on port " << port << " (FD: " << fd << ")" << std::endl;
            }
            catch (std::exception &e)
            {
                std::cerr << "Failed to setup server on port " << port << ": " << e.what() << std::endl;
            }
        }
    }

    if (server_fds.empty())
        throw std::runtime_error("No servers could be initialized.");
    while (true)
    {
        int ret = poll(&fds[0], fds.size(), -1); 

        if (ret < 0) throw std::runtime_error("poll Error");

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
                    std::cout << "New connection: Client " << client_fd << " on Server FD " << fds[i].fd << std::endl;
                }
                else
                {
                    std::cerr << "Accept failed." << std::endl;
                }
            }
            else if (!is_server && (fds[i].revents & POLLIN))
            {
                char buffer[4096];
                int bytes = recv(fds[i].fd, buffer, sizeof(buffer), 0);
                if (bytes <= 0)
                {
                    close(fds[i].fd);
                    if (clients.find(fds[i].fd) != clients.end())
                    {
                        delete clients[fds[i].fd];
                        clients.erase(fds[i].fd);
                    }

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
                        std::string body = "<html><body><h1>Webserv is working!</h1><p>Config loaded properly.</p></body></html>";
                        std::stringstream response;
                        response << "HTTP/1.1 200 OK\r\n";
                        response << "Content-Length: " << body.length() << "\r\n";
                        response << "Content-Type: text/html\r\n";
                        response << "Connection: keep-alive\r\n\r\n";
                        response << body;
                        client->setResponse(response.str());
                        fds[i].events = POLLIN | POLLOUT;
                    }
                }
            }
            else if (!is_server && (fds[i].revents & POLLOUT))
            {
                Client* client = clients[fds[i].fd];

                if (client->hasResponseToSend())
                {
                    int sent_bytes = send(fds[i].fd, client->getResponseData(), client->getResponseSize(), 0);
                    client->getRequest().debugPrint();
                    if (sent_bytes > 0)
                    {
                        client->markBytesSent(sent_bytes);
                    }
                    else if (sent_bytes < 0)
                    {
                        std::cerr << "Send error to Client " << fds[i].fd << std::endl;
                    }
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