/******************************************************************************/
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   helper1.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: scarlucc <scarlucc@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/29 11:52:49 by negambar          #+#    #+#             */
/*   Updated: 2025/11/13 17:38:51 by scarlucc         ###   ########.fr       */
/*                                                                            */
/******************************************************************************/

#include "../library/servers.hpp"
#include "../library/irc.hpp"


void Server::setClientName(User &u)
{
    int fd = u.getFd();
    std::ostringstream name_os;
    name_os << "user" << fd;
    u.setNickname(name_os.str());
    users[fd] = &u;
}

bool Server::recvLoop(int fd, std::map<int, std::string> &client_name, int i, std::vector<struct pollfd> &pfds)//aggiunto temporaneamente parametro pfds
{
    bool closed = false;
    char buf[4096];
    ssize_t n = recv(fd, buf, sizeof(buf), 0);

	//debugging per segfault
	std::cout << "DEBUG: pfds.size()=" << pfds.size()
          << " i=" << i
          << " fd=" << fd << std::endl;

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

            if (line.empty())
                //continue;
				return closed;
            if (line[0] == "CAP")
                //continue;
				return closed;
                    std::string msg = "[" + client_name[fd] + "]: " + inbuf[fd] + "\r\n";
                    for (size_t k = 1; k < pfds.size(); ++k)
                    {
                        if (pfds[k].fd != fd)
                        {
                            outbuf[pfds[k].fd].append(msg);
                            pfds[k].events |= POLLOUT;
                        }
                    }
                    outbuf[fd].append("You said: " + inbuf[fd] + "\r\n");//controlla se stampa
                pfds[i].events |= POLLOUT;//segmentation fault
            
                if (!handle_command(fd, line))
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
				inbuf[fd].clear();//svuotare inbuf dopo aver gestito una riga
            //}
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

bool Server::handle_command(int fd, const std::vector<std::string> &line)
{
	if (commands.find(line[0]) != commands.end())
	{
		return (this->*(commands[line[0]]))(fd, line);
	}
	else
	{
		std::string tmp = "temporary error: " + line[0] + "comando sconosciuto";
		setOutbuf(fd, tmp);
		return (false);
	}
	return (false); //forse cambiare	
}


/*
bool Server::handle_command(int fd, const std::vector<std::string> &line,
                    std::map<int,std::string> &client,
                    std::string       &server_password,
                    std::vector<struct pollfd> &pfds)
{
    if (line[0] != "JOIN" && line[0] != "NICK" && line[0] != "PASS" && line[0] != "USER")
        return (false);
    if (line[0] == "NICK")
    {
        if (setNick(outbuf, fd, iss, client, pfds) == 1)
        {
            outbuf[fd].append("Nickname is already in use. Please choose another.\r\n");//ERR_NICKCOLLISION
            return false;
        }
        else
            return (true);
    }
    else if (line[0] == "PASS")
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
    else if (line[0] == "USER")
    {
        std::string user_line = line[0];
        checkUser(serv, user_line, fd, outbuf, pfds);
        
        return (true);
    }
    if (line[0] != "NICK" && line[0] != "PASS" && line[0] != "USER") // Add all recognized commands here
        outbuf[fd].append(std::string("Unknown command: ") + line[0] + "\r\n");

    return (line[0] == "NICK" || line[0] == "PASS" || line[0] == "USER");
}*/