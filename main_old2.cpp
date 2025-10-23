// main.cpp
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <cstring>
#include <cerrno>
#include <stdio.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
/* 
 * fcntl(fd, F_GETFL, 0) legge i flag presenti in fd
 * fcntl(fd, F_SETFL, flags | O_NONBLOCK) aggiunge O_NONBLOCK ai flag presenti
 * 
*/
int set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) return -1;
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) return -1;
    return 0;
}

int make_server_socket(int port) {
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

	/* 
	* server in ascolto 
	* listen() con 16 di backlog come valore arbitrario temporaneo, dopo decidiamo quanto fare
	*/
    if (listen(srv, 16) < 0) {
        perror("listen");
        close(srv);
        return -1;
    }

	//setta server come non bloccante
    if (set_nonblocking(srv) < 0) {
        perror("set_nonblocking");
        close(srv);
        return -1;
    }

    return srv;
}

int main(int argc, char **argv) {
    if (argc != 3) {
        std::cerr << "Usage: ./ircserv <port> <password>\n";
        return 1;
    }
    int port = std::atoi(argv[1]);
    std::string password = argv[2];

    int server_fd = make_server_socket(port);
    if (server_fd < 0) return 1;

    std::cout << "Server listening on port " << port << std::endl;

    // vettore di pollfd: index 0 -> server_fd, gli altri -> clients
    std::vector<struct pollfd> pfds;
    struct pollfd p;
    p.fd = server_fd;
    p.events = POLLIN; // vogliamo sapere nuove connessioni
    p.revents = 0;
    pfds.push_back(p);

    // buffers per ogni client (in e out)
	/* questo e' dal punto di vista del server, quindi
	 * inbuf = da client a server
	 * outbuf = da server a client
	*/
    std::map<int, std::string> inbuf;
    std::map<int, std::string> outbuf;

    while (true) {
        int timeout = -1; // aspetta all'infinito
        int rv = poll(&pfds[0], pfds.size(), timeout);
        if (rv < 0) {
            if (errno == EINTR) continue; // interrotto da signal, riprova
            perror("poll");
            break;
        }
        if (rv == 0) continue; // timeout (non usato qui)

        // ---- 1) gestione nuove connessioni: pfds[0] è il listening socket ----
		//controlla se ci sono nuove connessioni al server
        if (pfds.size() > 0 && (pfds[0].revents & POLLIN)) {
            // Accept in loop perché server_fd è non bloccante: possono esserci più connessioni pronte
            while (true) {
                struct sockaddr_in cli_addr;
                socklen_t cli_len = sizeof(cli_addr);
                int client_fd = accept(server_fd, (struct sockaddr*)&cli_addr, &cli_len);
                if (client_fd < 0) {
                    if (errno == EAGAIN || errno == EWOULDBLOCK) {
                        // nessuna altra connessione pronta
                        break;
                    } else {
                        perror("accept");
                        break;
                    }
                }

                // rendi non bloccante il client socket
                if (set_nonblocking(client_fd) < 0) {
                    perror("set_nonblocking(client)");
                    close(client_fd);
                    continue;
                }

                // aggiungi al vettore pfds
                struct pollfd np;
                np.fd = client_fd;
                np.events = POLLIN; // inizialmente ci interessa leggere
                np.revents = 0;
                pfds.push_back(np);

                // inizializza i buffer
                inbuf[client_fd] = "";
                outbuf[client_fd] = "";

				/*converto indirizzo IPv4 in stringa per stamparlo
					The <netinet/in.h> header shall define the following macro to help applications declare buffers of the proper size to store IPv4 addresses in string form:
						INET_ADDRSTRLEN
						16. Length of the string form for IP.*/
                char ipbuf[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &(cli_addr.sin_addr), ipbuf, INET_ADDRSTRLEN);
                std::cout << "Accepted client fd=" << client_fd
                          << " ip=" << ipbuf
                          << " port=" << ntohs(cli_addr.sin_port) << std::endl;
            }
            // pulisci revents del server_fd
            pfds[0].revents = 0;
        }

        // ---- 2) gestione dei client esistenti ----
        for (size_t i = 1; i < pfds.size(); ++i) {
            int fd = pfds[i].fd;
            short rev = pfds[i].revents;

            if (rev == 0) continue; // niente accaduto per questo fd

            // error or hangup
			/* POLLERR = pipeline read end closed
			 * POLLHUP = hung up
			 * POLLNVAL = invalid request, fd not open
			*/ 
            if (rev & (POLLERR | POLLHUP | POLLNVAL)) {
                std::cout << "Client fd=" << fd << " closed or error (revents=" << rev << ")\n";
                close(fd);
                inbuf.erase(fd);
                outbuf.erase(fd);
                pfds.erase(pfds.begin() + i);
                --i;
                continue;
            }

            // dati disponibili in lettura
            if (rev & POLLIN) {
                bool closed = false;
                while (true) {
                    char buf[4096];
                    ssize_t n = recv(fd, buf, sizeof(buf), 0);
                    if (n > 0) {
                        inbuf[fd].append(buf, buf + n);
                        // prova a processare linee terminate da '\n'
                        size_t pos;
                        while ((pos = inbuf[fd].find('\n')) != std::string::npos) {
                            std::string line = inbuf[fd].substr(0, pos + 1);
                            inbuf[fd].erase(0, pos + 1);
                            // qui ci metteremo il parsing del comando; per ora facciamo echo
                            std::string reply = "Server echo: " + line;
                            outbuf[fd].append(reply);
                            // segna che vogliamo scrivere sul socket
                            pfds[i].events |= POLLOUT;
                        }
                    } else if (n == 0) {
                        // connessione chiusa dal peer
                        std::cout << "Client fd=" << fd << " disconnected (recv==0)\n";
                        closed = true;
                        break;
                    } else {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            // nessun altro dato da leggere ora
                            break;
                        } else {
                            perror("recv");
                            closed = true;
                            break;
                        }
                    }
                } // end while recv loop

                if (closed) {
                    close(fd);
                    inbuf.erase(fd);
                    outbuf.erase(fd);
                    pfds.erase(pfds.begin() + i);
                    --i;
                    continue;
                }
            } // end POLLIN

            // socket pronto a scrivere
            if (rev & POLLOUT) {
                std::string &buf = outbuf[fd];
                while (!buf.empty()) {
                    ssize_t n = send(fd, buf.c_str(), buf.size(), 0);
                    if (n > 0) {
                        buf.erase(0, n);
                    } else {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            // non possiamo inviare ora, riproviamo più tardi
                            break;
                        } else {
                            perror("send");
                            // chiudi il client su errore
                            close(fd);
                            inbuf.erase(fd);
                            outbuf.erase(fd);
                            pfds.erase(pfds.begin() + i);
                            --i;
                            break;
                        }
                    }
                }
                // se non abbiamo più dati da inviare, togliamo POLLOUT
                if (pfds.size() > i && outbuf.find(fd) != outbuf.end() && outbuf[fd].empty()) {
                    pfds[i].events &= ~POLLOUT;
                }
            } // end POLLOUT

            // pulisci revents per la prossima iterazione
            if (pfds.size() > i) pfds[i].revents = 0;
        } // end for clients
    } // end while

    // chiusura pulita
    for (size_t i = 0; i < pfds.size(); ++i) close(pfds[i].fd);
    return 0;
}
