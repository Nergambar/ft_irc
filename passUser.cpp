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
	for (size_t k = 1; k < _pfds.size(); ++k) {
		if (_pfds[k].fd != fd) {
			setOutbuf(_pfds[k].fd, join_msg);
			_pfds[k].events |= POLLOUT;
		}
	}
	return (true);
}

bool Server::pass(int fd, std::vector<std::string> cmd)
{
    std::string arg;
    
    // Check if the argument exists (cmd[1]) before trying to access it
    if (cmd.size() < 2)
        arg = "";
    else
        arg = cmd[1];

    // ********* CRITICAL FIX: TRIM TRAILING WHITESPACE/CR/LF *********
    // Find the position of the last non-whitespace/control character
    size_t end = arg.find_last_not_of(" \t\n\r");
    if (end != std::string::npos) {
        arg.resize(end + 1); // Resize the string to exclude trailing characters
    } else {
        // If the string was all whitespace/control chars, treat it as empty
        arg.clear();
    }
    // ***************************************************************
    
    // Now perform the clean comparison
    if (arg == _password)
    {
        _users[fd]->setAuth(true);
        pollfd* p = getPollfd(fd);
        if (p)
            p->events |= POLLOUT;
        // Optionally, send a success notice here
        return (true);
    }
    else
    {
        std::string s = "Incorrect password. Try again:\r\n";
        setOutbuf(fd, s);
        pollfd* p = getPollfd(fd);
        if (p)
            p->events |= POLLOUT;
        return (false);
    }
    // The original final 'return (false); //forse cambiare' from handle_command is redundant
    // if all paths return within the if/else block.
}

/* bool Server::pass(int fd, std::vector<std::string> cmd)
{
    std::string arg;
    if (cmd[1].empty())
        arg = "";
    else
        arg = cmd[1];
    if (arg == _password)
    {
        _users[fd]->setAuth(true);
		pollfd* p = getPollfd(fd);
		if (p)
		    p->events |= POLLOUT;
    }
    else
    {
        std::string s = "Incorrect password. Try again:\r\n";
        setOutbuf(fd, s);
        pollfd* p = getPollfd(fd);
        if (p)
            p->events |= POLLOUT;
		return (false);
    }
    return(true);
} */