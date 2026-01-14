#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <poll.h>
#include <vector>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <cerrno>

void setNonBlocking(int fd)
{
    
    if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1)
    {
        throw std::runtime_error("Erro ao configurar Non-Blocking");
    }
}

// AF_INET = IPv4, SOCK_STREAM = TCP
// htons defines port
// INADDR_ANY accepts connections from any IP address
int setupServer(int port)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) throw std::runtime_error("Erro no socket");

    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

   if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        throw std::runtime_error("Erro no bind");

   if (listen(sockfd, 100) < 0)
        throw std::runtime_error("Erro no listen");

    setNonBlocking(sockfd);
    return sockfd;
}
