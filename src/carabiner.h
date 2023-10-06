
#ifndef carabiner_h
#define carabiner_h
#include <Ethernet.h>
#include <EthernetUdp.h>

#define UPDATEINTERVAL_MS_DEFAULT 20

enum syncmodes{ OFF,
                MANUAL, //external code will be calling `lock-tempo` and `unlock-tmepo` to manipulate the Ableton Link session
                PASSIVE, //Ableton Link always follows the Pro DJ Link network, and we do not attempt to control other players on that network
                FULL    //bidirectional, determined by the Master and Sync states of players on the DJ Link network, including Beat Link's `VirtualCdj`, which stands in for the Ableton Link session
};

class Carabiner{
public:
    Link link();
    void start();
    void stop();
    bool setUpdateInterval(int interval_ms);    //Set the minimum interval between updates delivered by Carabiner when changes are occurring on the Link session.
private:
    int updateInterval_ms;
    EthernetUDP udp*;
    int syncMode = 
};









#endif