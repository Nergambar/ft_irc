/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handleClient.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: negambar <negambar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/28 12:09:18 by negambar          #+#    #+#             */
/*   Updated: 2025/10/28 14:32:30 by negambar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "library/irc.hpp"

void closeClient(std::map<int, std::string> &client_name, int fd, std::vector<pollfd> &pfds,
        std::map<int, std::string> &outbuf, std::map<int, std::string> &inbuf, short rev,
        int i)
{
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
}



void    readyForWrite(std::map<int, std::string> &client_name, int fd, std::vector<pollfd> &pfds,
        std::map<int, std::string> &outbuf, std::map<int, std::string> &inbuf, int i)
{
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
}