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

Artnet::Artnet() {}

void Artnet::begin(byte mac[], byte ip[])
{
  #if !defined(ARDUINO_SAMD_ZERO) && !defined(ESP8266) && !defined(ESP32)
    Ethernet.begin(mac,ip);
  #endif
  DCHP = false;
  //load the specified mac address into the ArtPollReply.mac[] array.
  memcpy(ArtPollReply.mac, mac, 6);
  Udp.begin(ART_NET_PORT);
}

void Artnet::begin(byte mac[])
{
  #if !defined(ARDUINO_SAMD_ZERO) && !defined(ESP8266) && !defined(ESP32)
    Ethernet.begin(mac);
  #endif

  DCHP = true;
  //load the specified mac address into the ArtPollReply.mac[] array.
  memcpy(ArtPollReply.mac, mac, 6);

  Udp.begin(ART_NET_PORT);
}

// NO MAC ADDRESS SET, REQUIRED!! no to update!
void Artnet::begin()
{
  DCHP = false;
  Udp.begin(ART_NET_PORT);
}

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
//    In all other cases 0.
uint16_t Artnet::read()
{
  packetSize = Udp.parsePacket();
  controllerIP = Udp.remoteIP();

  if(packetSize <= MAX_BUFFER_ARTNET && packetSize > 0)
  {
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
          sequence = artnetPacket[12];
          incomingUniverse = artnetPacket[14] | artnetPacket[15] << 8;
          dmxDataLength = artnetPacket[17] | artnetPacket[16] << 8;

          if (artDmxCallback) 
            (*artDmxCallback)(incomingUniverse, dmxDataLength, sequence, artnetPacket + ART_DMX_START, controllerIP);
          return ART_DMX;

        // -- OpPoll received, now we have to respond with an OpPollReply message within 3 seconds.
        // fill the reply struct, and then send it to the network's broadcast address
        case ART_POLL:
          sendArtPollReply();
          return ART_POLL;
        
        case ART_SYNC:
          if (artSyncCallback) 
            (*artSyncCallback)(controllerIP);
          return ART_SYNC;
        
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

void Artnet::setDefaults()
{
  // FIELD 1:: Array of 8 characters, the final character is a null termination. (Always 'A' 'r' 't' '-' 'N' 'e' 't' 0x00)
    sprintf((char *)id, "Art-Net");
    memcpy(ArtPollReply.id, id, sizeof(ArtPollReply.id));

  // FIELD 4:: Port - The Port is always 0x1936
    ArtPollReply.port =  ART_NET_PORT;
  
  //All other fields
    ArtPollReply.etsaman[0] = 0;                  //ESTA manufacturer code Lo
    ArtPollReply.etsaman[1] = 0;                  //ESTA manufacturer code Hi
    ArtPollReply.verH       = VersionInfoH;       //Use verH for library version
    ArtPollReply.verL       = 0;                  // User can set this value, default it is 0.
    ArtPollReply.subH       = 0;                  
    ArtPollReply.sub        = 0;                  
    ArtPollReply.oemH       = 0;                  // This device has no OEM code                          
    ArtPollReply.oemL       = 0xFF;               // This device has no OEM code 
    ArtPollReply.ubea       = 0;                  // We don't use User Bios Extension Area therefor set to 0               
    ArtPollReply.swvideo    = 0;                  // Set to 00 when video display is showing local data.               
    ArtPollReply.swmacro    = 0;                  // Set to 0 no marcro are used
    ArtPollReply.swremote   = 0;                  // Set to 0 no remote triggers are used
    ArtPollReply.style      = ART_ST_NODE;        // Set style to Standard node
    ArtPollReply.numbportsH = 0;
    ArtPollReply.numbports  = 4;

    // FIELD 15,16:: Short and long name.
    uint8_t shortname [18];
    uint8_t longname [64];
    sprintf((char *)shortname, "artnet arduino");
    sprintf((char *)longname, "Art-Net -> Arduino Bridge");
    memcpy(ArtPollReply.shortname, shortname, sizeof(shortname));
    memcpy(ArtPollReply.longname, longname, sizeof(longname));

    //Satus1 field #12 of the OpPollReply packet
    // bit0 =1 UBEA present. ;; =0 UBEA not present or corrupt
    // bit1 =1 Capable of Remote Device Management (RDM). ;; =0 Not capable of Remote Device Management (RDM).
    // bit2 =1 Booted from ROM. ;; =0 Normal firmware boot (from flash).
    // bit3 =0 Not implemented, transmit as zero, receivers do not test.
    // bit5:4 =00 Port-Address Programming Authority unknown.. ;; =01 All Port-Address set by front panel controls. ;; =10 All or part of Port-Address programmed by network or Web browser. ;; =11 Not used.
    // bit7:8 =00 Indicator state unknown. ;; =01 Indicators in Locate / Identify Mode. ;; = 10 Indicators in Mute Mode. ;; =11 Indicators in Normal Mode.
    ArtPollReply.status1     = 0xd2;

    //Satus 2 field #40 of the OpPollReply packet
    // bit0 =1 Product supports web browser configuration.
    // bit1 =1 Node’s IP is DHCP configured. ;; =0 Node’s IP is manually configured.
    // bit2 =1 Node is DHCP capable. ;; =0 Node is not DHCP capable.
    // bit3 =1 Node supports 15 bit Port-Address (Art-Net 3 or 4) ;; =0 Node supports 8 bit Port-Address (Art-Net II).
    // bit4 =1 Node is able to switch between Art-Net and sACN. ;; =0 Node not able to switch between Art-Net and sACN.
    // bit5 =1 squawking. ;; =0 Not squawking.
    ArtPollReply.status2    = 0x08;
}

void Artnet::sendArtPollReply() 
{
  if(DEBUG) {
      Serial.print("POLL from ");
      Serial.print(controllerIP);
      Serial.print(" broadcast addr: ");
      Serial.println(broadcastIP);
  }

  // FIELD 2:: OpCode = OpPollReply since this is the poll reply
  ArtPollReply.opCode = ART_POLL_REPLY;

  // FIELD 3:: Get the node's local IP address. And put it in the message.
  //memcpy(ArtPollReply.ip, nodeIP, sizeof(ArtPollReply.ip));

  // FIELD 38:: Get the node's local IP address. And put it in the message.
  //memcpy(ArtPollReply.bindip, nodeIP, sizeof(ArtPollReply.bindip));
  
  // FIELD 5,6:: VersInfoH, VersInfoL are specified in the ARtnet.h file.
  memset(ArtPollReply.goodinput,  0x08, 4); // incorrect 0x80 means receiving input data without issue
  memset(ArtPollReply.goodoutput,  0x80, 4); // correct 0x80 means transmitting output data without issue
  memset(ArtPollReply.porttypes,  0xc0, 4); //zou eigenlijk 0x80 moeten zijn aangezien er geen inputs nodig zijn.

  uint8_t swin[4]  = {0x01,0x02,0x03,0x04};
  uint8_t swout[4] = {0x01,0x02,0x03,0x04};
  for(uint8_t i = 0; i < 4; i++)
  {
      ArtPollReply.swout[i] = swout[i];
      ArtPollReply.swin[i] = swin[i];
  }

  // FIELD 17:: NodeReport 
  // Currently not compliant the Art-Net 4 specification (refer to page 21/22)
  sprintf((char *)ArtPollReply.nodereport, "%i DMX output universes active.", ArtPollReply.numbports);
  
  // UDP SEND :: All fields are filled in, now we can send the package to the controller using the boardcast IP.
  //BUGFIX: According to Art-Net 4 the node must reply to the controller that send out the poll. Only the controller is allowed to send out broadcast opPoll's.
  Udp.beginPacket(controllerIP, ART_NET_PORT);
  Udp.write((uint8_t *)&ArtPollReply, sizeof(ArtPollReply));
  Udp.endPacket();
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
      nodeIP = getIP();
    case 0:
    default: 
      return 0;
  }
}

// **** Function Artnet::getIP() ****
// Descr: Returns the IP address currently set in nodeIP.
// Return: is IPAddress type
IPAddress Artnet::getIP() {
  #if !defined(ARDUINO_SAMD_ZERO) && !defined(ESP8266) && !defined(ESP32)
    return Ethernet.localIP();
  #else
    return WiFi.localIP();
  #endif
}
