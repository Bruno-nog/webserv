/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main_loop.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdavi-al <sdavi-al@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/16 18:48:45 by sdavi-al          #+#    #+#             */
/*   Updated: 2026/01/16 19:09:44 by sdavi-al         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "socket.hpp"
#include "Client.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>
#include <vector>
#include <unistd.h>
#include <iostream>
#include <map> 

std::map<int, Client*> clients;

void runServer()
{
    std::vector<struct pollfd> fds;

    int server_fd = setupServer(8080);
    
    struct pollfd server_pollfd;
    server_pollfd.fd = server_fd;
    server_pollfd.events = POLLIN;
    fds.push_back(server_pollfd);

    std::cout << "Listening on port 8080..." << std::endl;

    while (true)
    {
        int ret = poll(&fds[0], fds.size(), -1); 

        if (ret < 0) throw std::runtime_error("poll Error");

        for (size_t i = 0; i < fds.size(); i++)
        {
            if (fds[i].fd == server_fd && (fds[i].revents & POLLIN))
            {
                int client_fd = accept(server_fd, NULL, NULL);
                if (client_fd >= 0)
                {
                    setNonBlocking(client_fd);
                    struct pollfd client_pollfd;
                    client_pollfd.fd = client_fd;
                    client_pollfd.events = POLLIN; 
                    fds.push_back(client_pollfd);
                    clients[client_fd] = new Client(client_fd);
                    std::cout << "Client " << client_fd << " connected." << std::endl;
                }
            }
            else if (fds[i].revents & POLLIN)
            {
                char buffer[1024];
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
                } else
                {
                    Client* client = clients[fds[i].fd];
                    client->feedData(buffer, bytes);
                    if (client->isRequestComplete())
                    {
                        client->processRequest();
                        client->getRequest().debugPrint();
                        std::string body = "<h1>Hello from C++ Client Class!</h1>";
                        std::stringstream response;
                        response << "HTTP/1.1 200 OK\r\n";
                        response << "Content-Length: " << body.length() << "\r\n";
                        response << "Content-Type: text/html\r\n\r\n";
                        response << body;
                        client->setResponse(response.str());
                        fds[i].events = POLLIN | POLLOUT;
                    }
                }
            }
            else if (fds[i].revents & POLLOUT)
            {
                Client* client = clients[fds[i].fd];

                if (client->hasResponseToSend())
                {
                    int sent_bytes = send(fds[i].fd, client->getResponseData(), client->getResponseSize(), 0);
                    
                    if (sent_bytes > 0)
                    {
                        client->markBytesSent(sent_bytes);
                    }
                }
                if (!client->hasResponseToSend())
                {
                    std::cout << "Response fully sent to Client " << fds[i].fd << std::endl;
                    client->clear(); 
                    fds[i].events = POLLIN;
                }
            }
        }
    }
}