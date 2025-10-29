/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   users.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: negambar <negambar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/16 10:28:17 by negambar          #+#    #+#             */
/*   Updated: 2025/10/29 12:54:50 by negambar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "library/irc.hpp"

User::User() : username(""), nickname(""), hostName(""), isOperator(false), fd(-1), channelPerm(), serv(NULL), messages(), maxChannel(9)
{
    messages.reserve(maxChannel);
}

User::User(int fd) : username(""), nickname(""), hostName(""), isOperator(false), fd(fd), channelPerm(), serv(NULL), messages(), maxChannel(9)
{
    messages.reserve(maxChannel);
}

int    User::getFd()
{
    return fd;
}

bool    User::isOper(std::string channel)
{
    std::map<std::string, bool>::iterator it = channelPerm.find(channel);
    if (it != channelPerm.end())
        return (it->second);
    else
    {
        std::cerr << this->nickname << " is NOT in " << channel << " ";
        return (false);
    }
}

User::User(std::string nickname) : username(""), nickname(nickname), hostName(""), isOperator(false), channelPerm(), serv(NULL), messages(), maxChannel(9)
{
    messages.reserve(maxChannel);
}

void    User::setNickname(std::string n)
{
    this->nickname = n;
}

void User::setUsername(std::string u)
{
    this->username = u;
}
/* 
 * user's nickname getter
 */
std::string User::getNickname() const
{
    return (this->nickname);
}

/* 
 * user's username getter
*/
std::string User::getUsername() const
{
    return (this->username);
}

//make operator moved to operators.cpp


void    User::setTopic(Channel &c, const std::string &t)
{
    std::map<std::string, bool>::iterator itUser1 = this->channelPerm.find(c.getName());
    Channel *channelPtr = serv->findChannel(c.getName());
    if (channelPtr == NULL)
    {
        std::cerr << "Channel " << c.getName() << " not found on server" << std::endl;
        return;
    }
    if (itUser1 != this->channelPerm.end() && itUser1->second == true)
    {
        // set the topic on the channel when the user has permission
        std::cout << this->nickname << " has set the topic of this channel to " << t << std::endl;
        channelPtr->setStringAsTopic(t);
    }   
}

void    User::setLimit(Channel &c, int l)
{
    if (l <= 0)
    {
        std::cerr << "Max users count cannot be negative or zero" << std::endl;
        return;
    }
    std::cout << this->nickname << " has set the limit of users for this channel to " << l << std::endl;
    c.setIntAsLimit(l - 1);
}

void    User::setPw(Channel &c, const std::string &pw)
{
    if (!pw.c_str())
        std::cerr << "Password cannot be NULL" << std::endl;

    std::cout << this->nickname << " has set the password of this channel to " << pw << std::endl;
    c.setStringAsPw(pw);
}

const std::map<std::string, bool>& User::getPermissions() const
{
    return (this->channelPerm);
}