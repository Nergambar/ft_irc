/******************************************************************************/
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   login.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: scarlucc <scarlucc@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/06 12:57:54 by scarlucc          #+#    #+#             */
/*   Updated: 2025/11/06 17:29:43 by scarlucc         ###   ########.fr       */
/*                                                                            */
/******************************************************************************/

#include "library/irc.hpp"

bool recvLoop2(int fd, Server &serv, std::map<int, std::string> &inbuf, std::map<int, std::string> &outbuf,
              std::map<int, bool> &authenticated, std::string &password, std::vector<pollfd> &pfds,
              std::map<int, std::string> &client_name, int i)
{
	bool closed = false;
    char buf[4096];
    ssize_t n = recv(fd, buf, sizeof(buf), 0);

	if (n > 0)
	{
		
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