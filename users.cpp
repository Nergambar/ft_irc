/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   users.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: negambar <negambar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/16 10:28:17 by negambar          #+#    #+#             */
/*   Updated: 2025/10/17 11:17:54 by negambar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "irc.hpp"


std::string User::getNickname() const
{
    return (this->nickname);
}

std::string User::getUsername() const
{
    return (this->username);
}

//make operator moved to operators.cpp