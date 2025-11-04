/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Users.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: negambar <negambar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/04 09:48:34 by negambar          #+#    #+#             */
/*   Updated: 2025/11/04 09:52:39 by negambar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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
#include <poll.h>

class User
{
    private:
        std::string username;
        std::string nickname;
        std::string hostName;
        bool        isOperator;
        int         fd;
        std::map<std::string, bool> channelPerm;
        // Server  *serv;
        std::vector<std::string> messages;
        int   maxChannel; // has to be initialised to 9

    public:
        User(); //only because i need to init maxChannel
        User(int fd);
        void        setNickname(std::string nickname);
        void        setUsername(std::string username);
        std::string getUsername() const;
        std::string getNickname() const;
        int         getFd();
        void        setFd(int fd);

        
        /* testing functions */
        User(std::string nickname);
        bool        isOper(std::string name);
        const std::map<std::string, bool>& getPermissions() const;
        
        
        /* TBF: to be fixed(?) */
        void    makeOperator(User &user, std::string channel);
        void    joinChannel(std::string channel, Server &serv);
        void    kick(User &u, std::string channel, Server &serv);
        
        void        setTopic(Channel &c, const std::string &t, Server &serv);
        void        setLimit(Channel &c, int l);
        void        setPw(Channel &c, const std::string &pw);
};