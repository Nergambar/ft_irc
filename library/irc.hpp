/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   irc.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: negambar <negambar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/22 09:56:41 by negambar          #+#    #+#             */
/*   Updated: 2025/11/14 14:53:19 by negambar         ###   ########.fr       */
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


int     create_serv_pocket(int _port);
int     addNickname(std::map<int,std::string>&cp, const std::string &n);
int     addWithKey(std::map<int, std::string>&copy, int key, const std::string &n);


int    handleClient(std::map<int, std::string> &inbuf, std::map<int, std::string> &outbuf,
                    std::map<int, bool> &authenticated, std::string &password, std::vector<pollfd> &pfds, 
                    std::map<int, std::string> &client_name, size_t i);



bool startswith(const std::string &s, const std::string &s2);

void pass(std::string &trimmed, int fd, std::map<int, std::string> &outbuf, std::map<int, bool> &authenticated, std::vector<pollfd> &pfds,
    std::string &password, int i);

void    checkUser(Server &serv, std::string &newUser, int fd, std::map<int,std::string> &outbuf, std::vector<pollfd> &pfds);
std::vector<std::string> ft_split(std::string str, char c);
std::vector<std::string> split2(std::string str, char c, size_t pos);

#endif