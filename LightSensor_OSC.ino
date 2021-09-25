#include <hp_BH1750.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <OSCMessage.h>
#include <EEPROM.h>

// ###################################### EEPROM ##############################################################
// Memory slots in use:
int memSlotsLocalIp[] = {0,1,2,3};
int memSlotsRemoteIp[] = {4,5,6,7};
int memSlotsLocalMask[] = {8,9,10,11};
int memSlotsGateway[] = {12,13,14,15};
int memSlotRemotePort[] = {20,21};
int memSlotsMacAddress[] = {101, 102, 103, 104, 105, 106};

// ################################### COMMON VARIABLES ####################################################
#define PowerLed (7)

bool debug = true;

hp_BH1750 BH1750;       //  create the sensor

// ################################### ETHERNET SHIELD VARIABLES ################################################

IPAddress localIp(EEPROM.read(0),EEPROM.read(1),EEPROM.read(2),EEPROM.read(3));
IPAddress localMask(EEPROM.read(memSlotsLocalMask[0]), EEPROM.read(memSlotsLocalMask[1]), EEPROM.read(memSlotsLocalMask[2]), EEPROM.read(memSlotsLocalMask[3]));
IPAddress localGateway(EEPROM.read(memSlotsGateway[0]), EEPROM.read(memSlotsGateway[1]), EEPROM.read(memSlotsGateway[2]), EEPROM.read(memSlotsGateway[3]));
byte mac[] = {EEPROM.read(memSlotsMacAddress[0]), EEPROM.read(memSlotsMacAddress[1]), EEPROM.read(memSlotsMacAddress[2]), EEPROM.read(memSlotsMacAddress[3]), EEPROM.read(memSlotsMacAddress[4]), EEPROM.read(memSlotsMacAddress[5])};
unsigned int localPort = 8888;

IPAddress remoteIp(EEPROM.read(4),EEPROM.read(5),EEPROM.read(6),EEPROM.read(7));
IPAddress remoteIpSec(EEPROM.read(4),EEPROM.read(5),EEPROM.read(6),EEPROM.read(7)+1);
unsigned int remotePort;

EthernetUDP Udp;
EthernetUDP Udp2;

String in_chars = "";


void setup() {

  Serial.begin(9600);

  bool avail = BH1750.begin(BH1750_TO_GROUND);
  BH1750.start();

  pinMode(PowerLed, OUTPUT);

  Serial.println("Absurd Solutions 2021, Light Sensor to OSC");

  restartEthernet();

  digitalWrite(PowerLed, HIGH);
  remotePort = readUnsignedIntFromEEPROM(memSlotRemotePort[0]);

  //setMacAddress(); // Use once for setting MAC address
 }

