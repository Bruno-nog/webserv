//POLLIN = data available to be read

void runServer()
{
    std::vector<struct pollfd> fds;

    int server_fd = setupServer(8080);
    
    struct pollfd server_pollfd;
    server_pollfd.fd = server_fd;
    server_pollfd.events = POLLIN;
    fds.push_back(server_pollfd);

    std::cout << "Server listening on port 8080..." << std::endl;

    while (true)
    {
        int ret = poll(&fds[0], fds.size(), -1); 

        if (ret < 0)
        throw std::runtime_error("poll Error");

        for (size_t i = 0; i < fds.size(); i++)
        {
            if (fds[i].revents & POLLIN)
            {
                if (fds[i].fd == server_fd)
                {
                    int client_fd = accept(server_fd, NULL, NULL);
                    if (client_fd >= 0)
                    {
                        setNonBlocking(client_fd);
                        struct pollfd client_pollfd;
                        client_pollfd.fd = client_fd;
                        client_pollfd.events = POLLIN | POLLOUT;
                        fds.push_back(client_pollfd);
                        
                        std::cout << "New client connected: " << client_fd << std::endl;
                    }
                } else
                {
                    char buffer[1024];
                    int bytes = recv(fds[i].fd, buffer, sizeof(buffer), 0);
                    
                    if (bytes <= 0)
                    {
                        close(fds[i].fd);
                        fds.erase(fds.begin() + i);
                        i--;
                    } else
                    {
                        std::cout << "Received: " << std::string(buffer, bytes) << std::endl;
                    }
                }
            }
        }
    }
}