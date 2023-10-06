#include <Arduino.h>
#include <SPI.h>
#include <linker.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

/*
- Ableton Link relies von multicast UDP messaging
- The multicast-ip for everyone in the network is 224.76.78.75 (76, 78, 75 in ASCII = LNK)
- Pioneer ("beatlink") library: https://deepsymmetry.org/beatlink/apidocs/org/deepsymmetry/beatlink/package-summary.html
- Ableton Link library: 
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

//config stuff
struct config_t{
  bool ipALinkManual = true;  //true = address will be set manual in ALink network, otherwise oder dhcp
  uint8_t aLinkMac[6] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
  uint8_t alinkIp[4] = {192, 168, 178, 25};
  uint8_t alinkDns[4] = {169, 254, 155, 239};
  uint8_t alinkGateway[4] = {192, 168, 178, 1};
  uint8_t alinkSubnet[4] = {255, 255, 255, 0};
}config;

unsigned int localPort = 20808;      // local port to listen on (Ableton Link)

void checkHardware();
void beginEthernet();

void setup() {
  Serial.begin(115200);
  delay(2000);

  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, CS_PIN);
  delay(100);
  checkHardware();
  beginEthernet();

  linker.setupALink();
}

void loop() {
  
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

void beginEthernet(){
  if(config.ipALinkManual == true){
    Serial.print("Set ALink ip manually...");
    Ethernet.begin(config.aLinkMac, config.alinkIp, config.alinkDns, config.alinkGateway, config.alinkSubnet);
    Serial.println("sucess.");
  }else{
    Serial.print("Set ALink ip via DHCP...");
    if (Ethernet.begin(config.aLinkMac) == 0) { //
      Serial.println("failed. setting manually");
      // Check for Ethernet hardware present
    if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernet cable is not connected.");
    }
    // initialize the Ethernet device not using DHCP:
    Ethernet.begin(config.aLinkMac, config.alinkIp, config.alinkDns, config.alinkGateway, config.alinkSubnet);
    Serial.println("sucess.");
  }
  }
}