void loop() {

  char in_char = ' ';
  while (Serial.available()){
    in_char = Serial.read();
    if (int(in_char)!=-1){
      in_chars+=in_char;
    }
  }
  if (in_char=='\n'){
    Serial.print(in_chars);

    if (in_chars.indexOf("remotePort ") == 0){
      remotePort = in_chars.substring(11, in_chars.length()).toInt();
      writeUnsignedIntIntoEEPROM(memSlotRemotePort[0], remotePort);
      Serial.println("remote port changed");
      restartEthernet();
    }

   else if (in_chars.indexOf("localIp ") == 0){
      String ip = "";
      int ip1;
      int ip2;
      int ip3;
      int ip4;

      ip = in_chars.substring(8, in_chars.length()); // split ip to octents and put in own var.
      ip1 = ip.substring(0, ip.indexOf(".")).toInt();
      ip.remove(0, ip.indexOf(".")+1);
      ip2 = ip.substring(0, ip.indexOf(".")).toInt();
      ip.remove(0, ip.indexOf(".")+1);
      ip3 = ip.substring(0, ip.indexOf(".")).toInt();
      ip.remove(0, ip.indexOf(".")+1);
      ip4 = ip.toInt();

      IPAddress lIp(ip1,ip2,ip3,ip4); //init new ip address
      localIp = lIp;
      EEPROM.write(memSlotsLocalIp[0], ip1);
      EEPROM.write(memSlotsLocalIp[1], ip2);
      EEPROM.write(memSlotsLocalIp[2], ip3);
      EEPROM.write(memSlotsLocalIp[3], ip4);

      restartEthernet();
      Serial.println("local ip changed");
    }

    else if (in_chars.indexOf("localMask ") == 0){
      String mask = "";
      int mask1;
      int mask2;
      int mask3;
      int mask4;

      mask = in_chars.substring(10, in_chars.length()); // split ip to octents and put in own var.
      mask1 = mask.substring(0, mask.indexOf(".")).toInt();
      mask.remove(0, mask.indexOf(".")+1);
      mask2 = mask.substring(0, mask.indexOf(".")).toInt();
      mask.remove(0, mask.indexOf(".")+1);
      mask3 = mask.substring(0, mask.indexOf(".")).toInt();
      mask.remove(0, mask.indexOf(".")+1);
      mask4 = mask.toInt();

      IPAddress lMask(mask1,mask2,mask3,mask4); //init new ip address
      localMask = lMask;
      EEPROM.write(memSlotsLocalMask[0], mask1);
      EEPROM.write(memSlotsLocalMask[1], mask2);
      EEPROM.write(memSlotsLocalMask[2], mask3);
      EEPROM.write(memSlotsLocalMask[3], mask4);

      restartEthernet();
      Serial.println("local mask changed");
    }

    else if (in_chars.indexOf("localGateway ") == 0){
      String gw = "";
      int gw1;
      int gw2;
      int gw3;
      int gw4;

      gw = in_chars.substring(13, in_chars.length()); // split ip to octents and put in own var.
      gw1 = gw.substring(0, gw.indexOf(".")).toInt();
      gw.remove(0, gw.indexOf(".")+1);
      gw2 = gw.substring(0, gw.indexOf(".")).toInt();
      gw.remove(0, gw.indexOf(".")+1);
      gw3 = gw.substring(0, gw.indexOf(".")).toInt();
      gw.remove(0, gw.indexOf(".")+1);
      gw4 = gw.toInt();

      IPAddress lGw(gw1,gw2,gw3,gw4); //init new ip address
      localGateway = lGw;
      EEPROM.write(memSlotsGateway[0], gw1);
      EEPROM.write(memSlotsGateway[1], gw2);
      EEPROM.write(memSlotsGateway[2], gw3);
      EEPROM.write(memSlotsGateway[3], gw4);

      restartEthernet();
      Serial.println("local Gateway changed");
    }

    else if (in_chars.indexOf("remoteIp ") == 0){
      String ip = "";
      int ip1;
      int ip2;
      int ip3;
      int ip4;

      ip = in_chars.substring(8, in_chars.length()); // split ip to octents and put in own var.
      ip1 = ip.substring(0, ip.indexOf(".")).toInt();
      ip.remove(0, ip.indexOf(".")+1);
      ip2 = ip.substring(0, ip.indexOf(".")).toInt();
      ip.remove(0, ip.indexOf(".")+1);
      ip3 = ip.substring(0, ip.indexOf(".")).toInt();
      ip.remove(0, ip.indexOf(".")+1);
      ip4 = ip.toInt();

      IPAddress rmiP (ip1,ip2,ip3,ip4); //init new ip address
      IPAddress rmSiP (ip1,ip2,ip3,ip4+1); //init new secondary ip address
      EEPROM.write(memSlotsRemoteIp[0], ip1);
      EEPROM.write(memSlotsRemoteIp[1], ip2);
      EEPROM.write(memSlotsRemoteIp[2], ip3);
      EEPROM.write(memSlotsRemoteIp[3], ip4);
      remoteIp = rmiP;
      remoteIpSec = rmSiP;
      restartEthernet();
      Serial.println("remote ip changed");
    }

    else if(in_chars.indexOf("help") == 0){
      Serial.println("=============================  HELP  =============================");
      Serial.println("remotePort <port nr> \t\t remotePort command will change the port OSC commands are sent to.");
      Serial.println("remoteIp <ip address> \t\t remoteIp changes the Ip address OSC commands are sent to.\n");

      Serial.println("localIp <ip address> \t\t localIp changes the Arduino Ip address");
      Serial.println("localGateway <ip address> \t localGateway changes the Gateway on the Arduino");
      Serial.println("localMask <mask> \t\t localMask changes the subnet mask on the Arduino\n");

      Serial.println("show config \t\t\t shows current network config in use.");
      debug = false;
    }

    else if(in_chars.indexOf("show config") == 0){
      Serial.println("======================= CURRENT CONFIG ==========================");
      Serial.print("Local MAC Address: ");
      for (int i=0; i < 6; i++){
        Serial.print(mac[i], HEX);
        if (i < 5) Serial.print(":");
      }
      Serial.println("");

      Serial.print("Local IP Address: ");
      for (int i=0; i<4; i++){
        Serial.print(localIp[i]);
        if (i < 3) Serial.print(".");
      }
      Serial.println("");

      Serial.print("Local Subnet Mask: ");
      for (int i=0; i<4; i++){
        Serial.print(localMask[i]);
        if (i < 3) Serial.print(".");
      }
      Serial.println("");

      Serial.print("Local Gateway: ");
      for (int i=0; i<4; i++){
        Serial.print(localGateway[i]);
        if (i < 3) Serial.print(".");
      }
      Serial.println("");

      Serial.print("Local UDP Port: ");
      Serial.println(localPort);
      Serial.println("");

      Serial.print("Remote IP Address: ");
      for (int i=0; i<4; i++){
        Serial.print(remoteIp[i]);
        if (i < 3) Serial.print(".");
      }
      Serial.println("");

      Serial.print("Remote Secondary IP Address: ");
      for (int i=0; i<4; i++){
        Serial.print(remoteIpSec[i]);
        if (i < 3) Serial.print(".");
      }
      Serial.println("");

      Serial.print("Remote UDP Port: ");
      Serial.println(remotePort);     

    }

    else{
      Serial.println("Unknown command");
      Serial.println("Use help for command list");
    }
    in_chars = "";
    Serial.print(">");
  }

  OSCMessage msg("/lux");

  if (BH1750.hasValue() == true){
    float luxValue = BH1750.getLux();
    if (debug){
      Serial.println(luxValue);
    }
    BH1750.start();
   
    msg.add(luxValue);

    Udp.beginPacket(remoteIp, remotePort);
    msg.send(Udp);
    Udp.endPacket();
    Udp2.beginPacket(remoteIpSec, remotePort);
    msg.send(Udp2);
    Udp2.endPacket();
   
    msg.empty();
  }

}

