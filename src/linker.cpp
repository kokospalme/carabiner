#include "linker.h"

LinkSession Linker::session;
sessionStatus_t Linker::sessionStatus;
EthernetUDP Linker::udpAlink;
IPAddress Linker::ipMultiALink(224, 76, 78, 75);
alinkConfig_t Linker::alinkConfig;
SemaphoreHandle_t Linker::sync_alinkconfig;
TaskHandle_t Linker::alinkListener_taskhandler;
unsigned long Linker::alinkListenerTimer = 0;
char Linker::alinkPacketBuffer[UDP_TX_PACKET_MAX_SIZE];
uint64_t Linker::peerIds[ALINK_MAXPEERS];


void Linker::setupALink(){
    sync_alinkconfig = xSemaphoreCreateMutex(); //create semaphore

    udpAlink.beginMulticast(ipMultiALink, LOCALPORT_ALINK);

    // xTaskCreatePinnedToCore(Linker::alinkListener_task, "alinkListener_task", 1024, NULL, ALINKLISTENER_TASKPRIO, &Linker::alinkListener_taskhandler, ALINKLISTENER_TASKCORE);
}

// !!! Does not work for now ( gets no data and crashes the uC if data is coming )
void Linker::alinkListener_task(void*pvParameters) { //task that listens to a ableton link ethernet UDP port
    Serial.println("alinkListener_task started.");
    alinkListenerTimer = 0;

    xSemaphoreTake(sync_alinkconfig, portMAX_DELAY);
    int updateInterval_ms  = alinkConfig.updateInterval_ms;
    xSemaphoreGive(sync_alinkconfig);

    for(;;){

        if(millis() >= alinkListenerTimer + updateInterval_ms){
            alinkListenerTimer = millis();

            int packetSize = udpAlink.parsePacket(); //get udp packet size
            if (packetSize) {
                Serial.print("Received packet of size ");
                Serial.println(packetSize);
                // Serial.print("From ");
                // IPAddress remote = udpAlink.remoteIP();
                for (int i=0; i < 4; i++) {
                // Serial.print(remote[i], DEC);
                if (i < 3) {
                    // Serial.print(".");
                }
                }
                // Serial.print(", port ");
                // Serial.println(Udp.remotePort());

                // read the packet into packetBuffer
                udpAlink.read(alinkPacketBuffer, packetSize);
                // Serial.println("Contents:");

                for(int i = 0; i < packetSize; i ++){
                Serial.printf("%x ",alinkPacketBuffer[i]);
                }
                Serial.println("\n");

                // send a reply to the IP address and port that sent us the packet we received
                // Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
                // Udp.write(ReplyBuffer);
                // Udp.endPacket();
            }
        }
        delay(5);
    }
} 

void Linker::run(){
    if(Ethernet.linkStatus() == LinkON) alinkListener_run();

    session.print();

}

void Linker::alinkListener_run(){
    if( millis() >= alinkListenerTimer + alinkConfig.updateInterval_ms){
        alinkListenerTimer = millis();

        int packetSize = udpAlink.parsePacket(); //get udp packet size
        if (packetSize) {
            udpAlink.read(alinkPacketBuffer, packetSize);

            alinkMessage _msg;
            uint8_t _charCounter = 0;
            if(alinkPacketBuffer[0] != ALINK_MAGIC0) return;    //check if magic happens
            if(alinkPacketBuffer[1] != ALINK_MAGIC1) return;
            if(alinkPacketBuffer[2] != ALINK_MAGIC2) return;
            if(alinkPacketBuffer[3] != ALINK_MAGIC3) return;
            if(alinkPacketBuffer[4] != ALINK_MAGIC4) return;
            if(alinkPacketBuffer[5] != ALINK_MAGIC5) return;
            // Serial.print("Magic ok. ");
            _charCounter +=6;
            _msg.protocolVersion[0] = alinkPacketBuffer[_charCounter];  //protocol version
            _msg.protocolVersion[1] = alinkPacketBuffer[_charCounter+1];
            _charCounter +=2;
            _msg.packetType[0] = alinkPacketBuffer[_charCounter];   //packet type & subtype
            _msg.packetType[1] = alinkPacketBuffer[_charCounter+1];
            _charCounter +=2;
            switch(_msg.packetType[0]){
                case ALINK_PACKETTYPE_TIME: //ToDo: peer to list if he/she is not new
                alinkerDecodeMusicmsg(alinkPacketBuffer, &_msg);
                break;
                case ALINK_PACKETTYPE_PEERGOODBYE:  //ToDO: erase peer from list
                break;
                default:break;
            }

            //ToDo: decode message...

            switch(alinkConfig.syncmode){
                case OFF:
                break;
                case PASSIVE:
                    session.setTempo(_msg.msPerBeat);
                break;
                default:
                break;

            }





        }
    }
        delay(alinkConfig.updateInterval_ms / 4);
}

