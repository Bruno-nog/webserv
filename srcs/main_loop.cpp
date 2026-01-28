/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main_loop.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdavi-al <sdavi-al@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/16 18:48:45 by sdavi-al          #+#    #+#             */
/*   Updated: 2026/01/27 18:31:55 by sdavi-al         ###   ########.fr       */
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

#include <sys/wait.h>

std::map<int, int> cgi_to_client;

#include <signal.h>

volatile sig_atomic_t g_running = 1;

void signalHandler(int signum)
{
    (void)signum;
    g_running = 0;
}

void runServer(const std::vector<ServerConfig>& servers)
{
    signal(SIGINT, signalHandler);

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

    while (g_running)
    {
        int ret = poll(&fds[0], fds.size(), -1); 

        if (ret < 0)
        {
            if (errno == EINTR) continue;
            throw std::runtime_error("poll Error");
        }

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
            
            if (cgi_to_client.count(fds[i].fd))
            {
                int client_fd = cgi_to_client[fds[i].fd];
                if (clients.count(client_fd) == 0)
                {
                    close(fds[i].fd);
                    cgi_to_client.erase(fds[i].fd);
                    fds.erase(fds.begin() + i);
                    i--;
                    continue;
                }

                Client* client = clients[client_fd];

                if (fds[i].revents & POLLIN)
                {
                    char buffer[4096];
                    int bytes = read(fds[i].fd, buffer, sizeof(buffer));
                    if (bytes > 0)
                    {
                        client->handleCgiOutput(buffer, bytes);
                    }
                    else
                    {
                        close(fds[i].fd);
                        cgi_to_client.erase(fds[i].fd);
                        CgiHandler* cgi = client->getCgiHandler();
                        if (cgi)
                        {
                            waitpid(cgi->getPid(), NULL, WNOHANG);
                        }
                        client->handleCgiFinished();
                        
                        for (size_t j = 0; j < fds.size(); j++) {
                            if (fds[j].fd == client_fd) {
                                fds[j].events = POLLIN | POLLOUT;
                                break;
                            }
                        }
                        
                        fds.erase(fds.begin() + i);
                        i--;
                    }
                }
                continue;
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

                    if (client->isRequestComplete() && client->getState() == Client::READING_REQUEST) 
                    {
                        client->processRequest();
                        
                        std::cout << "Processing request for: " << client->getRequest().getPath() << std::endl;

                        int s_fd = client_to_server[fds[i].fd];
                        
                        ServerConfig& config = getServerConfig(s_fd, client->getRequest());

                        Response response(client->getRequest(), config);
                        response.generateResponse();
                        
                        CgiHandler* cgi = response.takeCgiHandler();
                        if (cgi)
                        {
                            client->setCgiHandler(cgi);
                            int cgiFd = cgi->getOutputFd();
                            
                            struct pollfd cgi_pfd;
                            cgi_pfd.fd = cgiFd;
                            cgi_pfd.events = POLLIN;
                            fds.push_back(cgi_pfd);
                            cgi_to_client[cgiFd] = fds[i].fd;
                            
                            std::cout << "CGI started for Client " << fds[i].fd << " on Pipe FD " << cgiFd << std::endl;
                        }
                        else
                        {
                            client->setResponse(response.getResponseString());
                            fds[i].events = POLLIN | POLLOUT;
                        }
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

                if (!client->hasResponseToSend() && client->getState() == Client::SENDING_RESPONSE)
                {
                    if (!client->wantsKeepAlive()) 
                    {
                        std::cout << "Closing connection for Client " << fds[i].fd << std::endl;
                        close(fds[i].fd);
                        delete clients[fds[i].fd];
                        clients.erase(fds[i].fd);
                        client_to_server.erase(fds[i].fd);
                        fds.erase(fds.begin() + i);
                        i--;
                    }
                    else 
                    {
                        client->clear();
                        fds[i].events = POLLIN;
                        std::cout << "Response fully sent to Client " << fds[i].fd << " (Keep-Alive)" << std::endl;
                    }
                }
            }
        }
    }

    std::cout << "\nShutting down server..." << std::endl;
    for (size_t i = 0; i < fds.size(); ++i)
    {
        if (fds[i].fd >= 0) close(fds[i].fd);
    }
    
    for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it)
    {
        delete it->second;
    }
    clients.clear();
    client_to_server.clear();
    cgi_to_client.clear();
    server_fds.clear();
}