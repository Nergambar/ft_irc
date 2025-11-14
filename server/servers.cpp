/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   servers.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: negambar <negambar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/22 09:56:30 by negambar          #+#    #+#             */
/*   Updated: 2025/11/14 15:30:28 by negambar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../library/servers.hpp"

void    Server::run()
{
    while (true) {
        int indef = -1;
        int rv = poll(&_pfds[0], _pfds.size(), indef); //aspetta indefinitivamente
        if (rv < 0) {
            if (errno == EINTR) continue;
            perror("poll");
            break;
        }
        if (rv == 0) continue;

        // ---- 1) Handle new connections: pfds[0] is the listening socket ----
        if (_pfds.size() > 0 && (_pfds[0].revents & POLLIN)) // Verifica se il primo file descriptor è pronto per la lettura. il "&" compie un'azione 'read-only'
        {
            struct sockaddr_in cli_addr;
            socklen_t cli_len = sizeof(cli_addr);
            int client_fd = accept(_pfds[0].fd, (struct sockaddr*)&cli_addr, &cli_len);
            if (client_fd < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    // no more incoming connections ready
                    //break;//questo interrompe tutto il ciclo del server? e' giusto?
                    continue;
                }
                perror("accept");
                // break;
                continue;//?
            }

            if (set_nonblocking(client_fd) < 0) //e' gisuto settare anche i client nonblocking? a quanto non-necessario (da chiarire)
            {
                perror("set_nonblocking(client)");
                close(client_fd);
                continue;
            }
            // Allocate User on the heap so Server can store a valid pointer
            User *u = new User(client_fd);
            //setUser(*u, client_fd);
            setClientName(*u);

            struct pollfd newclient;
            newclient.fd = client_fd;
            newclient.events = POLLIN;
            newclient.revents = 0;
            _pfds.push_back(newclient);
            
            

           /*  // Initialize buffers and client info
            std::string empty = "";
            // setInbuf(client_fd, empty);
            _inbuf[client_fd] = empty;
            // setOutbuf(client_fd, empty);
            _outbuf[client_fd] = empty;
 */
            char ipbuf[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(cli_addr.sin_addr), ipbuf, INET_ADDRSTRLEN);

            setClientName(*u);
            std::map<int, User*>::iterator it = _users.find(client_fd);
            it->second->setAuth(_password.empty());

            std::cout << "Accepted client fd=" << client_fd
                        << " ip=" << ipbuf
                        << " port=" << ntohs(cli_addr.sin_port) << std::endl;
            // pfds[0].revents = 0;// non necessaria
        }

        // ---- 2) Handle existing clients (reading/writing) ----
        for (size_t i = 1; i < _pfds.size(); ++i) //++i
        {
            int fd = _pfds[i].fd;
            short rev = _pfds[i].revents;

            if (rev == 0) continue;

            // Error or hangup: Close client
            if (rev & (POLLERR | POLLHUP | POLLNVAL))
            {
                std::string iBuf = _inbuf[fd];
                std::string oBuf = _outbuf[fd];
                closeClient(fd, rev, i);
                --i;
                continue;
            }

            // Data available for reading (POLLIN)
            if (rev & POLLIN) {
                std::string iBuf = _inbuf[fd];
                std::string oBuf = _outbuf[fd];
                if (recvLoop(fd, i)) //--i
                {
                    std::map<int, User*>::iterator it = _users.find(fd);
                    std::string fdit = it->second->getNickname();
                    // Close client: Cleanup done in the error/hangup block above
                    std::string disconn_msg = fdit + " left the chat.\r\n";
                    for (size_t k = 1; k < _pfds.size(); ++k) {
                        if (_pfds[k].fd != fd) {
							//notifica a tutti i client che un client ha lasciato il server
                            _outbuf[_pfds[k].fd].append(disconn_msg);
                            _pfds[k].events |= POLLOUT;
                        }
                    }
                    
                    // clientCleanUp(fd, client_name, authenticated, pfds, i);
                    closeClient(fd, rev, i);
                    --i;
                    continue;
                }
            } // end POLLIN

            // Socket ready for writing (POLLOUT)
            if (_pfds.size() > i && (rev & POLLOUT)) {
                std::string iBuf = _inbuf[fd];
                std::string oBuf = _outbuf[fd];
                readyForWrite(fd, i);
                // If the buffer is empty, stop asking to write
                if (_pfds.size() > i && _outbuf[fd].empty()) {
                    _pfds[i].events &= ~POLLOUT;
                }
            } // end POLLOUT
            // Clean up revents for the next iteration
            if (_pfds.size() > i) _pfds[i].revents = 0;
        } // end for clients
    } // end while
}


