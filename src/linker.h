
#ifndef linker_h
#define linker_h
#include <Arduino.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include "linkSession.h"


// Ableton Link stuff
#define UPDATEINTERVAL_MS_DEFAULT 5
#define LOCALPORT_ALINK 20808
#define ALINKLISTENER_TASKPRIO 3
#define ALINKLISTENER_TASKCORE 0
#define ALINK_MAXPEERS 16
#define ALINK_MAGIC0 0x5f
#define ALINK_MAGIC1 0x61
#define ALINK_MAGIC2 0x73
#define ALINK_MAGIC3 0x64
#define ALINK_MAGIC4 0x70
#define ALINK_MAGIC5 0x5f
#define ALINK_PACKETTYPE_TIME 0x01
#define ALINK_PACKETSUBTYPE_A 0x05
#define ALINK_PACKETTYPE_PEERGOODBYE 0x03

enum syncmodes{ OFF,
                MANUAL, //external code will be calling `lock-tempo` and `unlock-tmepo` to manipulate the Ableton Link session
                PASSIVE, //Ableton Link always follows the Pro DJ Link network, and we do not attempt to control other players on that network
                FULL    //bidirectional, determined by the Master and Sync states of players on the DJ Link network, including Beat Link's `VirtualCdj`, which stands in for the Ableton Link session
};

struct alinkConfig_t{
    int updateInterval_ms = UPDATEINTERVAL_MS_DEFAULT;

    uint64_t elapsedBeats;  //ToDO: nur testzwecke!
    uint8_t syncmode = PASSIVE;
};
struct alinkMessage{
    char alinkMagic[6]; // "_asdp_"
    uint8_t protocolVersion[2];
    uint8_t packetType[2];  //type & subtype
    uint8_t unknown1[2];    //unknown bytes
    uint64_t peerId;    //unique ID
    uint32_t timelineMarker;
    uint32_t framerate; //never changing?
    uint64_t msPerBeat; // -->tempo
    uint64_t elapsedMicrobeats;  //unknown, but rate increase changes with bpm
    uint64_t elapsedMicroseconds;   //unknown
    uint32_t sessionmarker;
    uint8_t unknown2[4];
    char sessionId[8];
    uint8_t messagingExchPattern[4];    //unknown, is not changing
    uint8_t unknown3[4];    //is not changing
    uint8_t vendorId[2];
    uint8_t perCl;
    uint8_t perSession;
};

struct sessionStatus_t{
    uint8_t numberOfPeers = 0;
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

    static void setupALink();
    static bool setUpdateInterval(int interval_ms);    //Set the minimum interval between updates delivered by Carabiner when changes are occurring on the Link session.
    static void run();
private:
    static int updateInterval_ms;
    static EthernetUDP udpAlink;  //Ableton on port 20808
    static IPAddress ipMultiALink;
    static int syncMode;
    static LinkSession session;
    static sessionStatus_t sessionStatus;

    //alink stuff
    static alinkConfig_t alinkConfig;
    static  SemaphoreHandle_t sync_alinkconfig;
    static void alinkListener_task(void*pvParameters);  //task that listens to UDP channel
    static TaskHandle_t alinkListener_taskhandler;
    static void alinkListener_run();    //same as task, but not as task
    static unsigned long alinkListenerTimer;
    static char alinkPacketBuffer[UDP_TX_PACKET_MAX_SIZE];
    static void alinkerDecodeMusicmsg(char *buf, alinkMessage *msg);
    static uint64_t peerIds[ALINK_MAXPEERS];    //ToDo: dynamic array of peerIds

}; extern Linker linker;




#endif