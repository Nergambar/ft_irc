/******************************************************************************/
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   recvLoop.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: scarlucc <scarlucc@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/28 11:35:35 by negambar          #+#    #+#             */
/*   Updated: 2025/11/12 15:53:03:02 by scarlucc         ###   ########.fr       */
/*                                                                            */
/******************************************************************************/

  #include "library/irc.hpp"

bool    Server::user(int fd, std::vector<std::string> cmd)
{
	std::string newUser = cmd[1];
    User    *user_obj = getUser(fd);
    if (!user_obj)
        return false;//utente viene registrato quando si collega con nome temporaneo

    std::string oldUser = user_obj->getUsername();
    user_obj->setUsername(newUser);
	std::string tmp = "You are now " + newUser + "\r\n";
    setOutbuf(fd, tmp);
    std::string join_msg = newUser + " joined the chat.\r\n";
	for (size_t k = 1; k < pfds.size(); ++k) {
		if (pfds[k].fd != fd) {
			setOutbuf(pfds[k].fd, join_msg);
			pfds[k].events |= POLLOUT;
		}
	}
	return (true);
}

//ora in server.cpp
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
} */

bool Server::pass(int fd, std::vector<std::string> cmd)
{
    std::string arg;
    if (cmd[1].empty())
        arg = "";
    else
        arg = cmd[1];
    if (arg == password)
    {
        users[fd]->setAuth(true);
		pollfd* p = getPollfd(fd);
		if (p)
		    p->events |= POLLOUT;
    }
    else
    {
        outbuf[fd].append("Incorrect password. Try again:\r\n");
        pollfd* p = getPollfd(fd);
        if (p)
            p->events |= POLLOUT;
		return (false);
    }
    return(true);
}

/* 

bool passVecchia(std::string &trimmed, int fd, std::map<int, std::string> &outbuf, std::map<int, bool> &authenticated, std::vector<pollfd> &pfds,
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

bool Server::recvLoop(int fd, Server &serv, std::map<int, bool> &authenticated, std::string &password, std::vector<pollfd> &pfds,
              std::map<int, std::string> &client_name, int i)
{
    bool closed = false;
    char buf[4096];
    ssize_t n = recv(fd, buf, sizeof(buf), 0);

    if (n > 0)
    {
        std::string received(buf, n);
        // Detect if only CR/LF characters were received
        bool crlf_only = false;
		if (received == "\r\n")
			crlf_only = true;
        bool sent_crlf = false;
		if (inbuf[fd] == "\r\n")
			sent_crlf = true;

        // Skip pure CR/LF unless there's existing partial data
        if (!sent_crlf && crlf_only)
            return false;

		if (!(received.find("\r") == received.size() - 2 && received.find("\n") == received.size() - 1))
			received.append("\r\n");
        std::cout << "[RECV fd=" << fd << "] " << received << std::endl;
        inbuf[fd].append(received);

        size_t pos;
        if ((pos = inbuf[fd].find('\n')) != std::string::npos)//finche' a capo esiste nella stringa
        {
            //std::string line = inbuf[fd].substr(0, pos + 1);
			std::vector<std::string> line = split2(inbuf[fd], ' ', inbuf[fd].find(':'));
            inbuf[fd].erase(0, pos + 1);

            // Trim trailing CR/LF safely
			// std::string lastLine = line[line.size() - 1];
            // while (!line.empty() && ((lastLine.end() - 1) == '\n' || (lastLine.end() - 1) == '\r'))
            // {
            //     line.resize(line.size() -1);
            // }

            if (line.empty())
                //continue;
				return closed;
            if (line[0] == "CAP")
                //continue;
				return closed;

            // === Authentication & nickname logic ===
            if (!authenticated[fd])
            {
                pass(line[1], fd, outbuf, authenticated, pfds, password, i);
                pfds[i].events |= POLLOUT;
            }
            else if (authenticated[fd])
            {
                // Unified command handling: let handle_command deal with NICK/PASS/JOIN/etc.
                if (!handle_command(fd, line, outbuf, client_name, password, pfds, serv))
                {
                    // Not a recognized command -> broadcast message
                    std::string msg = "[" + client_name[fd] + "]: " + inbuf[fd] + "\r\n";
                    for (size_t k = 1; k < pfds.size(); ++k)
                    {
                        if (pfds[k].fd != fd)
                        {
                            outbuf[pfds[k].fd].append(msg);
                            pfds[k].events |= POLLOUT;
                        }
                    }
                    outbuf[fd].append("You said: " + inbuf[fd] + "\r\n");
                }
                pfds[i].events |= POLLOUT;
            }
            else if (authenticated[fd] && serv.getUser(fd)->getUsername().empty())
            {
                if (line.empty())
                { if (pfds.size() > 0 && (pfds[0].revents & POLLIN)) {
			struct sockaddr_in cli_addr;
			socklen_t cli_len = sizeof(cli_addr);
			int client_fd = accept(server_fd, (struct sockaddr*)&cli_addr, &cli_len);
			if (client_fd < 0) {
				perror("accept");
			}

			if (set_nonblocking(client_fd) < 0) {
				perror("set_nonblocking(client)");
				close(client_fd);
				continue;
			}

			// Add to pfds vector
			struct pollfd np;
			np.fd = client_fd;
			User u;
                    outbuf[fd].append("User cannot be empty. Please choose your nickname:\r\n");
                }
                else
                    checkUser(serv, line, fd, outbuf, pfds);
                pfds[i].events |= POLLOUT;
            } 
            else
            {
                // Handle commands
                if (!handle_command(fd, line, outbuf, client_name, password, pfds, serv))
                {
                    // Broadcast message
                    std::string msg = "[" + client_name[fd] + "]: " + inbuf[fd] + "\r\n";

                    for (size_t k = 1; k < pfds.size(); ++k)
                    {
                        if (pfds[k].fd != fd)
                        {
                            outbuf[pfds[k].fd].append(msg);
                            pfds[k].events |= POLLOUT;
                        }
                    }

                    // Echo to sender
                    outbuf[fd].append("You said: " + inbuf[fd] + "\r\n");
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
        //togliere fd da vettore di pollfd per evitare ciclo infinito
		//al momento non gestiamo disconnessione
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
 */