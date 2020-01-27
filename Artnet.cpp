/*The MIT License (MIT)

Copyright (c) 2014 Nathanaël Lécaudé
https://github.com/natcl/Artnet, http://forum.pjrc.com/threads/24688-Artnet-to-OctoWS2811

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

/* Credit: Art-Net™ Designed by and Copyright Artistic Licence Holdings Ltd */

#include <Artnet.h>

// **** Function Artnet::Artnet() ****
// Descr: Constructior of the call Artnet. Called once the library is loaded. Ideally to set all default values.
// Return: A constructor does not have a return!
Artnet::Artnet() {}

// **** Function Artnet::begin(mac[], ip[]) ****
// Descr: This function enables the Ethernet module (without DCHP) and opens the UDP port.
// Argumenets: mac[] = the mac address to be used. Pointer to array of 6 bytes, ip[] = the static IP address. pointer to array of 4 bytes.
void Artnet::begin(byte mac[], byte ip[])
{
  //First of all, load all default values.
  loadDefaults();

  //load the specified mac address into the node.mac[] array.
  memcpy(node.mac, mac, 6);

  //copy the IP address to the node struct.
  for(int i=0 ; i < 4 ; i++)
        node.ip[i] = ip[i];

  #if !defined(ARDUINO_SAMD_ZERO) && !defined(ESP8266) && !defined(ESP32)
    Ethernet.init(ETH_CHIP_SELLECT);
    Ethernet.begin(mac,ip);
  #endif
  
  //Now we can listen to the Art-Net port.
  Udp.begin(ART_NET_PORT);
}

// **** Function Artnet::begin(mac[]) ****
// Descr: This function enables the Ethernet module and opens the UDP port.
// Argumenets: mac[] = the mac address to be used. Pointer to array of 6 bytes, ip[] = the static IP address. pointer to array of 4 bytes.
// Return: 1 = DCHP assingment was succes ; 0 = DCHP failed!
uint8_t Artnet::begin(byte mac[])
{
  //First of all, load all default values.
  loadDefaults();

  //load the specified mac address into the node.mac[] array.
  memcpy(node.mac, mac, 6);

  #if !defined(ARDUINO_SAMD_ZERO) && !defined(ESP8266) && !defined(ESP32)
    Ethernet.init(ETH_CHIP_SELLECT);
    if(Ethernet.begin(mac)) {
      //Set DCHP true because we received an IP address.
      node.dchp = true;
      
      //Store the assigned IP address.
      //IPAddress temp = getIP();
      for(int i=0 ; i < 4 ; i++)
        node.ip[i] = getIP()[i];

      //Now we can listen to the Art-Net port.
      Udp.begin(ART_NET_PORT);
      return 1;
    }
  #endif

  return 0;
}

// **** Function Artnet::beginWifi(mac[]) ****
// Descr: This function enables the WiFi module and opens the UDP port.
// Argumenets: mac[] = the mac address to be used. Pointer to array of 6 bytes
// Return: 0 = connection failed ; 0x01 connection succes
/*uint8_t Artnet::beginWifi(byte mac[], char ssID[], char secKey[])
{
  //First of all, load all default values.
  loadDefaults();

  //load the specified mac address into the node.mac[] array.
  memcpy(node.mac, mac, 6);

  int i = 0;

  #if !defined(ARDUINO_SAMD_ZERO) && !defined(ESP8266) && !defined(ESP32)
    Ethernet.begin(mac);
  #endif

  WiFi.begin(ssID, secKey);

  if(DEBUG) 
  { 
    Serial.print("WiFi connecting to ");
    Serial.print(ssID);
  }

  while (WiFi.status() != WL_CONNECTED) {
    delay(RETRY_DELAY);
    
    if(DEBUG) 
      Serial.print(".");

    if (i > WIFI_CONNECT_ATTEMTS)
    {
      if(DEBUG) 
        Serial.print(" FAILED!");  
      return 0;
    }
    i++;

  Serial.print("SUCCESS! IP: ");
  Serial.println(WiFi.localIP());

  for(int i=0 ; i < 4 ; i++)
        node.ip[i] = getIP()[i];

  //LOCAL IP NOG OPSLAAN!
  node.dchp = true;

  Udp.begin(ART_NET_PORT);
  return 1;
}*/

