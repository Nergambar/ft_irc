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

class User
{
    private:
        std::string username;
        std::string nickname;
        std::string hostName;
        bool        isOperator;
        std::map<std::string, bool> channelPerm;
        // int   maxChannel;

    public:
        std::string getUsername() const;
        std::string getNickname() const;

        void    makeOperator(User &user, std::string channel);
};

int     create_serv_pocket(int port);
int     addNickname(std::map<int,std::string>&cp, const std::string &n);
int     addWithKey(std::map<int, std::string>&copy, int key, const std::string &n);

#endif