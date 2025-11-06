/******************************************************************************/
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   irc.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: scarlucc <scarlucc@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/22 09:56:41 by negambar          #+#    #+#             */
/*   Updated: 2025/11/06 16:45:12 by scarlucc         ###   ########.fr       */
/*                                                                            */
/******************************************************************************/

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
#include <poll.h>
#include <netinet/in.h> // sockaddr_in
#include <fcntl.h>      // fcntl
#include <vector>       // vector
#include <algorithm>    // vector find
#include <sstream>      //stringstream
#include <cerrno>
#include <limits.h>     //INT_MAX e INT_MIN
#include <sys/types.h>
#include <arpa/inet.h>
#include "servers.hpp"


int     create_serv_pocket(int port);
int     addNickname(std::map<int,std::string>&cp, const std::string &n);
int     addWithKey(std::map<int, std::string>&copy, int key, const std::string &n);


bool recvLoop(int fd, Server &serv, std::map<int, std::string> &inbuf, std::map<int, std::string> &outbuf,
    std::map<int, bool> &authenticated, std::string &password, std::vector<pollfd> &pfds,  
    std::map<int, std::string> &client_name, int i);


int    handleClient(std::map<int, std::string> &inbuf, std::map<int, std::string> &outbuf,
                    std::map<int, bool> &authenticated, std::string &password, std::vector<pollfd> &pfds, 
                    std::map<int, std::string> &client_name, size_t i);



bool startswith(const std::string &s, const std::string &s2);



bool handle_command(int fd, const std::string &line,
                    std::map<int,std::string> &outbuf,
                    std::map<int,std::string> &client,
                    std::string       &server_password,
                    std::vector<struct pollfd> &pfds,
                    Server                      &serv);
void closeClient(std::map<int, std::string> &client_name, int fd, std::vector<pollfd> &pfds,
        std::map<int, std::string> &outbuf, std::map<int, std::string> &inbuf, short rev, int i);

void    readyForWrite(std::map<int, std::string> &client_name, int fd, std::vector<pollfd> &pfds,
        std::map<int, std::string> &outbuf, std::map<int, std::string> &inbuf, int i);


std::vector<std::string> ft_split(std::string str, char c);

#endif