// **** Function Artnet::setBroadcast() ****
// Descr: With this function the IP can be set, either by IPAddress type or array of bytes.
void Artnet::setBroadcast(byte bc[])
{
  //sets the broadcast address
  broadcastIP = bc;
}
void Artnet::setBroadcast(IPAddress bc)
{
  //sets the broadcast address
  broadcastIP = bc;
}

// **** Function Artnet::read() ****
// Descr: This function parses the received package, checks wether it is valid
// Return:
//    In case a supported opcode was received the opcode is returned.
//    0xFFFF this means that DCHP renew/rebin failed, user should fall back to static IP or wait untill it recovers.
//    In all other cases 0.
uint16_t Artnet::read()
{ 
  if(node.dchp)
    if(maintainDCHP() != 0)
    {
      Serial.println("A DCHP rebind/renew failed!");
      return 0xFFFF;
    }
      
  packetSize = Udp.parsePacket();

  if(packetSize <= MAX_BUFFER_ARTNET && packetSize > 0)
  {
      controllerIP = Udp.remoteIP();
      Udp.read(artnetPacket, MAX_BUFFER_ARTNET);

      // Check that packetID is "Art-Net" otherwise ignore this packet
      for (byte i = 0 ; i < 8 ; i++)
      {
        if (artnetPacket[i] != ART_NET_ID[i])
          return 0;
      }

      opcode = artnetPacket[8] | artnetPacket[9] << 8;

      switch(opcode) 
      {
        // -- OpDmc or OpOutput was received, now we need to extract the DMX date from the frame.
        case ART_DMX:
          if(DEBUG)
          {
            Serial.print("ArtDmx Received universe [");
            Serial.print(getUniverse());
            Serial.print("] with length of ");
            Serial.print(getLength());
            Serial.print("bytes. Packet sequence is: ");
            Serial.println(getSequence());
          }
            
          sequence = artnetPacket[12];
          incomingUniverse = artnetPacket[14] | artnetPacket[15] << 8;
          dmxDataLength = artnetPacket[17] | artnetPacket[16] << 8;

          if (artDmxCallback) (*artDmxCallback)(incomingUniverse, dmxDataLength, sequence, artnetPacket + ART_DMX_START, controllerIP);
          
          return ART_DMX;

        // -- OpPoll received, now we have to respond with an OpPollReply message within 3 seconds.
        // fill the reply struct, and then send it to the network's broadcast address
        case ART_POLL:
          if(DEBUG)
            Serial.println("ArtPoll Received.");
          if(sendPacket(ART_POLL_REPLY, controllerIP, artnetPacket, 0))
            return ART_POLL;
          else
            return 0; 

        // -- OpSync received, this is the trigger to enable all outputs so they are syncronized. 
        case ART_SYNC:
          
          if (artSyncCallback) (*artSyncCallback)(controllerIP);
          
          return ART_SYNC;
        
        // -- OpAddress received, now we have to respond with an OpPollReply message within 3 seconds with to confirm the changes.
        case ART_ADDRESS:
        {
          if(DEBUG)
            Serial.println("ArtAddress Received.");
          uint8_t bindIndex = artnetPacket[13]-1; 
          //Update port address:
          uint16_t tempPortAddr = 0;
          tempPortAddr |= (0x007F && artnetPacket[12]) << 8;
          tempPortAddr |= 0x00F0 && artnetPacket[104];
          
          for(int j=0 ; j < 4 ; j++) 
          {
            if((bindIndex*4)+j < ART_NUM_UNIVERSES)
             {
                if(node.universe[(bindIndex*4)+j][1] == 1 && j < 4)     //if universe is input then look at Swin array
                  tempPortAddr |= (0x000F && artnetPacket[96+j]);
                else                                                    //if universe is output then look at Swout array
                  tempPortAddr |= (0x000F && artnetPacket[100+j]);
      
                node.universe[(bindIndex*4)+j][0] = tempPortAddr; 
             }  
          }

          //ShortName Field:
          boolean areEquel = false;
          for(int x=0 ; x < 18 ; x++)
          {
            if(node.shortname[x] == artnetPacket[14+x])
              areEquel = true;
          }
          if(areEquel)
          {
            setShortDescr((char*)&artnetPacket[14]);
            node.nodeReportCode = RC_SHNAME_OK;
          }

          //LongName Field:
          areEquel = false;
          for(int x=0 ; x < 18 ; x++)
          {
            if(node.longname[x] == artnetPacket[32+x])
              areEquel = true;
          }
          if(areEquel)
          {
            setShortDescr((char*)&artnetPacket[32]);
            node.nodeReportCode = RC_LONAME_OK;
          }
                  
          //Set Command Field and send out the reply to the controller. (artnetPacket[106])
          if(sendPacket(ART_POLL_REPLY, controllerIP, artnetPacket, 0))
            return ART_ADDRESS | (0x00FF & setCmd(artnetPacket[106]));
          else
            return 0;
        }
        default:
          if(DEBUG) {
            Serial.print("An unsupported Art-Net opcode was recieved: 0x");
            Serial.println(opcode, HEX);
          }  
          return 0;
      }
  }
  else
  {
    return 0;
  }
  return 0;
}

