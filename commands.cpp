/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   commands.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: negambar <negambar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/22 12:07:03 by negambar          #+#    #+#             */
/*   Updated: 2025/11/03 14:52:46 by negambar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "library/irc.hpp"

/* 
 * fa entrare lo user nel canale se esiste, se no lo crea e rende lo user operatore
 * se l'utente e' gia' in 10 canali non fa entrare.
 */
void    User::joinChannel(std::string channel)
{
    // 1. controlli base
    // controlla che sia connesso ad una istanza server
    if (serv == NULL)
    {
        std::cerr << "Error: User is not connected to a server" << std::endl;
        return;
    }

    
    //controlla se e' gia nel canale
    if (this->channelPerm.count(channel))
    {
        std::cerr << this->nickname << " is already in " << channel << std::endl;
        return;
    }
    
    
    if (this->maxChannel <= 0)
    {
        std::cerr << "Max channel per user reached! (10)" << std::endl;
        return ;
    }
    
    std::vector<Channel> &channels = serv->getChannel();
    Channel *it = serv->findChannel(channel);
    bool created = false;
    
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
    int lim = 0;
    lim = it->getLimit();
    if (lim <= 0)
    {
        std::cerr << this->nickname << " tried to join " << channel << "but " << 
        "the channel is full!" << std::endl;
        return;
    }
    this->channelPerm[channel] = created;
    if (created)
    {
        this->channelPerm[channel] = true;
        std::cout << this->nickname << " created and is operator of " << channel << std::endl;
        it->setIntAsLimit(lim - 1);
    }
    else
    {
        std::cout << this->nickname << " joined " << channel << std::endl;
        it->setIntAsLimit(lim - 1);
        if (it->getLimit() == 0)
            std::cout << channel << "'s limit reached! No more user can join!" << std::endl;
    }
    this->maxChannel--;
    // --- send JOIN notification to the client's outgoing buffer so HexChat updates UI ---
    // Format a minimal valid JOIN message: :nick!user@host JOIN <channel>\r\n
    // use available getters; fall back to "host" literal if none
    int fd = this->getFd();
    if (fd >= 0)
    {
        std::string nick = this->getNickname();
        std::string user = this->getUsername();
        std::string host = "localhost";
        // compose message
        std::string joinMsg = ":" + nick + "!" + user + "@" + host + " JOIN " + channel + "\r\n";

        // append to server out buffer for this fd
        std::string out = serv->getOutbuf(fd);
        out.append(joinMsg);

        // If the user created the channel, grant operator mode notification so client shows ops
        if (created)
        {
            // MODE message from server announcing +o to the user
            std::string modeMsg = ":" + nick + " MODE " + channel + " +o " + nick + "\r\n";
            out.append(modeMsg);
        }

        serv->setOutbuf(fd, out);
    }
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
