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

bool startswith(const std::string &s, const std::string &s2)
{
    if (s2.size() > s.size()) return false;
    return s.compare(0, s2.size(), s2) == 0;
}

bool handle_command(int fd, const std::string &line,
                    std::map<int,std::string> &outbuf,
                    std::map<int,std::string> &client,
                    std::string       &server_password,
                    std::vector<struct pollfd> &pfds)
{
    if (line.empty() || (line[0] != '/' && !startswith(line, "PASS") && !startswith(line, "NICK")))
        return (false);
    std::istringstream iss(line);
    std::string cmd;
    iss >> cmd;

    if (cmd == "/nick" || cmd == "NICK")
    {
        std::string newNick;
        std::ostringstream ss;
        ss << fd;
        iss >> newNick;
        bool is_taken = false;
        for (std::map<int, std::string>::iterator it = client.begin(); it != client.end(); ++it) {
            if (it->first != fd && it->second == newNick) {
                is_taken = true;
                break;
            }
        }
        if (is_taken) {
            outbuf[fd].append("Nickname '"+ newNick + "' is already in use. Please choose another.\r\n");//ERR_NICKCOLLISION
            return (true);
        } else {
            if (newNick.empty())
                outbuf[fd].append("Usage: /nick <name> OR NICK <name>\n");
            else{
                std::string old;
                if (client[fd].empty())
                {
                    old = "user" + ss.str();
                }
                else
                    old = client[fd];
                client[fd] = newNick;
                std::string msg = old + " is now " + newNick + "\r\n";
                for (size_t k = 1; k < pfds.size(); ++k){
                    if (pfds[k].fd != fd){
                        outbuf[pfds[k].fd].append(msg);
                        pfds[k].events |= POLLOUT;
                    }
                }
                outbuf[fd].append("You are now "+ newNick + "\r\n");
            }
            return (true);
        }
    }
    else if (cmd == "/pass" || cmd == "PASS")
    {
        std::string newpw;
        iss >> newpw;
        if (newpw.empty()) {
            outbuf[fd].append("Usage: /pass <newpassword>  (use non-empty value to set)\r\n");
        } else if (newpw == server_password) {
			outbuf[fd].append("Can't change into the same password as before.\r\n");
		}
		else {
            server_password = newpw;
            outbuf[fd].append("Server password changed.\r\n");
            // Notify other clients (optional)
            std::string notice = "Server password has been changed by " + client[fd] + ".\r\n";
            for (size_t k = 1; k < pfds.size(); ++k) {
                if (pfds[k].fd != fd) {
                    outbuf[pfds[k].fd].append(notice);
                    pfds[k].events |= POLLOUT;
                }
            }
        }
        return (true);
    }
    outbuf[fd].append(std::string("Unknown command: ") + cmd + "\r\n");
    return true;
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

    std::cout << "Chat Server listening on port " << port << std::endl;
    
    // A map to hold messages to be broadcasted to all other clients
    std::map<int, std::string> broadcast_outbuf;
    
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
            while (true) {
                struct sockaddr_in cli_addr;
                socklen_t cli_len = sizeof(cli_addr);
                int client_fd = accept(server_fd, (struct sockaddr*)&cli_addr, &cli_len);
                if (client_fd < 0) {
                    if (errno == EAGAIN || errno == EWOULDBLOCK) {
                        break; // No more connections ready
                    } else {
                        perror("accept");
                        break;
                    }
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

                // Initialize buffers and client info
                inbuf[client_fd] = "";
                outbuf[client_fd] = "";
                
                char ipbuf[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &(cli_addr.sin_addr), ipbuf, INET_ADDRSTRLEN);
                
                std::ostringstream name_os;
                name_os << "user" << client_fd;
                client_name[client_fd] = name_os.str();
                
                authenticated[client_fd] = password.empty();

                std::cout << "Accepted client fd=" << client_fd
                          << " ip=" << ipbuf
                          << " port=" << ntohs(cli_addr.sin_port) << std::endl;
                
                outbuf[client_fd].append("Welcome. This server requires a password.\r\n");
                outbuf[client_fd].append("Enter password:\r\n");
                // Ensure the prompt is sent out
                for (size_t k = 1; k < pfds.size(); ++k)
                {
                    if (pfds[k].fd == client_fd)
                    {
                        pfds[k].events |= POLLOUT;
                    }
                }

            }
            pfds[0].revents = 0;
        }

        // ---- 2) Handle existing clients (reading/writing) ----
        for (size_t i = 1; i < pfds.size(); ++i) {
            int fd = pfds[i].fd;
            short rev = pfds[i].revents;

            if (rev == 0) continue;

            // Error or hangup: Close client
            if (rev & (POLLERR | POLLHUP | POLLNVAL)) {
                std::cout << "Client " << client_name[fd] << " fd=" << fd << " closed or error (revents=" << rev << ")\n";
                
                // Broadcast disconnection message
                std::string disconn_msg = client_name[fd] + " left the chat.\r\n";
                for (size_t k = 1; k < pfds.size(); ++k) {
                    if (pfds[k].fd != fd) {
                        outbuf[pfds[k].fd].append(disconn_msg);
                        pfds[k].events |= POLLOUT;
                    }
                }

                close(fd);
                inbuf.erase(fd);
                outbuf.erase(fd);
                client_name.erase(fd);
                pfds.erase(pfds.begin() + i);
                --i;
                continue;
            }

            // Data available for reading (POLLIN)
            if (rev & POLLIN) {
                bool closed = false;
                while (true) {
                    char buf[4096];
                    ssize_t n = recv(fd, buf, sizeof(buf), 0);
                    if (n > 0) {
                        inbuf[fd].append(buf, buf + n);

						// 🔍 stampa quello che è arrivato
						std::string received(buf, n); // crea stringa dai bytes ricevuti
						std::cout << "[RECV fd=" << fd << "] " << received << std::endl;
                        
                        // Process line(s) terminated by '\n'
                        size_t pos;
                        while ((pos = inbuf[fd].find('\n')) != std::string::npos) {
                            std::string line = inbuf[fd].substr(0, pos + 1);
                            inbuf[fd].erase(0, pos + 1);
                            
                            std::string trimmed = line;
                            
                            while (!trimmed.empty() && (trimmed[trimmed.size()-1] == '\n' || trimmed[trimmed.size()-1] == '\r'))
                                trimmed.erase(trimmed.size()-1, 1);

                            if (!authenticated[fd] && !password.empty()) {
                                if (trimmed.empty()) {
                                    outbuf[fd].append("Enter password:\r\n");
                                } else if (trimmed == password) {
                                    authenticated[fd] = true;
                                    outbuf[fd].append("Password accepted. You are now authenticated.\r\n");
                                    outbuf[fd].append("Set your nickname:\r\n");
                                    pfds[i].events |= POLLOUT;
                                    // Broadcast join now that this client is authenticated
                                    pfds[i].events |= POLLOUT;
                                } else {
                                    outbuf[fd].append("Incorrect password. Try again:\r\n");
                                }
                                pfds[i].events |= POLLOUT;
                            } 
                            else if (authenticated[fd] && client_name[fd].find("user") == 0) {
                                // The client has authenticated (or didn't need a password)
                                // and still has the default "userN" name.
                                
                                std::string newNick = trimmed;
                                if (newNick.empty()) {
                                    outbuf[fd].append("Nickname cannot be empty. Please choose your nickname:\r\n");
                                } else {
                                    // Simple check if the nickname is already taken (case-sensitive)
                                    bool is_taken = false;
                                    for (std::map<int, std::string>::iterator it = client_name.begin(); it != client_name.end(); ++it) {
                                        if (it->first != fd && it->second == newNick) {
                                            is_taken = true;
                                            break;
                                        }
                                    }

                                    if (is_taken) {
                                        outbuf[fd].append("Nickname '"+ newNick + "' is already in use. Please choose another:\r\n");
                                    } else {
                                        std::string oldNick = client_name[fd];
                                        client_name[fd] = newNick;
                                        outbuf[fd].append("You are now known as " + newNick + ".\r\n");
                                        
                                        // Broadcast join message now that the nickname is set
                                        std::string join_msg = newNick + " joined the chat.\r\n";
                                        for (size_t k = 1; k < pfds.size(); ++k) {
                                            if (pfds[k].fd != fd) {
                                                outbuf[pfds[k].fd].append(join_msg);
                                                pfds[k].events |= POLLOUT;
                                            }
                                        }
                                    }
                                }
                                pfds[i].events |= POLLOUT;

                            }
                            else {
                                if (handle_command(fd, trimmed, outbuf, client_name, password, pfds)) {
                                    // A command was handled; make sure sender gets any response queued by handle_command
                                    pfds[i].events |= POLLOUT;
                                } 
                                else {
                                    // Remove trailing CR/LF
                                    while (!trimmed.empty() && (trimmed[trimmed.size()-1] == '\n' || trimmed[trimmed.size()-1] == '\r'))
                                        trimmed.erase(trimmed.size()-1, 1);

                                    std::string message_to_broadcast = "[" + client_name[fd] + "]: " + trimmed + "\r\n";

                                    // Queue message to all *other* connected clients
                                    for (size_t k = 1; k < pfds.size(); ++k) {
                                        if (pfds[k].fd != fd) {
                                            outbuf[pfds[k].fd].append(message_to_broadcast);
                                            pfds[k].events |= POLLOUT;
                                        }
                                    }
                                    // Echo back to sender
                                    outbuf[fd].append("You said: " + trimmed + "\r\n");
                                    pfds[i].events |= POLLOUT;
                                }
                            }
                        }
                    } else if (n == 0) {
                        std::cout << "Client " << client_name[fd] << " fd=" << fd << " disconnected (recv==0)\n";
                        closed = true;
                        break;
                    } else {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            break; // No more data to read now
                        } else {
                            perror("recv");
                            closed = true;
                            break;
                        }
                    }
                } // end while recv loop

                if (closed) {
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
                    
                    close(fd);
                    inbuf.erase(fd);
                    outbuf.erase(fd);
                    client_name.erase(fd);
                    pfds.erase(pfds.begin() + i);
                    --i;
                    continue;
                }
            } // end POLLIN

            // Socket ready for writing (POLLOUT)
            if (pfds.size() > i && (rev & POLLOUT)) {
                std::string &buf = outbuf[fd];
                while (!buf.empty()) {
                    ssize_t n = send(fd, buf.c_str(), buf.size(), 0);
                    if (n > 0) {
                        buf.erase(0, n);
                    } else {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            break; // Cannot send now, try again later
                        } else {
                            perror("send");
                            // Close client on error
                            std::string disconn_msg = client_name[fd] + " left the chat.\r\n";
                            for (size_t k = 1; k < pfds.size(); ++k) {
                                if (pfds[k].fd != fd) {
                                    outbuf[pfds[k].fd].append(disconn_msg);
                                    pfds[k].events |= POLLOUT;
                                }
                            }
                            close(fd);
                            inbuf.erase(fd);
                            outbuf.erase(fd);
                            client_name.erase(fd);
                            pfds.erase(pfds.begin() + i);
                            --i;
                            break;
                        }
                    }
                }
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
    for (size_t i = 0; i < pfds.size(); ++i) close(pfds[i].fd);
    return 0;
}