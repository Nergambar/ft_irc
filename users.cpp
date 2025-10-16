/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   users.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: negambar <negambar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/16 10:28:17 by negambar          #+#    #+#             */
/*   Updated: 2025/10/16 11:19:46 by negambar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "irc.hpp"


std::string User::getNickname() const
{
    return (this->nickname);
}

std::string User::getUsername() const
{
    return (this->username);
}

void    User::makeOperator(User &u, std::string channel)
{
    std::map<std::string, bool>::iterator itUser1 = this->channelPerm.begin();
    std::map<std::string, bool>::iterator itUser2 = u.channelPerm.begin();
    itUser1 = this->channelPerm.find(channel);
    itUser2 = u.channelPerm.find(channel);

    if (itUser1 == this->channelPerm.end())
    {
        perror("no channel available for user1");
        return;
    }
    if (itUser2 == u.channelPerm.end())
    {
        perror("no channel available for user2");
        return;
    }

    // check that "this" has the permit to make u an operator
    if (itUser1->second == false)
    {
        std::string msg = nickname + "is not an operator in the channel";
        perror(msg.c_str());
    }

    // grant operator flag in u's channel permissions if not already set
    if (u.isOperator == false && itUser2->second == false)
    {
        itUser2->second = true;
        std::string msg = u.nickname + "has been made into an operator!";
        std::cout << msg << std::endl;
    }
}