/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   map.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: negambar <negambar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/22 09:56:34 by negambar          #+#    #+#             */
/*   Updated: 2025/10/28 14:32:30 by negambar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "library/irc.hpp"

using namespace std;

/* 
 * checks if the nickname already exists, if not, it adds it to the list. 
*/
int addNickname(map<int,string>&cp, const string &n)
{
    if (cp.empty())
    {
        cp[0] = n;
        return (0);
    }
    int k;
    for (map<int, string>::iterator it = cp.begin(); it != cp.end(); ++it)
    {
        if (n == it->second)
        {
            std::cerr << "nickname already in use" << std::endl;
            return (-999);
        }
    }
    k = cp.rbegin()->first + 1;
    cp[k] = n;
    return (k);
}

/*
 * same as addNickname() but with the exception that, 
 * if there already is an element at the given key, 
 * that pre-existing key and all the ones after are moved down by one.
 */
int addWithKey(map<int, string>&copy, int key, const string &n)
{
    int i = 0;
    if (copy.empty())
        copy[i] = n;
    else
    {
        map<int, string>::iterator it = copy.begin();
        for (it = copy.begin(); it != copy.end(); ++it)
            {
            if (key == it->first)
            {
                map<int, string> temp;
                for (map<int, string>::iterator iter = copy.begin(); iter != copy.end(); ++iter) {
                    if (iter->first == key) {
                        temp[key] = n;
                    }
                    temp[iter->first + (iter->first >= key ? 1 : 0)] = iter->second;
                }
                copy = temp;
                return (-222);
            }
        }
        if (it == copy.end())
            addNickname(copy, n);
    }
    return (1);
}


int main(){
    map<int,string> nickname;

    char c = 'a';
    for (int i = 0; i < 4; i++) {nickname[i] = c;c++;}

    if (addNickname(nickname, "e") < 0)
    {
        return (cout << "nickname already exists!" << endl, 1);
    }
    else
    {   
        map<int,string>::iterator it;
        for(it = nickname.begin(); it!=nickname.end(); ++it)
        {
            cout << it->first << "=> nickname:" << it->second << endl;
        }
    }
    map<int, string>::iterator it = nickname.end();
    it--;
    nickname.erase(it);
    cout << endl
         << endl
         << endl
         << endl
         << endl
         << endl;
    it = nickname.begin();
    map<int, string>::iterator toErase;
    while (it != nickname.end())
    {
        toErase = it++;
        nickname.erase(toErase);
    }
    addWithKey(nickname, 0, "testing");
    for(it = nickname.begin(); it!=nickname.end(); ++it)
    {
        cout << it->first << "=>" << it->second << endl;
    }
    cout << endl;
    addWithKey(nickname, 0, "testing successivo");
    addWithKey(nickname, 0, "testing ancora dopo");
    for(it = nickname.begin(); it!=nickname.end(); ++it)
    {
        cout << it->first << "=>" << it->second << endl;
    }
    return 0;
}
