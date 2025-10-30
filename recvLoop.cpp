/******************************************************************************/
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   recvLoop.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: scarlucc <scarlucc@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/28 11:35:35 by negambar          #+#    #+#             */
/*   Updated: 2025/10/30 14:07:26 by scarlucc         ###   ########.fr       */
/*                                                                            */
/******************************************************************************/

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

void enterPw(std::string &trimmed, int fd, std::map<int, std::string> &outbuf, std::map<int, bool> &authenticated, std::vector<pollfd> &pfds,
    std::string &password, int i)
{
    trimmed = trimmed.substr(5, trimmed.length() - 5);
    if (trimmed == password) {
        authenticated[fd] = true;

        pfds[i].events |= POLLOUT;
        // Broadcast join now that this client is authenticated
    } else {
        outbuf[fd].append("Incorrect password. Try again:\r\n");
        }
}


bool recvLoop(int fd, std::map<int, std::string> &inbuf, std::map<int, std::string> &outbuf,
    std::map<int, bool> &authenticated, std::string &password, std::vector<pollfd> &pfds, 
    std::map<int, std::string> &client_name, int i)
{
    bool closed = false;
	char buf[4096];
	ssize_t n = recv(fd, buf, sizeof(buf), 0);
	if (n > 0) {
		inbuf[fd].append(buf, buf + n);

            // 🔍 stampa quello che è arrivato
            std::string received(buf, n); // crea stringa dai bytes ricevuti

			// Debug: stampa in modo leggibile CR (\r) / LF(\n)
			/* std::string printable;
			for (size_t j = 0; j < received.size(); ++j) {
				char c = received[j];
				if (c == '\r') printable += "\\r";
				else if (c == '\n') printable += "\\n";
				else printable += c;
			} */

			bool crlf_only = (received.find_first_not_of("\r\n") == std::string::npos);//controllo Neri 30/10/2025

			// Append the chunk only if it's not pure CR/LF, OR if we already have partial data
			// for this fd (a lone '\n' arriving can complete a pending line).
			if (crlf_only) {  //controllo Neri 30/10/2025
				return (false);
			}
			
            std::cout << "[RECV fd=" << fd << "] " << received << std::endl;
			
            size_t pos;
            while ((pos = inbuf[fd].find("\r\n")) != std::string::npos) {
                std::string line = inbuf[fd].substr(0, pos + 2);
                inbuf[fd].erase(0, pos + 2);
                
                std::string trimmed = line;

				if (trimmed.size() >= 2 && trimmed.substr(trimmed.size() - 2) == "\r\n")
					trimmed.erase(trimmed.size() - 2);

				while (!trimmed.empty() && (trimmed[trimmed.size() - 1] == '\n' || trimmed[trimmed.size() - 1] == '\r'))
        			trimmed.erase(trimmed.size()-1, 1);

				if (trimmed.empty())
					continue;

			if (!authenticated[fd] && !password.empty()) {
				enterPw(trimmed, fd, outbuf, authenticated, pfds, password, i); //INSERIMENTO PASSWORD
				pfds[i].events |= POLLOUT;
			} 
			else if (authenticated[fd] && client_name[fd].find("user") == 0) {
				// The client has authenticated (or didn't need a password)
				// and still has the default "userN" name.
				
				std::string newNick = trimmed;
				if (newNick.empty()) {
					outbuf[fd].append("Nickname cannot be empty. Please choose your nickname:\r\n");
				} else {
					// Simple check if the nickname is already taken (case-sensitive)
					checkNick(client_name, newNick, fd, outbuf, pfds);
				}
				pfds[i].events |= POLLOUT;

			}
			else {
				if (handle_command(fd, trimmed, outbuf, client_name, password, pfds)) {
					// A command was handled; make sure sender gets any response queued by handle_command
					pfds[i].events |= POLLOUT;
				} 
				else {
					// Remove trailing CR/LF
					while (!trimmed.empty() && (trimmed[trimmed.size()-1] == '\n' || trimmed[trimmed.size()-1] == '\r'))
						trimmed.erase(trimmed.size()-1, 1);

					std::string message_to_broadcast = "[" + client_name[fd] + "]: " + trimmed + "\r\n";

					// Queue message to all *other* connected clients
					for (size_t k = 1; k < pfds.size(); ++k) {
						if (pfds[k].fd != fd) {
							outbuf[pfds[k].fd].append(message_to_broadcast);
							pfds[k].events |= POLLOUT;
						}
					}
					// Echo back to sender
					outbuf[fd].append("You said: " + trimmed + "\r\n");
					pfds[i].events |= POLLOUT;
				}
			}
		}
	} else if (n == 0) {
		std::cout << "Client " << client_name[fd] << " fd=" << fd << " disconnected (recv==0)\n";
		closed = true;
	} else {
		std::cerr << "recv" << std::endl;
		closed = true;
	}
    return (closed);
}