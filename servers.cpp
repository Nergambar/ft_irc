/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   servers.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: negambar <negambar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/22 09:56:30 by negambar          #+#    #+#             */
/*   Updated: 2025/10/29 15:19:48 by negambar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "library/servers.hpp"

std::vector<Channel> &Server::getChannel()
{
    return (this->allChannel);
}

void Server::setInbuf(int fd, std::string &buf)
{
    inbuf[fd] = buf;
}

void Server::setOutbuf(int fd, std::string &buf)
{
    outbuf[fd] = buf;
}

std::string         Server::getInbuf(int fd)
{
    return (inbuf[fd]);
}
std::string         Server::getOutbuf(int fd)
{
    return (outbuf[fd]);
}

void        Server::clientCleanUp(std::string &name, int fd)
{
    close(fd);
    inbuf.erase(fd);
    outbuf.erase(fd);
    name.erase(fd);
}

void       Server::setUser(User &u, int fd)
{
    u.setFd(fd);
    users[fd] = &u;
}