Server::Server(std::string port, std::string psw)
{
	this->_port = std::atoi(port.c_str());
    std::cout << "Chat Server listening on port " << port << std::endl;
	_password = psw;

    int server_fd = make_server_socket(this->_port);
    //if (server_fd < 0) return 1; //che errore se make server socket fallisce?
    struct pollfd p;
    p.fd = server_fd;
    p.events = POLLIN; // Wait for new connections
    p.revents = 0;
    _pfds.push_back(p);

    //std::map<int, std::string> client_name; // Simple map to store client 'name' (fd in this case)

    //std::map<int, bool> authenticated;//cambiare logica di questo con : accedi a lista utenti e controlla se sono autenticati
    this->_inbuf = std::map<int, std::string>();
    this->_outbuf = std::map<int, std::string>();
	command_map();
}

bool	Server::nick(int fd, std::vector<std::string> vect)
{
	std::string nick = this->_users[fd]->getNickname();
    std::string newNick = vect[1];
    std::map<int, User *>::iterator it = _users.begin();
    for (; it != _users.end(); ++it)
	{
		if (it->first != fd && it->second->getNickname() == newNick)
		{
			//error message here
			return (false);
		}
	}
    it->second->setNickname(newNick);
    return (true);
}

void Server::command_map()
{
	_commands["NICK"] = &Server::nick;
	_commands["PASS"] = &Server::pass;
	_commands["USER"] = &Server::user;
}


std::vector<Channel> &Server::getChannel()
{
    return (this->_allChannel);
}

void Server::setInbuf(int fd, std::string &buf)
{
    _inbuf[fd] = buf;
}

void Server::setOutbuf(int fd, std::string &buf)
{
    _outbuf[fd] = buf;
}

std::string&         Server::getInbuf(int fd)
{
    return (_inbuf[fd]);
}
std::string&         Server::getOutbuf(int fd)
{
    return (_outbuf[fd]);
}

/* void        Server::clientCleanUp(int fd, std::map<int, std::string> &client_names,
                                  std::map<int, bool> &authenticated,
                                  std::vector<pollfd> &pfds, size_t idx)
{
    // close socket
    close(fd);

    // erase buffers and client records stored in the Server
    iBuf.erase(fd);
    oBuf.erase(fd);

    // erase external maps passed by caller
    client_names.erase(fd);
    authenticated.erase(fd);

    // erase user pointer if present
    users.erase(fd);

    // remove pollfd entry if index still valid
    if (idx < pfds.size())
        pfds.erase(pfds.begin() + idx);
} */

void       Server::setUser(User &u, int fd)
{
    // User* user = new User(fd);
    _users[fd] = &u;
}

User        *Server::getUser(int fd)
{
    std::map<int, User *>::iterator it = _users.find(fd);
    if (it != _users.end())
        return (it->second);
    return NULL;
}

/* void	add_to_pfds(struct pollfd p, Server serv)
{
	pfds.push_back(p);
	std::cout << 
} */

int Server::set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) return -1;
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) return -1;
    return 0;
}

int Server::make_server_socket(int port) {
    int srv = socket(AF_INET, SOCK_STREAM, 0);
    if (srv < 0) { perror("socket"); return -1; }

    int opt = 1;
    if (setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(srv);
        return -1;
    }

    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(srv, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(srv);
        return -1;
    }

    if (listen(srv, 16) < 0) {
        perror("listen");
        close(srv);
        return -1;
    }

    if (set_nonblocking(srv) < 0) {
        perror("set_nonblocking");
        close(srv);
        return -1;
    }

    return srv;
}