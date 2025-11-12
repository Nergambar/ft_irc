/******************************************************************************/
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handleCommands.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: scarlucc <scarlucc@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/28 14:26:41 by negambar          #+#    #+#             */
/*   Updated: 2025/11/12 17:19:56 by scarlucc         ###   ########.fr       */
/*                                                                            */
/******************************************************************************/

#include "library/irc.hpp"

int setNick(std::map<int, std::string> &outbuf, int fd, std::istringstream &iss, std::map<int, std::string> &client, std::vector<pollfd> &pfds)
{
    std::string newNick;
    std::ostringstream ss;
    ss << fd;
    iss >> newNick;
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
}


/* bool handle_command(int fd, const std::string &line,
                    std::map<int,std::string> &client,
                    std::string       &server_password,
                    std::vector<struct pollfd> &pfds)
{
    char c = ' ';
	std::vector<std::string> split = split2(line, c, line.find(':'));
    (void)serv;

    if (split[0] != "JOIN" && split[0] != "NICK" && split[0] != "PASS" && split[0] != "USER")
        return (false);
    std::istringstream iss(line);
    std::string cmd;
    iss >> cmd;
    if (cmd == "NICK")
    {
        if (setNick(outbuf, fd, iss, client, pfds) == 1)
        {
            outbuf[fd].append("Nickname is already in use. Please choose another.\r\n");//ERR_NICKCOLLISION
            return false;
        }
        else
            return (true);
    }
    else if (cmd == "PASS")
    {
        std::string newpw;
        iss >> newpw;
        if (newpw.empty()) {
            outbuf[fd].append("Usage: /pass <newpassword>  (use non-empty value to set)\r\n");
        } else if (newpw == server_password) {
			outbuf[fd].append("Can't change into the same password as before.\r\n");
		}
		else {
            server_password = newpw;
            outbuf[fd].append("Server password changed.\r\n");
            // Notify other clients (optional)
            std::string notice = "Server password has been changed by " + client[fd] + ".\r\n";
            for (size_t k = 1; k < pfds.size(); ++k) {
                if (pfds[k].fd != fd) {
                    outbuf[pfds[k].fd].append(notice);
                    pfds[k].events |= POLLOUT;
                }
            }
        }
        return (true);
    }
    else if (cmd == "JOIN")
    {
        std::string channel;
        iss >> channel;
        if (channel.empty())
        {
            outbuf[fd].append("Usage: /join <channel>\r\n");
            return true;
        }
        User *u = serv.getUser(fd);
        if (!u)
        {
            outbuf[fd].append("Internal error: user not found\r\n");
            return true;
        }
        std::cout << channel << std::endl;
        if (u->getUsername().empty())
            u->setUsername("user");
        outbuf[fd].append(":"+u->getNickname() + "!" + u->getUsername() + "@" + "10.12.6.3" + " JOIN :" + channel);
        u->joinChannel(channel, serv);
        // Ensure outbuf exists and mark fd for POLLOUT so the client receives JOIN/MODE replies
        (void)outbuf[fd];
        for (size_t k = 1; k < pfds.size(); ++k)
        {
            if (pfds[k].fd == fd)
            {
                pfds[k].events |= POLLOUT;
                break;
            }
        }
        return true;
    } 
    else if (cmd == "USER")
    {
        std::string user_line = line;
        checkUser(serv, user_line, fd, outbuf, pfds);
        
        return (true);
    }
    if (cmd != "NICK" && cmd != "PASS" && cmd != "USER") // Add all recognized commands here
        outbuf[fd].append(std::string("Unknown command: ") + cmd + "\r\n");

    return (cmd == "NICK" || cmd == "PASS" || cmd == "USER");
} */