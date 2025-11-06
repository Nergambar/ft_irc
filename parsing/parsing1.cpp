/******************************************************************************/
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing1.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: scarlucc <scarlucc@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/28 15:10:17 by negambar          #+#    #+#             */
/*   Updated: 2025/11/06 14:44:31 by scarlucc         ###   ########.fr       */
/*                                                                            */
/******************************************************************************/

#include "../library/irc.hpp"


std::vector<std::string> ft_split(std::string str, char c)
{
    std::vector<std::string> vect;
    std::string excerpt = str;
    std::size_t found;
    while ((found = excerpt.find(c)) != std::string::npos)
    {
        vect.push_back(excerpt.substr(0, found));
        excerpt = excerpt.substr(found + 1);
    }
    if (!excerpt.empty())
        vect.push_back(excerpt);
    return (vect);
}


/* int main (void)
{
	std::string str = "ciao mondo";
	std::string excerpt = str;
    std::size_t found;
	if ((found = excerpt.find('\n')) != std::string::npos)
		std::cout << "trovato" << std::endl;
	else	
		std::cout << "non trovato" << std::endl;
	return (0);		
} */