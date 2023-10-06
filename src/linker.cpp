#include "linker.h"


sessionStatus_t Linker::sessionStatus;
EthernetUDP Linker::udpAlink;
IPAddress Linker::ipMultiALink(224, 76, 78, 75);
alinkConfig_t Linker::alinkConfig;
SemaphoreHandle_t Linker::sync_alinkconfig;
TaskHandle_t Linker::alinkListener_taskhandler;
unsigned long Linker::alinkListenerTimer = 0;
char Linker::alinkPacketBuffer[UDP_TX_PACKET_MAX_SIZE];


void Linker::setupALink(){
    udpAlink.beginMulticast(ipMultiALink, LOCALPORT_ALINK);
    xTaskCreatePinnedToCore(Linker::alinkListener_task, "alinkListener_task", 1024, NULL, ALINKLISTENER_TASKPRIO, &Linker::alinkListener_taskhandler, ALINKLISTENER_TASKCORE);
}

void Linker::alinkListener_task(void*pvParameters) { //transmit via NRF24
    Serial.println("alinkListener_task started.");
    alinkListenerTimer = 0;

    xSemaphoreTake(sync_alinkconfig, portMAX_DELAY);
    int updateInterval_ms  = alinkConfig.updateInterval_ms;
    xSemaphoreGive(sync_alinkconfig);

    for(;;){

        if(Ethernet.linkStatus() == LinkON && millis() >= alinkListenerTimer + updateInterval_ms){
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
        delay(updateInterval_ms / 4);
    }
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