void Artnet::printPacketHeader()
{
  Serial.print("packet size = ");
  Serial.print(packetSize);
  Serial.print("\topcode = ");
  Serial.print(opcode, HEX);
  Serial.print("\tuniverse number = ");
  Serial.print(incomingUniverse);
  Serial.print("\tdata length = ");
  Serial.print(dmxDataLength);
  Serial.print("\tsequence n0. = ");
  Serial.println(sequence);
}

void Artnet::printPacketContent()
{
  for (uint16_t i = ART_DMX_START ; i < dmxDataLength ; i++){
    Serial.print(artnetPacket[i], DEC);
    Serial.print("  ");
  }
  Serial.println('\n');
}

// **** Function Artnet::sendPacket(uint16_t opcode, IPAddress destinationIP, uint8_t *data, uint16_t datasize) ****
// Descr: This function generates the packets and sends to the controller
// Arguments: opcode = this defines the packet you want to send.
//            destinationIP = the location to send the packet to (important for unicast as well as broadcast).
//            *data = pointer to a data buffer (for future expension, this to enable to send ArtDmx packages)
//            datasize = defines the size of the to tranfer buffer. (for future expension)
//    
// Return:  0 = UDP packet was send succesfully
//          1 = UDP packet was not send out for some reason
uint8_t Artnet::sendPacket(uint16_t opcode, IPAddress destinationIP, uint8_t *data, uint16_t datasize)
{
  uint8_t packetsize = 0;
  uint8_t packet[MAX_BUFFER_ARTNET] = {0};
  // Set Art-Net ID
  sprintf((char *)id, "Art-Net");
  memcpy(&packet[0], id, sizeof(id));

  switch(opcode)
  {
    // -- OpPollReply
    case ART_POLL_REPLY:
      for(uint16_t univ=0 ; univ < ART_NUM_UNIVERSES ; univ++) {
        // Set the size of the packet to send, relevant to send out the packet.
        packetsize = ART_SIZE_POLLREPLY;
        // Set the opcode
        packet[ART_NET_OP_OFFSET] = (uint8_t)ART_POLL_REPLY;
        packet[ART_NET_OP_OFFSET+1] = (uint8_t)(ART_POLL_REPLY >> 8);

        //set IP address
        for(int i=0 ; i < 4 ; i++)
          packet[10+i] = node.ip[i];

          // Set artnet port number
          packet[14] = (uint8_t)(ART_NET_PORT >> 8);
          packet[15] = (uint8_t)ART_NET_PORT;

          //Set VersionVersion High, Low
          packet[16] = VersionInfoH;
          packet[17] = node.version;

          //Net and Sub switch are part of the Port-Address
          packet[18] = (uint8_t)(node.universe[univ][0] & 0x7F00) >> 8;       //Net switch :: Only bits 14 to 8 are relevant and should be placed in this byte.
          packet[19] = (uint8_t)(node.universe[univ][0] & 0x00F0) >> 4;       //SubNet switch :: Only bits 7 to 4 are relevant and should be placed in this byte.

          //Satus1 field
            // bit0 =1 UBEA present. ;; =0 UBEA not present or corrupt
            // bit1 =1 Capable of Remote Device Management (RDM). ;; =0 Not capable of Remote Device Management (RDM).
            // bit2 =1 Booted from ROM. ;; =0 Normal firmware boot (from flash).
            // bit3 =0 Not implemented, transmit as zero, receivers do not test.
            // bit5:4 =00 Port-Address Programming Authority unknown.. ;; =01 All Port-Address set by front panel controls. ;; =10 All or part of Port-Address programmed by network or Web browser. ;; =11 Not used.
            // bit7:8 =00 Indicator state unknown. ;; =01 Indicators in Locate / Identify Mode. ;; = 10 Indicators in Mute Mode. ;; =11 Indicators in Normal Mode.
          packet[23] = 0xC0;              //status1 field:: currently set to normal mode for display status.

          // Set node identification OEM code, ESTA Man
          packet[20] = node.oem >> 8;
          packet[21] = node.oem;
          packet[24] = node.etsaman;
          packet[25] = node.etsaman >> 8;

          //Set the long name fields max 18 bytes
          for(uint8_t i=0 ; i < sizeof(node.shortname) ; i++)
            packet[26+i] = node.shortname[i];

          //Set the long name fields max 64 bytes
          for(int i=0 ; i < 64 ; i++)
            packet[44+i] = node.longname[i];
        
          //Set the node report message
          //sprintf((char *)&packet[108],"#%d04 [%04X] %50s", node.nodeReportCode, node.pollReplyCounter, node.reportMsg);
          sprintf((char *)&packet[108],"#%04X [%04X] %50s", node.nodeReportCode, node.pollReplyCounter, node.reportMsg);
          
          if(node.pollReplyCounter == 9999)
            node.pollReplyCounter = 0;
          else
            node.pollReplyCounter++;
          
          // Set the NumPorts, this will be one since we are going to send a OpPortReply for each port as defined in the Art-Net 4 spec.
          packet[173] = 1;
          packet[174] = (node.universe[univ][1] == 0) ? 0x80 : 0x40;   // Set 0x80 is the port is an output or input
          packet[174] |= node.universe[univ][2];                       // Set the used protol.

          //Good input / good output
          packet[178] |= (node.universe[univ][3] >> 8);               //Upper byte represents good input
          packet[172] |= node.universe[univ][3];                      //Lower byte represents good output
        
          //Set Swin/Swout values
          if(node.universe[univ][1] == 1)                              //if the universe is an input universe then swin is set otherwise swout
            packet[186] |= (node.universe[univ][3] & 0x0F);           //Swin bits 3-0 represent a part of the 15 bits port address.
          else
            packet[190] |= (node.universe[univ][3] & 0x0F);           //Swout bits 3-0 represent a part of the 15 bits port address.

          //set the style of the node
          packet[200] = node.style;

          //Set the mac address
          for(int i=0 ; i < 6 ; i++)
            packet[201+i] = node.mac[i];

          //Set the ip address
          for(int i=0 ; i < 4 ; i++)
            packet[207+i] = node.ip[i];

          //Set the bindIndex, an incremental number depending on universe x of the MAN_NUM_UNIVERSES
          packet[211] = univ+1;     //this is an incremental number for the universe.
          
          //Status 2 field
            // bit0 =1 Product supports web browser configuration.
            // bit1 =1 Node’s IP is DHCP configured. ;; =0 Node’s IP is manually configured.
            // bit2 =1 Node is DHCP capable. ;; =0 Node is not DHCP capable.
            // bit3 =1 Node supports 15 bit Port-Address (Art-Net 3 or 4) ;; =0 Node supports 8 bit Port-Address (Art-Net II).
            // bit4 =1 Node is able to switch between Art-Net and sACN. ;; =0 Node not able to switch between Art-Net and sACN.
            // bit5 =1 squawking. ;; =0 Not squawking.
          packet[212] = (node.dchp == 1) ? 0x0E : 0x0C;       //If connected through DCHP the result is 0xE0 otherwise 0x0C bit 2 is the only relevant one.

          //Give the above prepaired buffer so I can be send out to the controller.
          if(!transferPacket(destinationIP, packet, packetsize))
            return 1;
      }
      return 0;

    // -- OpIpProgReply
    case ART_IPPROG_REPLY:
      return 0;

    // -- OpTodData (future expansion)
    case RDM_TOD_DATA:

    case RDM_SUB:
    default:
      return 0;
  } 
}

