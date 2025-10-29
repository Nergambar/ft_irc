/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   helper1.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: negambar <negambar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/29 11:52:49 by negambar          #+#    #+#             */
/*   Updated: 2025/10/29 12:29:18 by negambar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../library/servers.hpp"


void Server::setClientName(User &u)
{
    int fd = u.getFd();
    std::ostringstream name_os;
    name_os << "user" << fd;
    u.setNickname(name_os.str());
    users[fd] = &u;
}

