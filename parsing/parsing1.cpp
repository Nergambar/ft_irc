/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parseRecv.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: negambar <negambar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/28 15:10:17 by negambar          #+#    #+#             */
/*   Updated: 2025/11/03 14:39:11 by negambar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../library/irc.hpp"


std::vector<std::string> ft_split(std::string str)
{
    std::vector<std::string> vect;
    std::string excerpt = str;
    std::size_t found;
    while ((found = excerpt.find(' ')) != std::string::npos)
    {
        vect.push_back(excerpt.substr(0, found));
        excerpt = excerpt.substr(found + 1);
    }
    if (!excerpt.empty())
        vect.push_back(excerpt);
    return (vect);
}
