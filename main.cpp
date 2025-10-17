/******************************************************************************/
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: scarlucc <scarlucc@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/15 17:29:48 by scarlucc          #+#    #+#             */
/*   Updated: 2025/10/17 12:53:07 by scarlucc         ###   ########.fr       */
/*                                                                            */
/******************************************************************************/

#include "irc.hpp"

int create_serv_pocket(int port)
{
    // 1) Creazione del socket
	/*AF_INET == ipv4
	SOCK_STREAM == TCP
	0 == protocollo default del dominio specificato e tipo*/
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return -1;
    }
    
    // 2) Imposta opzione SO_REUSEADDR (evita errori "address already in use")
	/*server_fd == fd del socket da modificare
	SOL_SOCKET == level, specifica CHI definisca l'opzione da cambiare (impostazione di socket-level generale)
	SO_REUSEADDR == option_name : impostazione (specifica) da cambiare
	opt == option_value
	sizeof() == */
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(server_fd);
        return -1;
    }

	/*La struttura SOCKADDR_IN specifica un indirizzo di trasporto e una porta per la famiglia di indirizzi AF_INET
	sin_family == Famiglia di indirizzi per l'indirizzo di trasporto. Questo membro deve essere sempre impostato su AF_INET
	sin_addr == Struttura IN_ADDR che contiene un indirizzo di trasporto IPv4
	sin_port == ??*/
    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;  // ascolta su tutte le interfacce (127.0.0.1, ecc.)
    addr.sin_port = htons(port);        // converte in "network byte order"
    
    // 3️⃣ Associazione del socket alla porta
    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind"); close(server_fd);
        return -1;
    }
    
    // 4️⃣ Metti il socket in ascolto
    if (listen(server_fd, 10) < 0) {
        perror("listen"); close(server_fd);
        return -1;
    }
    return server_fd;
}

int main(int argc, char **argv) {
    if (argc != 3) {
        std::cerr << "Usage: ./ircserv <port> <password>\n";
        return 1;
    }

    int port = std::atoi(argv[1]);
    std::string password = argv[2];
    
    int server_fd = create_serv_pocket(port);
    if (server_fd < 0)
        return (std::cout << "error with pocket creation", 1);

    std::cout << "IRC Server listening on port " << port << "..." << std::endl;

    // 5️⃣ Ciclo principale: accetta connessioni
    while (true) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        if (client_fd < 0) {
            perror("accept");
            continue;
        }

        std::cout << "New client connected! FD = " << client_fd << std::endl;
        // Chiudi subito la connessione per ora (solo test)
        close(client_fd);
    }

    close(server_fd);
    return 0;
}
