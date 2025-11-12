/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   servers.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: negambar <negambar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/22 09:56:30 by negambar          #+#    #+#             */
/*   Updated: 2025/11/12 16:35:34 by negambar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../library/servers.hpp"

Server::Server(std::string port, std::string psw)
{
	this->port = std::atoi(port.c_str());
	password = psw;
	command_map();
	// outbuf;
	// inbuf;
}

bool	Server::nick(int fd, std::vector<std::string> vect)
{
	// bool is_taken = false;
	std::string nick = this->users[fd]->getNickname();
    for (std::map<int, User *>::iterator it = users.begin(); it != users.end(); ++it)
	{
		if (it->first != fd && it->second->getNickname() == vect[1])
		{
			
			return (false);
		}
	}
	
	// for (std::map<int, std::string>::iterator it = .begin(); it != client_name.end(); ++it) {
    //     if (it->first != fd && it->second == newNick) {
    //         is_taken = true;
    //         break;
    //     }
    // }
    return (true);
}

void Server::command_map()
{
	commands["NICK"] = &Server::nick;
	commands["PASS"] = &Server::pass;
	commands["USER"] = &Server::user;
}

/* void    checkNick(std::map<int, std::string> &client_name, std::string &newNick, int fd, std::map<int, std::string> &outbuf,
        std::vector<pollfd> &pfds)
{
    bool is_taken = false;
    for (std::map<int, std::string>::iterator it = client_name.begin(); it != client_name.end(); ++it) {
        if (it->first != fd && it->second == newNick) {
            is_taken = true;
            break;
        }
    }

    if (is_taken) {
        outbuf[fd].append("Nickname '"+ newNick + "' is already in use. Please choose another:\r\n");
    } else {
        std::string oldNick = client_name[fd];
        client_name[fd] = newNick;
        outbuf[fd].append("You are now known as " + newNick + ".\r\n");
        (void)pfds;
        // Broadcast join message now that the nickname is set
        std::string join_msg = newNick + " joined the chat.\r\n";
        for (size_t k = 1; k < pfds.size(); ++k) {
            if (pfds[k].fd != fd) {
                outbuf[pfds[k].fd].append(join_msg);
                pfds[k].events |= POLLOUT;
            }
        }
    }
}

int Server::setNick(std::map<int, std::string> &outbuf, int fd, std::string &iss, std::map<int, std::string> &client, std::vector<pollfd> &pfds)
{
    std::string newNick;
    std::ostringstream ss;
    ss << fd;
    newNick = iss;
    bool is_taken = false;
    for (std::map<int, std::string>::iterator it = client.begin(); it != client.end(); ++it) {
        if (it->first != fd && it->second == newNick) {
            is_taken = true;
            break;
        }
    }
    if (is_taken) {
        // outbuf[fd].append("Nickname '"+ newNick + "' is already in use. Please choose another.\r\n");//ERR_NICKCOLLISION
        return (1);
    } else {
        if (newNick.empty())
            outbuf[fd].append("Usage: NICK <name>\n");
        else{
            std::string old;
            if (client[fd].empty())
            {
                old = "user" + ss.str();
            }
            else
                old = client[fd];
            std::string oldNick = client[fd];
            client[fd] = newNick;
            std::string msg = old + " is now " + newNick + "\r\n";
            for (size_t k = 1; k < pfds.size(); ++k){
                if (pfds[k].fd != fd){
                    outbuf[pfds[k].fd].append(msg);
                    pfds[k].events |= POLLOUT;
                }
            }
            // outbuf[fd].append(":" + oldNick + " NICK :" + newNick);
            outbuf[fd].append("You are now "+ newNick + "\r\n");
        }
    }
    return (0);
} */

std::vector<Channel> &Server::getChannel()
{
    return (this->allChannel);
}

void Server::setInbuf(int fd, std::string &buf)
{
    inbuf[fd] = buf;
}

void Server::setOutbuf(int fd, std::string &buf)
{
    outbuf[fd] = buf;
}

std::string         Server::getInbuf(int fd)
{
    return (inbuf[fd]);
}
std::string         Server::getOutbuf(int fd)
{
    return (outbuf[fd]);
}

void        Server::clientCleanUp(int fd, std::map<int, std::string> &client_names,
                                  std::map<int, bool> &authenticated,
                                  std::vector<pollfd> &pfds, size_t idx)
{
    // close socket
    close(fd);

    // erase buffers and client records stored in the Server
    inbuf.erase(fd);
    outbuf.erase(fd);

    // erase external maps passed by caller
    client_names.erase(fd);
    authenticated.erase(fd);

    // erase user pointer if present
    users.erase(fd);

    // remove pollfd entry if index still valid
    if (idx < pfds.size())
        pfds.erase(pfds.begin() + idx);
}

void       Server::setUser(User &u, int fd)
{
    u.setFd(fd);
    users[fd] = &u;
}

User        *Server::getUser(int fd)
{
    std::map<int, User *>::iterator it = users.find(fd);
    if (it != users.end())
        return (it->second);
    return NULL;
}