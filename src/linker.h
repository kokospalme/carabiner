
#ifndef linker_h
#define linker_h
#include <Ethernet.h>
#include <EthernetUdp.h>


// Ableton Link stuff
#define UPDATEINTERVAL_MS_DEFAULT 20
#define LOCALPORT_ALINK 20808
#define ALINKLISTENER_TASKPRIO 0
#define ALINKLISTENER_TASKCORE 0

struct alinkConfig_t{
    int updateInterval_ms = UPDATEINTERVAL_MS_DEFAULT;
};

enum syncmodes{ OFF,
                MANUAL, //external code will be calling `lock-tempo` and `unlock-tmepo` to manipulate the Ableton Link session
                PASSIVE, //Ableton Link always follows the Pro DJ Link network, and we do not attempt to control other players on that network
                FULL    //bidirectional, determined by the Master and Sync states of players on the DJ Link network, including Beat Link's `VirtualCdj`, which stands in for the Ableton Link session
};

struct sessionStatus_t{
    float linkBpm;   //session bpm
};


// CDJ stuff
struct CdjDevice{  //devices broadcasting their presence by announcement packages on Port 50000
    IPAddress ip;
};


class Linker{
public:
    Linker();
    virtual ~Linker();

    void setupALink();
    bool setUpdateInterval(int interval_ms);    //Set the minimum interval between updates delivered by Carabiner when changes are occurring on the Link session.
private:
    static int updateInterval_ms;
    static EthernetUDP udpAlink;  //Ableton on port 20808
    static IPAddress ipMultiALink;
    static int syncMode;
    static sessionStatus_t sessionStatus;

    //alink stuff
    static alinkConfig_t alinkConfig;
    static  SemaphoreHandle_t sync_alinkconfig;
    static void alinkListener_task(void*pvParameters);
    static TaskHandle_t alinkListener_taskhandler;
    static unsigned long alinkListenerTimer;
    static char alinkPacketBuffer[UDP_TX_PACKET_MAX_SIZE];

}; extern Linker linker;






#endif