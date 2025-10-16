#include "irc.hpp"

using namespace std;

int addNickname(map<int,string>&cp, const string &n)
{
    int i;
    int k;
    if (cp.empty())
        i = 0;
    else
    {
        for (map<int, string>::iterator it = cp.begin(); it != cp.end(); ++it)
        {
            if (n == it->second)
            {
                return (-999);
            }
        }
        k = cp.rbegin()->first + 1;
        cp[k] = n;
    }
    return (k);
}

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

    addWithKey(nickname, 1, "testing");
    if (addNickname(nickname, "e") < 0)
    {
        return (cout << "nickname already exists!" << endl, 1);
    }
    else
    {   
        map<int,string>::iterator it;
        for(it = nickname.begin(); it!=nickname.end(); ++it)
        {cout << it->first << "=>" << it->second << endl;}
    }
    map<int, string>::iterator it = nickname.end();
    it--;
    nickname.erase(it);
    cout << endl;
    cout << endl;
    cout << endl;
    cout << endl;
    cout << endl;
    cout << endl;
    for(it = nickname.begin(); it!=nickname.end(); ++it)
    {
        cout << it->first << "=>" << it->second << endl;
    }
    
    return 0;
}
