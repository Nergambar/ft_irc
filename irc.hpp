/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   irc.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: negambar <negambar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/22 09:56:41 by negambar          #+#    #+#             */
/*   Updated: 2025/10/22 15:56:40 by negambar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef IRC_HPP
#define IRC_HPP

#include <iostream>
#include <map>
#include <string>
#include <cstdio> 		//perror
#include <cstdlib>      // atoi
#include <cstring>      // memset
#include <unistd.h>     // close
#include <sys/socket.h> // socket, bind, listen, accept
#include <netinet/in.h> // sockaddr_in
#include <fcntl.h>      // fcntl
#include <vector>       // vector
#include <algorithm>    // vector find
#include <sstream>      //stringstream
#include <limits.h>     //INT_MAX e INT_MIN

#include "servers.hpp"

class User
{
    private:
        std::string username;
        std::string nickname;
        std::string hostName;
        bool        isOperator;
        std::map<std::string, bool> channelPerm;
        // std::vector<std::string> channelAll;
        Server  *serv;
        std::vector<std::string> messages;
        int   maxChannel; // has to be initialised to 9

    public:
        User(); //only because i need to init maxChannel
        std::string getUsername() const;
        std::string getNickname() const;
        
        void    makeOperator(User &user, std::string channel);
        void    joinChannel(std::string channel);
        void    kick(User &u, std::string channel);

        void        setTopic(Channel &c, const std::string &t);
        void        setLimit(Channel &c, int l);
        void        setPw(Channel &c, const std::string &pw);

        /* testing functions */
        User(std::string nickname);
        bool        isOper(std::string name);
        void        setNickname(std::string nickname);
        void        setUsername(std::string username);
        const std::map<std::string, bool>& getPermissions() const;
        void        setServ(Server *s) {serv = s;}
};

int     create_serv_pocket(int port);
int     addNickname(std::map<int,std::string>&cp, const std::string &n);
int     addWithKey(std::map<int, std::string>&copy, int key, const std::string &n);

#endif