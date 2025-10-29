/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   servers.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: negambar <negambar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/22 09:56:28 by negambar          #+#    #+#             */
/*   Updated: 2025/10/29 15:36:24 by negambar         ###   ########.fr       */
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
#include <poll.h>
/* USER */
class Server;
class Channel;

class User
{
    private:
        std::string username;
        std::string nickname;
        std::string hostName;
        bool        isOperator;
        int         fd;
        std::map<std::string, bool> channelPerm;
        Server  *serv;
        std::vector<std::string> messages;
        int   maxChannel; // has to be initialised to 9

    public:
        User(); //only because i need to init maxChannel
        User(int fd);
        void        setNickname(std::string nickname);
        void        setUsername(std::string username);
        std::string getUsername() const;
        std::string getNickname() const;
        void        setServ(Server *s) {serv = s;}
        int         getFd();
        void        setFd(int fd);

        
        /* testing functions */
        User(std::string nickname);
        bool        isOper(std::string name);
        const std::map<std::string, bool>& getPermissions() const;
        
        
        /* TBF: to be fixed(?) */
        void    makeOperator(User &user, std::string channel);
        void    joinChannel(std::string channel);
        void    kick(User &u, std::string channel);
        
        void        setTopic(Channel &c, const std::string &t);
        void        setLimit(Channel &c, int l);
        void        setPw(Channel &c, const std::string &pw);
};


/* CHANNEL */


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


    
/* SERVER */


class Server{
    private:
        std::map<int,std::string> inbuf;
        std::map<int, std::string> outbuf;
        std::vector<Channel> allChannel;
        std::map<int, User*> users;
        struct pollfd np;
    public:
        /* setters and getters */
        std::vector<Channel> &getChannel();
        void                setClientName(User &u);
        void                setInbuf(int fd, std::string &buf);
        void                setOutbuf(int fd, std::string &buf);
        std::string         getInbuf(int fd);
        std::string         getOutbuf(int fd);
        void                clientCleanUp(std::string &name, int fd);
        void                setUser(User &u, int fd);

        Server() {};
        ~Server() {};
        Channel             *findChannel(std::string name);
        
};


#endif