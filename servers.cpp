/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   servers.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: negambar <negambar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/22 09:56:30 by negambar          #+#    #+#             */
/*   Updated: 2025/11/03 15:44:09 by negambar         ###   ########.fr       */
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

void        Server::clientCleanUp(int fd, std::map<int, std::string> &client_names,
                                  std::map<int, bool> &authenticated,
                                  std::vector<pollfd> &pfds, size_t idx)
{
    // close socket
    close(fd);

    // erase buffers and client records stored in the Server
    inbuf.erase(fd);
    outbuf.erase(fd);

    // erase external maps passed by caller
    client_names.erase(fd);
    authenticated.erase(fd);

    // erase user pointer if present
    users.erase(fd);

    // remove pollfd entry if index still valid
    if (idx < pfds.size())
        pfds.erase(pfds.begin() + idx);
}

void       Server::setUser(User &u, int fd)
{
    u.setFd(fd);
    users[fd] = &u;
}

User        *Server::getUser(int fd)
{
    std::map<int, User *>::iterator it = users.find(fd);
    if (it != users.end())
        return (it->second);
    return NULL;
}