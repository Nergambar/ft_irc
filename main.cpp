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
int main(int argc, char **argv) {
    if (argc != 3) {
        std::cerr << "Usage: ./ircserv <port> [password]\n";
        return 1;
    }
    //mettere dentro try and catch
    Server serv(argv[1], argv[2]);//creazione oggetto server
    serv.run();
    
    //int port = std::atoi(argv[1]);
    //std::cout << "Chat Server listening on port " << port << std::endl;
    //int server_fd = serv.make_server_socket(port);
    //if (server_fd < 0) return 1;
    
    //std::string password = argv[2];
    
    // Vector of pollfd: index 0 -> server_fd, the others -> clients
    /* std::vector<struct pollfd> pfds = serv.getPfds();//forse la causa del loop infinito e' mancata cancellazione
    struct pollfd p;
    p.fd = server_fd;
    p.events = POLLIN; // Wait for new connections
    p.revents = 0;
    pfds.push_back(p); //pfds e' private, quindi non funziona. */
    
    /* while (true) {
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

            if (serv.set_nonblocking(client_fd) < 0) {
                perror("set_nonblocking(client)");
                close(client_fd);
                continue;
            }

            // Add to pfds vector
            struct pollfd server_fd;
            server_fd.fd = client_fd;
            server_fd.events = POLLIN; // Initially interested in reading
            server_fd.revents = 0;
            pfds.push_back(server_fd);

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
                std::string inbuf = serv.getInbuf(fd);
                std::string outbuf = serv.getOutbuf(fd);
                serv.closeClient(client_name, fd, pfds, rev, i);
                pfds.erase(pfds.begin() + i);
                --i;
                continue;
            }

            // Data available for reading (POLLIN)
            if (rev & POLLIN) {
                std::string inbuf = serv.getInbuf(fd);
                std::string outbuf = serv.getOutbuf(fd);
                if (serv.recvLoop(fd, client_name, i, pfds))
                {
                    // Close client: Cleanup done in the error/hangup block above
                    // The client will be cleaned up in the loop iteration's POLLERR/POLLHUP check 
                    // or we can put the cleanup logic here to be immediate. Let's do it here for immediacy.
                    std::string disconn_msg = client_name[fd] + " left the chat.\r\n";
                    for (size_t k = 1; k < pfds.size(); ++k) {
                        if (pfds[k].fd != fd) {
							//notifica a tutti i client che un client ha lasciato il server
							//cambiare con quel
                            serv.getOutbuf(pfds[k].fd).append(disconn_msg);
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
                std::string inbuf = serv.getInbuf(fd);
                std::string outbuf = serv.getOutbuf(fd);
                serv.readyForWrite(client_name, fd, pfds, i);
                // If the buffer is empty, stop asking to write
                if (pfds.size() > i && serv.getOutbuf(fd).empty()) {
                    pfds[i].events &= ~POLLOUT;
                }
            } // end POLLOUT

            // Clean up revents for the next iteration
            if (pfds.size() > i) pfds[i].revents = 0;
        } // end for clients
    } // end while */
    
    // Clean shutdown
    serv.~Server();
    //for (size_t i = 0; i < pfds.size(); ++i) close(pfds[i].fd); //spostato in ~Server
    return 0;
}