void Linker::alinkerDecodeMusicmsg(char *buf, alinkMessage *msg){
    // Serial.print("musicMsg. ");
    uint8_t _charCounter = 10;  //12 bytes at message's start is fix
    msg->unknown1[0] = buf[_charCounter];   //unknown1
    msg->unknown1[1] = buf[_charCounter+1];
    _charCounter +=2;
    msg->peerId = ((uint64_t)(buf[_charCounter + 0]) << 56) //peerId
                | ((uint64_t)(buf[_charCounter + 1]) << 48)
                | ((uint64_t)(buf[_charCounter + 2]) << 40)
                | ((uint64_t)(buf[_charCounter + 3]) << 32)
                | ((uint64_t)(buf[_charCounter + 4]) << 24)
                | ((uint64_t)(buf[_charCounter + 5]) << 16)
                | ((uint64_t)(buf[_charCounter + 6]) << 8)
                | (uint64_t)buf[_charCounter + 7]; //array to uin16_t
    _charCounter +=8;

    bool peerFound = false;
    for(int i = 0; i < sessionStatus.numberOfPeers; i++){   //looking for peerId
        if(msg->peerId == peerIds[i]){
            peerFound = true;
            break;
        }
    }
    if(peerFound == false){ //peerId not found --> new peer!
        Serial.println("new peer! write in list...");
        peerIds[sessionStatus.numberOfPeers] = msg->peerId; //write peer ID to list
        sessionStatus.numberOfPeers++;  //number of peers +1
    }

    msg->timelineMarker = ((uint32_t)(buf[_charCounter + 0]) << 24) //timeline marker
                | ((uint32_t)(buf[_charCounter + 1]) << 16)
                | ((uint32_t)(buf[_charCounter + 2]) << 8)
                | (uint32_t)buf[_charCounter + 3]; //array to uin16_t
    _charCounter +=4;

    msg->framerate = ((uint32_t)(buf[_charCounter + 0]) << 24)
                | ((uint32_t)(buf[_charCounter + 1]) << 16)
                | ((uint32_t)(buf[_charCounter + 2]) << 8)
                | (uint32_t)buf[_charCounter + 3]; //array to uin16_t
    _charCounter +=4;
    msg->msPerBeat = ((uint64_t)(buf[_charCounter + 0]) << 56)
                | ((uint64_t)(buf[_charCounter + 1]) << 48)
                | ((uint64_t)(buf[_charCounter + 2]) << 40)
                | ((uint64_t)(buf[_charCounter + 3]) << 32)
                | ((uint64_t)(buf[_charCounter + 4]) << 24)
                | ((uint64_t)(buf[_charCounter + 5]) << 16)
                | ((uint64_t)(buf[_charCounter + 6]) << 8)
                | (uint64_t)buf[_charCounter + 7]; //array to uin16_t
    _charCounter +=8;
    long double _msPerBeat = (long double) msg->msPerBeat;
    long double _tempo = 60000.0 / _msPerBeat;
    _tempo *= 1000;
    double _tempo2 = (double) _tempo;

    msg->elapsedMicrobeats = ((uint64_t)(buf[_charCounter + 0]) << 56)  //ToDo: find out what this value stands for
                | ((uint64_t)(buf[_charCounter + 1]) << 48)
                | ((uint64_t)(buf[_charCounter + 2]) << 40)
                | ((uint64_t)(buf[_charCounter + 3]) << 32)
                | ((uint64_t)(buf[_charCounter + 4]) << 24)
                | ((uint64_t)(buf[_charCounter + 5]) << 16)
                | ((uint64_t)(buf[_charCounter + 6]) << 8)
                | (uint64_t)buf[_charCounter + 7]; //array to uin16_t
    _charCounter +=8;

    if(msg->elapsedMicrobeats != alinkConfig.elapsedBeats){
        uint64_t difference = msg->elapsedMicrobeats - alinkConfig.elapsedBeats;
        alinkConfig.elapsedBeats = msg->elapsedMicrobeats;
        uint64_t _beats = (uint64_t) difference;
        _beats /=100000.0;
        Serial.println(_beats);
    }

    msg->elapsedMicroseconds = ((uint64_t)(buf[_charCounter + 0]) << 56)  //ToDo: find out what this value stands for
                | ((uint64_t)(buf[_charCounter + 1]) << 48)
                | ((uint64_t)(buf[_charCounter + 2]) << 40)
                | ((uint64_t)(buf[_charCounter + 3]) << 32)
                | ((uint64_t)(buf[_charCounter + 4]) << 24)
                | ((uint64_t)(buf[_charCounter + 5]) << 16)
                | ((uint64_t)(buf[_charCounter + 6]) << 8)
                | (uint64_t)buf[_charCounter + 7]; //array to uin16_t
    _charCounter +=8;

    uint64_t _elapsedMs = msg->elapsedMicroseconds / 1000;

    // Serial.println(msg->elapsedMicrobeats);

}


/*Set the minimum interval between updates delivered by Carabiner when changes are occurring on the Link session. The default value is 20, to deliver updates up to fifty times per second.
*/
bool Linker::setUpdateInterval(int interval_ms){
    if(interval_ms <=1 && interval_ms > 1000){
        interval_ms = UPDATEINTERVAL_MS_DEFAULT;   //set to default 
        return false;
    }
    else{
        updateInterval_ms = interval_ms;
        return true;
    }
    
}