/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handleClient.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: negambar <negambar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/28 12:09:18 by negambar          #+#    #+#             */
/*   Updated: 2025/11/14 14:48:43 by negambar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../library/irc.hpp"

void Server::closeClient(int fd,short rev, int i)
{
    std::string name;
    std::map<int, User*>::iterator it = _users.begin();
    for (; it != _users.end(); ++it)
    {
        if (it->first == fd)
        {
            name = it->second->getNickname();
            break;
        }
    }
    std::cout << "Client " << name << " fd=" << fd << " closed or error (revents=" << rev << ")\n";
                
    // Broadcast disconnection message
    std::string disconn_msg = name + " left the chat.\r\n";
    for (size_t k = 1; k < _pfds.size(); ++k) {
        if (_pfds[k].fd != fd) {
            setOutbuf(_pfds[k].fd, disconn_msg);
            _pfds[k].events |= POLLOUT;
        }
    }

    close(fd);
    _inbuf.erase(fd);
    _outbuf.erase(fd);
    clientCleanUp(fd, i);
}



void    Server::readyForWrite(int fd, int i)
{
    std::string buf = _outbuf[fd];
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
                std::map<int, User*>::iterator it = _users.find(fd);
                std::string disconn_msg = it->second->getNickname() + " left the chat.\r\n";
                for (size_t k = 1; k < _pfds.size(); ++k) {
                    if (_pfds[k].fd != fd) {
                        setOutbuf(_pfds[k].fd, disconn_msg);
                        _pfds[k].events |= POLLOUT;
                    }
                }
                clientCleanUp(fd, i);
                break;
            }
        }
    }
}