/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: negambar <negambar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/29 12:29:55 by negambar          #+#    #+#             */
/*   Updated: 2025/11/14 14:51:04 by negambar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "library/servers.hpp"


Channel::Channel() : name(""), topic(""), maxUsers(100)
{}

Channel::Channel(const std::string &name) : name(name), topic(""), maxUsers(100)
{}

std::string Channel::getName() const
{
    (void)maxUsers;
    return (name);
}

void    Channel::setName(const std::string &n)
{
    name = n;
}

Channel     *Server::findChannel(std::string name)
{
    for (std::vector<Channel>::iterator it = _allChannel.begin(); it != _allChannel.end(); ++it)
    {
        if (it->getName() == name)
            return &(*it);
    }
    return (NULL);
}


void        Channel::setStringAsTopic(const std::string &t)
{
    topic = t;
}

std::string Channel::getTopic()
{
    return topic;
}

void        Channel::setIntAsLimit(int i)
{
    maxUsers = i;
}

int         Channel::getLimit()
{
    return (maxUsers);
}

void       Channel::setStringAsPw(const std::string &pw)
{
    _password = pw;
}

std::string Channel::getPw()
{
    return (_password);
}