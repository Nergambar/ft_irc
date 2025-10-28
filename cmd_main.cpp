#include "library/servers.hpp"
#include "library/irc.hpp"

int main()
{
    Server gserver;
    // Create two users (adjust constructors/fields if your irc.hpp defines different signatures)
    User alice("alice");
    User bob("bob");
    alice.setServ(&gserver);
    bob.setServ(&gserver);

    // Set simple fields used by operators.cpp

    // Ensure channel permission entries exist for the test channel
    const std::string chan = "#test";

    // Alice joins/creates the channel (joinChannel implementation uses 'this', so pass alice as first arg)
    alice.joinChannel(chan);    
    // bob.joinChannel(chan);

    std::cout << "Before makeOperator:\n";
    std::cout << " Alice operator flag for " << chan << ": " << alice.isOper(chan) << "\n";
    std::cout << " Bob   operator flag for " << chan << ": " << bob.isOper(chan) << "\n\n";

    // Alice makes Bob an operator
    alice.makeOperator(bob, chan);
    
    std::cout << "\nAfter makeOperator:\n";
    std::cout << " Alice in " << chan << ": " << alice.isOper(chan) << "\n";
    std::cout << " Bob in " << chan << ": " << bob.isOper(chan) << "\n";


    // Alic kicks bob from chan
    alice.kick(bob, chan);
    
    std::cout << "\nAfter kick:\n";
    std::cout << " Alice in " << chan << ": " << alice.isOper(chan) << "\n";
    std::cout << " Bob in " << chan << ": " << bob.isOper(chan) << "\n";
    
    
    std::cout << std::endl
              << std::endl
              << std::endl
              << std::endl
              << std::endl;
    
    
    
    Channel *name = gserver.findChannel(chan);
    alice.setLimit(*name, 2);
    // bob.joinChannel(name->getName());
    bob.joinChannel(chan);
    User carl("Carl");
    carl.setServ(&gserver);
    carl.joinChannel(chan);
    std::vector<Channel> &c = gserver.getChannel();
    for (std::vector<Channel>::iterator it = c.begin(); it != c.end(); ++it)
    {
        std::cout << "'" << it->getName() << "' : '" << it->getTopic() << "' : '" << it->getPw() << "' : '" << it->getLimit() << "'" << std::endl;
    }


    std::cout << std::endl
              << std::endl
              << std::endl
              << std::endl
              << std::endl;

    alice.kick(bob, chan);
    carl.joinChannel(chan);

    alice.joinChannel("ccccc");
    std::map<std::string, bool> m = alice.getPermissions();
    std::cout << "ALICE:" << std::endl;
    for (std::map<std::string, bool>::iterator it = m.begin(); it != m.end(); ++it)
    {
        std::cout << it->first << " ==== " <<it->second << std::endl;
    }
    m = bob.getPermissions();
    std::cout << "BOB:" << std::endl;
    for (std::map<std::string, bool>::iterator it = m.begin(); it != m.end(); ++it)
    {
        std::cout << it->first << " ==== " <<it->second << std::endl;
    }
    m = carl.getPermissions();
    std::cout << "CARL:" << std::endl;
    for (std::map<std::string, bool>::iterator it = m.begin(); it != m.end(); ++it)
    {
        std::cout << it->first << " ==== " <<it->second << std::endl;
    }
    std::cout << std::endl
              << std::endl
              << std::endl
              << std::endl
              << std::endl;
    return 0;
}