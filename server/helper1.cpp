/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   helper1.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: negambar <negambar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/29 11:52:49 by negambar          #+#    #+#             */
/*   Updated: 2025/11/14 15:51:55 by negambar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../library/servers.hpp"
#include "../library/irc.hpp"


void Server::setClientName(User &u)
{
    int fd = u.getFd();
    std::ostringstream name_os;
    name_os << "user" << fd;
    u.setNickname(name_os.str());
    _users[fd] = &u;
}

bool Server::recvLoop(int fd, int i)
{
    bool closed = false;
    char buf[4096];
    ssize_t n = recv(fd, buf, sizeof(buf), 0);

    // Debugging (optional, but good practice)
    std::cout << "DEBUG: pfds.size()=" << _pfds.size()
              << " i=" << i
              << " fd=" << fd << std::endl;

    if (n > 0)
    {
        std::string received(buf, n);
        
        // **CRITICAL CHANGE:** REMOVE the line that adds "\r\n" if not present.
        // It should be the client's responsibility to send a well-formed IRC command.
        // if (!(received.find("\r") == received.size() - 2 && received.find("\n") == received.size() - 1))
        //     received.append("\r\n");
        
        // Append raw received data to the input buffer
        _inbuf[fd].append(received);

        size_t pos;
        // **CRITICAL CHANGE:** Use a while loop to process all complete lines received in one recv call
        while ((pos = _inbuf[fd].find('\n')) != std::string::npos)
        {
            // Extract the full line (including the terminator)
            std::string full_line = _inbuf[fd].substr(0, pos + 1);

            // 1. Parse the command line into tokens
            // The split function should handle parsing the extracted line, not the entire _inbuf.
            // Assuming split2 is designed to use the first argument as the string to split:
            // NOTE: You used _inbuf[fd].find(':') which is dangerous if the line doesn't contain ':'.
            // For now, let's assume split2 works correctly on the command and arguments.
            std::vector<std::string> cmd_tokens = split2(full_line, ' ', full_line.find(':')); 

            // 2. Remove the processed line from the input buffer
            _inbuf[fd].erase(0, pos + 1);
            
            // 3. Handle empty lines or CAP commands immediately
            if (cmd_tokens.empty() || cmd_tokens[0] == "CAP")
                continue; // Process the next line if available

            // 4. Execute the command
            // We use a temporary string of the command arguments for the echo back.
            // This is safer than trying to rebuild the message from cmd_tokens later.
            std::string cmd_args = full_line.substr(cmd_tokens[0].length());
            cmd_args.erase(cmd_args.find_last_not_of(" \r\n") + 1); // Trim trailing whitespace/CRLF

            if (!handle_command(fd, cmd_tokens))
            {
                // Command failed (e.g., PASS failed) or was unknown.
                // The error message (e.g., "Incorrect password...") is already set in _outbuf[fd] 
                // inside handle_command or Server::pass, so we only need to echo the input.
                
                // Echo only the client's input (the command/line they sent)
                _outbuf[fd].append("You said: " + cmd_args + "\r\n");
            }
            // else: Command succeeded. Output (if any) was handled by the command's specific function.
            
            // In either case, ensure the POLLOUT flag is set to send any queued output
            _pfds[i].events |= POLLOUT;

            // **CRITICAL CHANGE:** REMOVE all the chat message broadcasting logic 
            // inside this function. That logic belongs in a separate command (like PRIVMSG) 
            // or in a dedicated "handle_message" block if the input is not a command.
        }
    }
    else if (n == 0)
    {
        // Client disconnected gracefully
        std::map<int, User*>::iterator it = _users.find(fd);
        std::string fdit = (it != _users.end() && it->second) ? it->second->getNickname() : "Unknown";
		std::cout << "Client " << fdit << " fd=" << fd << " disconnected (recv==0)\n";
        closed = true;
    }
    else
    {
        // Recv error
        std::cerr << "recv error\n";
        closed = true;
    }

    return closed;
}

bool Server::handle_command(int fd, const std::vector<std::string> &line)
{
	if (_commands.find(line[0]) != _commands.end())
	{
        std::cout << "ENTRA" << std::endl;
		return (this->*(_commands[line[0]]))(fd, line);
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