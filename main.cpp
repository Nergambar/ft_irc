/******************************************************************************/
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: scarlucc <scarlucc@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/15 17:29:48 by scarlucc          #+#    #+#             */
/*   Updated: 2025/10/15 17:36:44 by scarlucc         ###   ########.fr       */
/*                                                                            */
/******************************************************************************/

#include <iostream>
#include <string>
#include <cstdio> 		//perror
#include <cstdlib>      // atoi
#include <cstring>      // memset
#include <unistd.h>     // close
#include <sys/socket.h> // socket, bind, listen, accept
#include <netinet/in.h> // sockaddr_in
#include <fcntl.h>      // fcntl

int main(int argc, char **argv) {
    if (argc != 3) {
        std::cerr << "Usage: ./ircserv <port> <password>\n";
        return 1;
    }

    int port = std::atoi(argv[1]);
    std::string password = argv[2];

    // 1️⃣ Creazione del socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return 1;
    }

    // 2️⃣ Imposta opzione SO_REUSEADDR (evita errori "address already in use")
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(server_fd);
        return 1;
    }

    // 3️⃣ Associazione del socket alla porta
    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;  // ascolta su tutte le interfacce (127.0.0.1, ecc.)
    addr.sin_port = htons(port);        // converte in "network byte order"

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(server_fd);
        return 1;
    }

    // 4️⃣ Metti il socket in ascolto
    if (listen(server_fd, 10) < 0) {
        perror("listen");
        close(server_fd);
        return 1;
    }

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