// **** Function Artnet::transderPacket() ****
// Descr: This function generates the packets and sends to the controller
// Return: 1 = success , 0 = fail
uint8_t Artnet::transferPacket(IPAddress destinationIP, uint8_t *packet, uint16_t size) 
{
  Udp.beginPacket(destinationIP, ART_NET_PORT);
  Udp.write((uint8_t *)packet, size);
  return Udp.endPacket(); 
}

// **** Function setNodeReportMsg(char msg[]) ****
// Descr: This sets the report message in the OpPollReply package to the controller. Very helpfull for debugging!
//        NOTE that the field is limited by 50 ASCII characters.
void Artnet::setNodeReportMsg(char *msg) 
{
  sprintf(node.reportMsg,"%50s", msg);   //set the message with padding spaces ending with a \0
}

// **** Function clearNodeReportMsg(char msg[]) ****
// Descr: This clear the report message in the OpPollReply package.
void Artnet::clearNodeReportMsg() 
{
  char msg = 0x20;
  sprintf(node.reportMsg,"%50c", msg);   //set space with padding spaces ending with a \0
}


// **** Function Artnet::maintainDCHP() ****
// Descr: This function maintains the network connection with DCHP. It is only valid to use this with DCHP enabled.
// Return:
//    0 = nothing has changed ; renewal succes ; rebind success
//    1 = Renew failed.
//    3 = Rebind failed.
uint16_t Artnet::maintainDCHP()
{
  switch(Ethernet.maintain()) {
    case 1:
      return 1;

    case 3:
      return 3;

    case 2:
    case 4:
    { 
      //IPAddress temp = getIP();
      for(int i=0 ; i < 4 ; i++)
        node.ip[i] = getIP()[i];
      return 0;
    }
    case 0:
    default: 
      return 0;
  }
}

