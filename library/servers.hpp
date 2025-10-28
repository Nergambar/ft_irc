/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   servers.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: negambar <negambar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/22 09:56:28 by negambar          #+#    #+#             */
/*   Updated: 2025/10/23 13:05:09 by negambar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERS
#define SERVERS

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


class Channel;

class Channel{
    private:
        std::string name;
        std::string topic;
        int         maxUsers;
        std::string password;
    public:
        Channel();
        Channel(const std::string &n);
        std::string getName() const;
        void        setName(const std::string &n);
        std::string getTopic();
        void        setStringAsTopic(const std::string &t);
        int         getLimit();
        void        setIntAsLimit(int i);
        std::string getPw();
        void        setStringAsPw(const std::string &pw);
};

class Server{
    private:
        std::vector<Channel> allChannel;
    public:
        std::vector<Channel> &getChannel();
        Channel             *findChannel(std::string name);
};


#endif