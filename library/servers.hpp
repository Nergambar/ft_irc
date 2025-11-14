/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   servers.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: negambar <negambar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/22 09:56:28 by negambar          #+#    #+#             */
/*   Updated: 2025/11/14 15:30:28 by negambar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERS
#define SERVERS


#include <cstdio> 		//perror
#include <cstdlib>      // atoi
#include <limits.h>     //INT_MAX e INT_MIN
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <cstring>
#include <cerrno>
#include <stdio.h>
#include <sstream>
#include <cctype>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include "channels.hpp"
#include "users.hpp"

class Server{
    private:
        std::map<int,std::string> 		_inbuf;//command o request
        std::map<int, std::string> 		_outbuf;//reply o response
        std::vector<Channel> 			_allChannel;
        std::map<int, User*> 			_users;
		struct pollfd 					_server_fd;
        std::vector<pollfd> 			_pfds;
		std::string						_password;
		int								_port;
		std::map<std::string, bool (Server::*)(int, std::vector<std::string>)> _commands;		

		
    public:
        /* setters and getters */
        void                setUser(User &u, int fd);
        void                setClientName(User &u);
        void                setInbuf(int fd, std::string &buf);
        void                setOutbuf(int fd, std::string &buf);
		

        std::vector<Channel> 	&getChannel();
        std::string&         	getInbuf(int fd);
        std::string&        	getOutbuf(int fd);
        std::vector<pollfd>     getPfds() {return (_pfds);};// returns the vector
		pollfd* 				getPollfd(int fd) // returns specific pollfd
        {
		    for (size_t i = 0; i < _pfds.size(); ++i) {
		        if (_pfds[i].fd == fd)
		            return &_pfds[i];
		    }
		    return NULL;
		}
		

        void                clientCleanUp(int fd, size_t i)
        {
            std::map<int, User*>::iterator it = _users.find(fd);
            if (it != _users.end())
                delete it->second; // serve a eliminare la classe puntata (valore della mappa)
            _users.erase(it); // serve a eliminare l'indice della mappa (anche chiamato key)
            _pfds.erase(_pfds.begin() + i);
        };
		void				command_map(void);

		bool				nick(int fd, std::vector<std::string>nick);
		bool				pass(int fd, std::vector<std::string> cmd);
		bool				user(int fd, std::vector<std::string> cmd);
		int                 set_nonblocking(int fd);
		int                 make_server_socket(int _port);
		
        Server() {};
		Server(std::string port, std::string psw);
        ~Server() {for (size_t i = 0; i < _pfds.size(); ++i) close(_pfds[i].fd);};

		void run();
		
        Channel             *findChannel(std::string name);
        User                *getUser(int fd);
        
        void    readyForWrite(int fd, int i);
        void closeClient(int fd, short rev, int i);
        bool recvLoop(int fd, int i);
        bool    handle_command(int fd, const std::vector<std::string> &line);
};

#endif