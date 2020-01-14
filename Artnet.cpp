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
  DcHp = false;
  Udp.begin(ART_NET_PORT);
}

void Artnet::begin(byte mac[], byte ip[])
{
  #if !defined(ARDUINO_SAMD_ZERO) && !defined(ESP8266) && !defined(ESP32)
    Ethernet.begin(mac);
    DcHp = true;
  #endif

  Udp.begin(ART_NET_PORT);
}

void Artnet::begin()
{
  DcHp = false;
  Udp.begin(ART_NET_PORT);
}

void Artnet::setBroadcast(byte bc[])
{
  //sets the broadcast address
  broadcast = bc;
}
void Artnet::setBroadcast(IPAddress bc)
{
  //sets the broadcast address
  broadcast = bc;
}

// **** Function Artnet::read() ****
// Descr: This function parses the received package, checks wether it is valid
// Return:
//    In case a supported opcode was received the opcode is returned.
//    In all other cases 0.
uint16_t Artnet::read()
{
  packetSize = Udp.parsePacket();

  remoteIP = Udp.remoteIP();
  if (packetSize <= MAX_BUFFER_ARTNET && packetSize > 0)
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
        case OpDmx:
          sequence = artnetPacket[12];
          incomingUniverse = artnetPacket[14] | artnetPacket[15] << 8;
          dmxDataLength = artnetPacket[17] | artnetPacket[16] << 8;

          // When an artDmxCallback is specified we call this function.
          if (artDmxCallback) (*artDmxCallback)(incomingUniverse, dmxDataLength, sequence, artnetPacket + ART_DMX_START, remoteIP);
          
          // Return that we received ad OpDmx OpOutput packet.
          return OpDmx;


        // -- OpPoll received, now we have to respond with an OpPollReply message within 3 seconds.
        // fill the reply struct, and then send it to the network's broadcast address
        case OpPoll:
            if(DEBUG) {
              Serial.print("POLL from ");
              Serial.print(remoteIP);
              Serial.print(" broadcast addr: ");
              Serial.println(broadcast);
            }

          // FIELD 1:: Array of 8 characters, the final character is a null termination. (Always 'A' 'r' 't' '-' 'N' 'e' 't' 0x00)
          sprintf((char *)id, "Art-Net");
          memcpy(ArtPollReply.id, id, sizeof(ArtPollReply.id));

          // FIELD 2:: OpCode = OpPollReply since this is the poll reply
          ArtPollReply.opCode = OpPollReply;

          // FIELD 3:: Get the node's local IP address. And put it in the message.
          #if !defined(ARDUINO_SAMD_ZERO) && !defined(ESP8266) && !defined(ESP32)
            IPAddress local_ip = Ethernet.localIP();
          #else
            IPAddress local_ip = WiFi.localIP();
          #endif

          node_ip_address[0] = local_ip[0];
          node_ip_address[1] = local_ip[1];
          node_ip_address[2] = local_ip[2];
          node_ip_address[3] = local_ip[3];
          memcpy(ArtPollReply.ip, node_ip_address, sizeof(ArtPollReply.ip));

          // FIELD 4:: Port - The Port is always 0x1936
          ArtPollReply.port =  ART_NET_PORT;

          // FIELD 5,6:: VersInfoH, VersInfoL are specified in the ARtnet.h file.
          
          // FIELD 5,6:: VersInfoH, VersInfoL are specified in the ARtnet.h file.
          memset(ArtPollReply.goodinput,  0x08, 4);
          memset(ArtPollReply.goodoutput,  0x80, 4);
          memset(ArtPollReply.porttypes,  0xc0, 4);

          // FIELD 15,16:: Short and long name.
          uint8_t shortname [18];
          uint8_t longname [64];
          sprintf((char *)shortname, "artnet arduino");
          sprintf((char *)longname, "Art-Net -> Arduino Bridge");
          memcpy(ArtPollReply.shortname, shortname, sizeof(shortname));
          memcpy(ArtPollReply.longname, longname, sizeof(longname));

          ArtPollReply.etsaman[0] = 0;
          ArtPollReply.etsaman[1] = 0;
          ArtPollReply.verH       = 1;
          ArtPollReply.ver        = 0;
          ArtPollReply.subH       = 0;
          ArtPollReply.sub        = 0;
          ArtPollReply.oemH       = 0;
          ArtPollReply.oem        = 0xFF;
          ArtPollReply.ubea       = 0;
          ArtPollReply.status     = 0xd2;
          ArtPollReply.swvideo    = 0;
          ArtPollReply.swmacro    = 0;
          ArtPollReply.swremote   = 0;
          ArtPollReply.style      = 0;

          ArtPollReply.numbportsH = 0;
          ArtPollReply.numbports  = 4;
          ArtPollReply.status2    = 0x08;

          ArtPollReply.bindip[0] = node_ip_address[0];
          ArtPollReply.bindip[1] = node_ip_address[1];
          ArtPollReply.bindip[2] = node_ip_address[2];
          ArtPollReply.bindip[3] = node_ip_address[3];

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
          Udp.beginPacket(broadcast, ART_NET_PORT);
          Udp.write((uint8_t *)&ArtPollReply, sizeof(ArtPollReply));
          Udp.endPacket();

          return OpPoll;

        case OpSync:
          if (artSyncCallback) (*artSyncCallback)(remoteIP);
        
          return OpSync;
        
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
    ArtPollReply.style      = StNode;             // Set style to Standard node
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

void sendArtPollReply() 
{

}
