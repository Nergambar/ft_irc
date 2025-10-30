/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   recvLoop.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: negambar <negambar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/28 11:35:35 by negambar          #+#    #+#             */
/*   Updated: 2025/10/30 16:02:04 by negambar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "library/irc.hpp"

void    checkNick(std::map<int, std::string> &client_name, std::string &newNick, int fd, std::map<int, std::string> &outbuf,
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

// ...existing code...
void enterPw(std::string &trimmed, int fd, std::map<int, std::string> &outbuf, std::map<int, bool> &authenticated, std::vector<pollfd> &pfds,
    std::string &password, int i)
{
    // Safely extract the token after "PASS "
    std::string arg;
    size_t sp = trimmed.find(' ');
    if (sp == std::string::npos) {
        arg = ""; // no password supplied
    } else {
        arg = trimmed.substr(sp + 1);
    }

    if (arg == password) {
        authenticated[fd] = true;
        pfds[i].events |= POLLOUT;
    } else {
        outbuf[fd].append("Incorrect password. Try again:\r\n");
    }
}
// ...existing code...


bool recvLoop(int fd, std::map<int, std::string> &inbuf, std::map<int, std::string> &outbuf,
              std::map<int, bool> &authenticated, std::string &password, std::vector<pollfd> &pfds,
              std::map<int, std::string> &client_name, int i)
{
    bool closed = false;
    char buf[4096];
    ssize_t n = recv(fd, buf, sizeof(buf), 0);

    if (n > 0)
    {
        std::string received(buf, n);

        // Detect if only CR/LF characters were received
        bool crlf_only = (received.find_first_not_of("\r\n") == std::string::npos);
        bool sent_crlf = false;
		if (inbuf[fd] == "\r\n")
			sent_crlf = true;

        // Skip pure CR/LF unless there's existing partial data
        if (crlf_only && sent_crlf)
            return false;

        std::cout << "[RECV fd=" << fd << "] " << received << std::endl;
        inbuf[fd].append(received);

        size_t pos;
        while ((pos = inbuf[fd].find('\n')) != std::string::npos)
        {
            std::string line = inbuf[fd].substr(0, pos + 1);
            inbuf[fd].erase(0, pos + 1);

            // Trim trailing CR/LF safely
            while (!line.empty() && (line[line.size() - 1] == '\n' || line[line.size() - 1] == '\r'))
            {
                line.resize(line.size() -1);
            }

            if (line.empty())
                continue;

            // === Authentication & nickname logic ===
            if (!authenticated[fd] && !password.empty())
            {
                enterPw(line, fd, outbuf, authenticated, pfds, password, i);
                pfds[i].events |= POLLOUT;
            }
            else if (authenticated[fd] && client_name[fd].find("user") == 0)
            {
                if (line.empty())
                {
                    outbuf[fd].append("Nickname cannot be empty. Please choose your nickname:\r\n");
                }
                else
                {
                    checkNick(client_name, line, fd, outbuf, pfds);
                }
                pfds[i].events |= POLLOUT;
            }
            else
            {
                // Handle commands
                if (!handle_command(fd, line, outbuf, client_name, password, pfds))
                {
                    // Broadcast message
                    std::string msg = "[" + client_name[fd] + "]: " + line + "\r\n";

                    for (size_t k = 1; k < pfds.size(); ++k)
                    {
                        if (pfds[k].fd != fd)
                        {
                            outbuf[pfds[k].fd].append(msg);
                            pfds[k].events |= POLLOUT;
                        }
                    }

                    // Echo to sender
                    outbuf[fd].append("You said: " + line + "\r\n");
                    pfds[i].events |= POLLOUT;
                }
                else
                {
                    // Command handled, make sure POLLOUT is set
                    pfds[i].events |= POLLOUT;
                }
            }
        }
    }
    else if (n == 0)
    {
        std::cout << "Client " << client_name[fd] << " fd=" << fd << " disconnected (recv==0)\n";
        closed = true;
    }
    else
    {
        std::cerr << "recv error\n";
        closed = true;
    }

    return closed;
}
