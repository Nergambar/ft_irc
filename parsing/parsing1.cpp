/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing1.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: negambar <negambar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/28 15:10:17 by negambar          #+#    #+#             */
/*   Updated: 2025/11/12 11:04:20 by negambar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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


std::vector<std::string> split2(std::string str, char c, size_t pos)
{
    std::vector<std::string> vect;
    size_t i = 0;
    std::size_t found;
    while ((found = str.find(c, i)) != std::string::npos)
    {
        // if the ':' (pos) is inside the current token, push the rest of the string as a single token
        if (pos != std::string::npos && pos >= i && pos < found)
        {
            vect.push_back(str.substr(i));
            return (vect);
        }
        vect.push_back(str.substr(i, found - i));
        i = found + 1;
    }
    if (i < str.size())
        vect.push_back(str.substr(i));
    return (vect);
}
/* 
int main (void)
{
	std::string str = "ciao mondo :test stuff out";
    size_t pos = str.find(':');
    std::vector<std::string> vect = split2(str, ' ', pos);
    std::vector<std::string>::iterator it = vect.begin();
    while (it != vect.end())
    {
        std::cout << *it << std::endl;
        ++it;
    }
	return (0);		
} */