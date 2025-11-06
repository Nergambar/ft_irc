// main.cpp
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <cstring>
#include <cerrno>
#include <stdio.h>
#include <sstream>
#include <cctype>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include "library/irc.hpp"
/* 
 * fcntl(fd, F_GETFL, 0) legge i flag presenti in fd
 * fcntl(fd, F_SETFL, flags | O_NONBLOCK) aggiunge O_NONBLOCK ai flag presenti
 * 
*/

int set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) return -1;
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) return -1;
    return 0;
}


int make_server_socket(int port) {
    int srv = socket(AF_INET, SOCK_STREAM, 0);
    if (srv < 0) { perror("socket"); return -1; }

    int opt = 1;
    if (setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(srv);
        return -1;
    }

    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(srv, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(srv);
        return -1;
    }

    if (listen(srv, 16) < 0) {
        perror("listen");
        close(srv);
        return -1;
    }

    if (set_nonblocking(srv) < 0) {
        perror("set_nonblocking");
        close(srv);
        return -1;
    }

    return srv;
}

// --- Main Server Logic ---

int main(int argc, char **argv) {
    if (argc != 3) {
        std::cerr << "Usage: ./ircserv <port> [password]\n";
        return 1;
    }
    int port = std::atoi(argv[1]);
    int server_fd = make_server_socket(port);
    if (server_fd < 0) return 1;
    
    std::string password = argv[2];
    Server serv;
    
    std::cout << "Chat Server listening on port " << port << std::endl;
    
    // Vector of pollfd: index 0 -> server_fd, the others -> clients
    std::vector<struct pollfd> pfds;
    struct pollfd p;
    p.fd = server_fd;
    p.events = POLLIN; // Wait for new connections
    p.revents = 0;
    pfds.push_back(p);

    // Buffers for each client (in and out)
    std::map<int, std::string> inbuf;
    std::map<int, std::string> outbuf;
    
    std::map<int, std::string> client_name; // Simple map to store client 'name' (fd in this case)

    std::map<int, bool> authenticated;
    while (true) {
        int timeout = -1; // Wait indefinitely
        int rv = poll(&pfds[0], pfds.size(), timeout);
        if (rv < 0) {
            if (errno == EINTR) continue;
            perror("poll");
            break;
        }
        if (rv == 0) continue;

        // ---- 1) Handle new connections: pfds[0] is the listening socket ----
        if (pfds.size() > 0 && (pfds[0].revents & POLLIN)) {
            struct sockaddr_in cli_addr;
            socklen_t cli_len = sizeof(cli_addr);
            int client_fd = accept(server_fd, (struct sockaddr*)&cli_addr, &cli_len);
            if (client_fd < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    // no more incoming connections ready
                    break;
                }
                perror("accept");
                break;
            }

            if (set_nonblocking(client_fd) < 0) {
                perror("set_nonblocking(client)");
                close(client_fd);
                continue;
            }

            // Add to pfds vector
            struct pollfd np;
            np.fd = client_fd;
            np.events = POLLIN; // Initially interested in reading
            np.revents = 0;
            pfds.push_back(np);

            // Allocate User on the heap so Server can store a valid pointer
            User *u = new User();
            serv.setUser(*u, client_fd);

            // Initialize buffers and client info
            std::string empty = "";
            serv.setInbuf(client_fd, empty);
            serv.setOutbuf(client_fd, empty);

            char ipbuf[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(cli_addr.sin_addr), ipbuf, INET_ADDRSTRLEN);

            serv.setClientName(*u);

            authenticated[client_fd] = password.empty();

            std::cout << "Accepted client fd=" << client_fd
                        << " ip=" << ipbuf
                        << " port=" << ntohs(cli_addr.sin_port) << std::endl;
            pfds[0].revents = 0;
        }

        // ---- 2) Handle existing clients (reading/writing) ----
        for (size_t i = 1; i < pfds.size(); ++i) {
            int fd = pfds[i].fd;
            short rev = pfds[i].revents;

            if (rev == 0) continue;

            // Error or hangup: Close client
            if (rev & (POLLERR | POLLHUP | POLLNVAL)) {
                closeClient(client_name, fd, pfds, outbuf, inbuf, rev, i);
                pfds.erase(pfds.begin() + i);
                --i;
                continue;
            }

            // Data available for reading (POLLIN)
            if (rev & POLLIN) {
                if (recvLoop(fd, serv, inbuf, outbuf, authenticated, password, pfds, client_name, i))
                {
                    // Close client: Cleanup done in the error/hangup block above
                    // The client will be cleaned up in the loop iteration's POLLERR/POLLHUP check 
                    // or we can put the cleanup logic here to be immediate. Let's do it here for immediacy.
                    std::string disconn_msg = client_name[fd] + " left the chat.\r\n";
                    for (size_t k = 1; k < pfds.size(); ++k) {
                        if (pfds[k].fd != fd) {
                            outbuf[pfds[k].fd].append(disconn_msg);
                            pfds[k].events |= POLLOUT;
                        }
                    }
                    
                    serv.clientCleanUp(fd, client_name, authenticated, pfds, i);
                    pfds.erase(pfds.begin() + i);
                    --i;
                    continue;
                }

            } // end POLLIN

            // Socket ready for writing (POLLOUT)
            if (pfds.size() > i && (rev & POLLOUT)) {
                readyForWrite(client_name, fd, pfds, outbuf, inbuf, i);
                // If the buffer is empty, stop asking to write
                if (pfds.size() > i && outbuf.find(fd) != outbuf.end() && outbuf[fd].empty()) {
                    pfds[i].events &= ~POLLOUT;
                }
            } // end POLLOUT

            // Clean up revents for the next iteration
            if (pfds.size() > i) pfds[i].revents = 0;
        } // end for clients
    } // end while
    // Clean shutdown
    serv.~Server();
    for (size_t i = 0; i < pfds.size(); ++i) close(pfds[i].fd);
    return 0;
}