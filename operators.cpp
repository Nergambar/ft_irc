#include "irc.hpp"

void    User::makeOperator(User &u, std::string channel)
{
    std::map<std::string, bool>::iterator itUser1 = this->channelPerm.begin();
    std::map<std::string, bool>::iterator itUser2 = u.channelPerm.begin();
    itUser1 = this->channelPerm.find(channel);
    itUser2 = u.channelPerm.find(channel);

    if (itUser1 == this->channelPerm.end())
    {
        perror("no channel available for user1");
        return;
    }
    if (itUser2 == u.channelPerm.end())
    {
        perror("no channel available for user2");
        return;
    }

    // check that "this" has the permit to make u an operator
    if (itUser1->second == false)
    {
        std::string msg = nickname + "is not an operator in the channel";
        perror(msg.c_str());
    }

    // grant operator flag in u's channel permissions if not already set
    if (u.isOperator == false && itUser2->second == false)
    {
        itUser2->second = true;
        std::string msg = u.nickname + "has been made into an operator!";
        std::cout << msg << std::endl;
    }
}


void    User::joinChannel(User &u, std::string channel)
{
    std::vector<std::string>::iterator it = this->channelAll.begin();
    it = std::find(this->channelAll.begin(), this->channelAll.end(), channel);
    if (it == this->channelAll.end())
    {
        std::cerr << "No channel available called " + channel << std::endl;
        std::cerr << "Creating " + channel + "..." << std::endl;
        this->channelAll.push_back(channel);
    } // deve aggiungerlo a tutti, per ora solo all'utente che lo crea
    this->channelPerm[channel] = true;
}