// **** Function Artnet::getIP() ****
// Descr: Returns the IP address currently set in nodeIP.
// Return: is IPAddress type
IPAddress Artnet::getIP() 
{
  #if !defined(ARDUINO_SAMD_ZERO) && !defined(ESP8266) && !defined(ESP32)
    return Ethernet.localIP();
  #else
    return WiFi.localIP();
  #endif
}

// **** Function Artnet::loadDefaults() ****
// Descr: Enables the user to change the short description of the node.
// NOTE: Respect the max length of the field. It is only 18 ASCI characters.
void Artnet::loadDefaults() 
{
  node.version          = VersionInfoH;
  node.oem              = ART_OEM_CODE;
  node.style            = ART_ST_NODE;
  node.etsaman          = ETSI_DEV_CODE;         //This is an code defined by ETSI for development/experimental usage.
  node.nodeReportCode   = RC_PWR_OK;
  node.pollReplyCounter = 0;
  node.dchp             = false;

  strncpy((char*)node.shortname, defaultShortname, 18);
  strncpy((char*)node.longname, defaultLongname, 64);
  strncpy((char*)node.reportMsg, defaultRprtMsg, 51);

  for(int i=0 ; i < ART_NUM_UNIVERSES; i++) 
  {
    node.universe[i][0] = i;      //default set universe from 0 to ART_NUM_UNIVERSES
    node.universe[i][1] = 0;      //Set direction to output (which means art-net receive)
    node.universe[i][2] = 0;      //Set DMX as output type
    node.universe[i][3] = 0x80;   //Set goodoutput
  }
}

// **** Function Artnet::setShortDescr() ****
// Descr: Enables the user to change the short description of the node.
// NOTE: Respect the max length of the field. It is only 18 ASCI characters.
void Artnet::setShortDescr(char *sname) 
{

}

// **** Function Artnet::setLongDescr() ****
// Descr: Enables the user to change the long description of the node.
// NOTE: Respect the max length of the field. It is only 64 ASCI characters.
void Artnet::setLongDescr(char *lname) 
{

}

// **** Function Artnet::setCmd() ****
// Descr: This function is used to execute command received in FIELD 13.
// Return: Same value as inserted in case of succes. 0 when it failed.
uint8_t Artnet::setCmd(uint8_t cmd) 
{
  switch (cmd)
  {
  case ART_AC_RESET_RX:
    //clear those flags.
    for (int i = 0; i < ART_NUM_UNIVERSES; i++)
    {
      node.universe[i][3] &= 0x88;   //Clear Rx/Tx Flags
    }
    return ART_AC_RESET_RX;

  case ART_AC_CANCEL:
    return ART_AC_CANCEL;
  
  case ART_AC_LED_NORMAL:
    return ART_AC_LED_NORMAL;

  case ART_AC_LED_MUTE:
    return ART_AC_LED_NORMAL;

  case ART_AC_NONE:
    return ART_AC_NONE;
  
  default:
    return cmd;
  }
}
