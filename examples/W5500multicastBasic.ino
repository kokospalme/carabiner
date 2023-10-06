#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

/*
- Ableton Link relies von multicast UDP messaging
- The multicast-ip for everyone in the network is 224.76.78.75 (76, 78, 75 in ASCII = LNK)
- 
*/

//pin defines for ESP32 C3
#define I2C_CLOCK 400000
#define SCL_PIN 5
#define SDA_PIN 4
#define MISO_PIN 2
#define MOSI_PIN 7
#define SCK_PIN 6
#define W5500_RST_PIN 3
#define CS_PIN 10

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ipMulti(224, 76, 78, 75);
IPAddress ip(192, 168, 178, 25);
IPAddress dns(192, 168, 178, 1);
IPAddress gateway(192, 168, 178, 1);
IPAddress subnet(255, 255, 255, 0);
unsigned int localPort = 20808;      // local port to listen on

// buffers for receiving and sending data
char packetBuffer[UDP_TX_PACKET_MAX_SIZE];  // buffer to hold incoming packet,
char ReplyBuffer[] = "acknowledged";        // a string to send back

// An EthernetUDP instance to let us send and receive packets over UDP
EthernetUDP Udp;

void checkHardware();
void receiveUdpPacket();

void setup() {
  Serial.begin(115200);
  delay(2000);

  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, CS_PIN);
  // Ethernet.init(33);  // (33) default for ESP32 ; -->(10) for most Arduino
  Serial.println("Trying to get an IP address using DHCP");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // Check for Ethernet hardware present
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
      while (true) {
        delay(1); // do nothing, no point running without Ethernet hardware
      }
    }
    if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernet cable is not connected.");
    }
    // initialize the Ethernet device not using DHCP:
    Ethernet.begin(mac, ip, dns, gateway, subnet);
  }
  // Check for Ethernet hardware present
  
  checkHardware();
  Udp.beginMulticast(ipMulti, localPort);
}

void loop() {
  auto link = Ethernet.linkStatus();
  // Serial.print("Link status: ");
  switch (link) {
    case Unknown:
      Serial.println("Link status: Unknown");
      delay(1000);
      break;
    case LinkON:
      // Serial.println("ON");
      break;
    case LinkOFF:
      Serial.println("Link status: OFF");
      delay(1000);
      break;
  }
  

  if(Ethernet.linkStatus() == LinkON){
    // Serial.println("checkPacket...");
    receiveUdpPacket();
  }
  delay(10);
}


void checkHardware(){
  if (Ethernet.hardwareStatus() == EthernetNoHardware) 
  {
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    while (true) 
    {
      delay(1); // do nothing, no point running without Ethernet hardware
    } 

  }
  else if (Ethernet.hardwareStatus() == EthernetW5100) 
  {
    Serial.println("W5100 Ethernet controller detected.");
    delay(5000);
  }
  else if (Ethernet.hardwareStatus() == EthernetW5200) 

  {
    Serial.println("W5200 Ethernet controller detected.");
  }
  else if (Ethernet.hardwareStatus() == EthernetW5500) 
  {
    Serial.println("W5500 Ethernet controller detected.");
    delay(5000);
  }
}


void receiveUdpPacket(){
    // if there's data available, read a packet

  int packetSize = Udp.parsePacket();
  if (packetSize) {
    Serial.print("Received packet of size ");
    Serial.println(packetSize);
    Serial.print("From ");
    IPAddress remote = Udp.remoteIP();
    for (int i=0; i < 4; i++) {
      Serial.print(remote[i], DEC);
      if (i < 3) {
        Serial.print(".");
      }
    }
    Serial.print(", port ");
    Serial.println(Udp.remotePort());

    // read the packet into packetBuffer
    Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
    Serial.println("Contents:");
    Serial.println(packetBuffer);

    // send a reply to the IP address and port that sent us the packet we received
    // Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    // Udp.write(ReplyBuffer);
    // Udp.endPacket();
  }
}