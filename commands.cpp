/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   commands.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: negambar <negambar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/22 12:07:03 by negambar          #+#    #+#             */
/*   Updated: 2025/10/22 16:25:20 by negambar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "irc.hpp"

/* 
 * fa entrare lo user nel canale se esiste, se no lo crea e rende lo user operatore
 * se l'utente e' gia' in 10 canali non fa entrare.
 */
void    User::joinChannel(std::string channel)
{
    if (serv == NULL)
    {
        std::cerr << "Error: User is not connected to a server" << std::endl;
        return ;
    }

    std::vector<Channel> &channels = serv->getChannel();

    if (this->channelPerm.find(channel) != this->channelPerm.end())
    {
        std::cerr << this->nickname << " is already in " << channel << std::endl;
        return;
    }

    Channel *it = serv->findChannel(channel);
    bool created = false;
    int lim = 0;

    if (it == NULL)
    {
        std::cerr << "No channel available called " + channel << std::endl;
        std::cerr << this->nickname << " is creating " + channel + "..." << std::endl;
        channels.push_back(Channel(channel));
        it = &channels.back();
        created = true;
        // new channel default limit expected to be valid
    }

    // now it is guaranteed non-null
    lim = it->getLimit();
    if (lim <= 0)
    {
        std::cerr << this->nickname << " tried to join " << channel << "but " << 
        "the channel is full!" << std::endl;
        return;
    }

    if (this->maxChannel == 0)
    {   // user is already in 10 channels.
        std::cerr << "Max channel per user reached! (10)" << std::endl;
        // undo creation if we created the channel but couldn't join
        if (created)
            channels.pop_back();
        return ;
    }

    if (created)
    {
        this->channelPerm[channel] = true;
        std::cout << this->nickname << " created and is operator of " << channel << std::endl;
        it->setIntAsLimit(lim - 1);
    }
    else
    {
        this->channelPerm[channel] = false;
        std::cout << this->nickname << " joined " << channel << std::endl;
        it->setIntAsLimit(lim - 1);
        if (it->getLimit() == 0)
            std::cout << channel << "'s limit reached! No more user can join!" << std::endl;
    }
    this->maxChannel--;
}

void    User::kick(User &u, std::string channel)
{
    if (serv == NULL)
    {
        return;
    }
    
    Channel *found = serv->findChannel(channel);

    if (!found)
    {
        std::cerr << "channel doesn't exist" << std::endl;
        return ;
    }

    std::map<std::string, bool>::iterator it = this->channelPerm.find(channel);
    std::map<std::string, bool>::iterator it2 = u.channelPerm.find(channel); 
    if (it == this->channelPerm.end())
    {
        std::cerr << this->nickname << " isn't in " << channel << std::endl;
        return;
    }
    else if (it2 == u.channelPerm.end())
    {
        std::cerr << u.nickname << " isn't in " << channel << std::endl;
        return;
    }
    
    // Only proceed if this user is operator in the channel
    if (it->second == true)
    {
        u.channelPerm.erase(channel);
        std::cerr << u.nickname << " has been kicked from " << channel << " by " << nickname << std::endl;
        int lim = found->getLimit();
        found->setIntAsLimit(lim + 1);
    }
    else
    {
        std::cerr << this->nickname << " is not an operator in " << channel << " and cannot kick users" << std::endl;
    }
}
