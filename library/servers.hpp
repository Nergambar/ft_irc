/******************************************************************************/
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   servers.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: scarlucc <scarlucc@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/22 09:56:28 by negambar          #+#    #+#             */
/*   Updated: 2025/11/12 17:17:51 by scarlucc         ###   ########.fr       */
/*                                                                            */
/******************************************************************************/

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

#include "channels.hpp"
#include "users.hpp"

class Server{
    private:
        std::map<int,std::string> inbuf;
        std::map<int, std::string> outbuf;
        std::vector<Channel> allChannel;
        std::map<int, User*> users;
        std::vector<pollfd> pfds;
		std::string		password;
		int				port;
        struct pollfd np;
		std::map<std::string, bool (Server::*)(int, std::vector<std::string>)> commands;		

		
    public:
        /* setters and getters */
        void                setUser(User &u, int fd);
        void                setClientName(User &u);
        void                setInbuf(int fd, std::string &buf);
        void                setOutbuf(int fd, std::string &buf);

        std::vector<Channel> 	&getChannel();
        std::string         	getInbuf(int fd);
        std::string         	getOutbuf(int fd);
        std::vector<pollfd>     getPfds() {return (pfds);};
		pollfd* 				getPollfd(int fd) {
		    for (size_t i = 0; i < pfds.size(); ++i) {
		        if (pfds[i].fd == fd)
		            return &pfds[i];
		    }
		    return NULL;
		}
		

        void                clientCleanUp(int fd, std::map<int, std::string> &client_names, 
                                std::map<int, bool> &authenticated, std::vector<pollfd> &pfds,
                                size_t i);
		void				command_map(void);

		bool				nick(int fd, std::vector<std::string>nick);
		bool				pass(int fd, std::vector<std::string> cmd);
		bool				user(int fd, std::vector<std::string> cmd);
		
        Server() {};
		Server(std::string port, std::string psw);
        ~Server() {};
        Channel             *findChannel(std::string name);
        User                *getUser(int fd);
        
        void    readyForWrite(std::map<int, std::string> &client_name, int fd, std::vector<pollfd> &pfds,
                int i);
        void closeClient(std::map<int, std::string> &client_name, int fd, std::vector<pollfd> &pfds,
            short rev, int i);
        bool recvLoop(int fd, std::map<int, std::string> &client_name, int i);
        bool    handle_command(int fd, const std::vector<std::string> &line);
};


#endif