void restartEthernet(){
  Udp.stop();
  Udp2.stop();
  Ethernet.begin(mac, localIp);
  Ethernet.setSubnetMask(localMask);
  Ethernet.setGatewayIP(localGateway);
  Udp.begin(localPort);
  Udp2.begin(localPort);
}

void setMacAddress(){
  byte setMacAddressArray[] = {0xA8,0x61,0x0A,0xAE,0x67,0xFF};

  EEPROM.write(memSlotsMacAddress[0], setMacAddressArray[0]);
  EEPROM.write(memSlotsMacAddress[1], setMacAddressArray[1]);
  EEPROM.write(memSlotsMacAddress[2], setMacAddressArray[2]);
  EEPROM.write(memSlotsMacAddress[3], setMacAddressArray[3]);
  EEPROM.write(memSlotsMacAddress[4], setMacAddressArray[4]);
  EEPROM.write(memSlotsMacAddress[5], setMacAddressArray[5]);
}

void writeUnsignedIntIntoEEPROM(int address, unsigned int number)
{ 
  EEPROM.write(address, number >> 8);
  EEPROM.write(address + 1, number & 0xFF);
}
unsigned int readUnsignedIntFromEEPROM(int address)
{
  return (EEPROM.read(address) << 8) + EEPROM.read(address + 1);
}

