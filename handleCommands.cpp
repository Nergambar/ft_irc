/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handleCommands.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: negambar <negambar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/28 14:26:41 by negambar          #+#    #+#             */
/*   Updated: 2025/10/28 14:33:00 by negambar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "library/irc.hpp"


bool startswith(const std::string &s, const std::string &s2)
{
    if (s2.size() > s.size()) return false;
    return s.compare(0, s2.size(), s2) == 0;
}

bool handle_command(int fd, const std::string &line,
                    std::map<int,std::string> &outbuf,
                    std::map<int,std::string> &client,
                    std::string       &server_password,
                    std::vector<struct pollfd> &pfds)
{
    if (line.empty() || (line[0] != '/' && !startswith(line, "PASS") && !startswith(line, "NICK")))
        return (false);
    std::istringstream iss(line);
    std::string cmd;
    iss >> cmd;

    if (cmd == "/nick" || cmd == "NICK")
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
            outbuf[fd].append("Nickname '"+ newNick + "' is already in use. Please choose another.\r\n");//ERR_NICKCOLLISION
            return (true);
        } else {
            if (newNick.empty())
                outbuf[fd].append("Usage: /nick <name> OR NICK <name>\n");
            else{
                std::string old;
                if (client[fd].empty())
                {
                    old = "user" + ss.str();
                }
                else
                    old = client[fd];
                client[fd] = newNick;
                std::string msg = old + " is now " + newNick + "\r\n";
                for (size_t k = 1; k < pfds.size(); ++k){
                    if (pfds[k].fd != fd){
                        outbuf[pfds[k].fd].append(msg);
                        pfds[k].events |= POLLOUT;
                    }
                }
                outbuf[fd].append("You are now "+ newNick + "\r\n");
            }
            return (true);
        }
    }
    else if (cmd == "/pass" || cmd == "PASS")
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
    outbuf[fd].append(std::string("Unknown command: ") + cmd + "\r\n");
    return true;
}