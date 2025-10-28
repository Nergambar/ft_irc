/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   operators.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: negambar <negambar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/22 10:06:12 by negambar          #+#    #+#             */
/*   Updated: 2025/10/28 14:32:30 by negambar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "library/irc.hpp"

/* 
 * se l'utente che usa il comando e' operatore di quel canale, rende il secondo user tale.
 * se il primo utente non e' operatore del canale non succede nulla
*/
void    User::makeOperator(User &u, std::string channel)
{
    if (this->serv == NULL)
    {
        std::cerr << "no server for " + this->nickname << std::endl;
        return;
    }
    if (u.serv == NULL)
    {
        std::cerr << "no server for " + u.nickname << std::endl;
        return;
    }
    
    std::map<std::string, bool>::iterator itUser1 = this->channelPerm.find(channel);
    std::map<std::string, bool>::iterator itUser2 = u.channelPerm.find(channel);

    if (itUser1 == this->channelPerm.end())
    {
        std::cerr << "no channel available for " + this->nickname << std::endl;
        return;
    }
    if (itUser2 == u.channelPerm.end())
    {
        std::cerr << "no channel available for " + u.nickname << std::endl;
        return;
    }

    // check that "this" has the permit to make u an operator
    if (itUser1->second == false)
    {
        std::cerr << nickname + " is not an operator in the channel" << std::endl;
        return;
    }

    // grant operator flag in u's channel permissions if not already set
    if (u.isOperator == false && itUser2->second == false)
    {
        itUser2->second = true;
        u.isOperator = true;
        std::string msg = u.nickname + " has been made into an operator by " + this->nickname + "!";
        std::cerr << msg << std::endl;
    }
}

