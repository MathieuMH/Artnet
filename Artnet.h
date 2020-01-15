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

#ifndef ARTNET_H
#define ARTNET_H

#include <Arduino.h>

#if defined(ARDUINO_SAMD_ZERO)
    #include <WiFi101.h>
    #include <WiFiUdp.h>
#elif defined(ESP8266)
    #include <ESP8266WiFi.h>
    #include <WiFiUdp.h>
#elif defined(ESP32)
    #include <WiFi.h>
    #include <WiFiUdp.h>
#else
    #include <Ethernet.h>
    #include <EthernetUdp.h>
#endif

#ifndef defined(DEBUG)
    #define DEBUG false
#endif

// Art-Net Library information
const uint8_t  VersionInfoH       = 0x01;
//const uint8_t  VersionInfoL       0x01;     //VersionInfoL is not used. The user can use this to set its own version info.

// *** Art-Net packet related paramters
#define   ART_NET_PORT            0x1936    // Art-Net default port = 0x1936 = 6454 (DO NOT CHANGE!!)
#define   MAX_BUFFER_ARTNET       530       // Maximum buffer size.                

// Packet
#define   ART_NET_ID              "Art-Net\0"    //Every Art-Net package is obligate to carry the packet ID ‘A’ ‘r’ ‘t’ ‘-‘ ‘N’ ‘e’ ‘t’ 0x00
#define   ART_DMX_START           18             //Start byte of the DMX data in the ArtDmx packet.

  // *** Art-Net Opcodes
#define  ART_POLL                 0x2000      //This is an ArtPoll packet, no other data is contained in this UDP packet.
#define  ART_POLL_REPLY           0x2100      //This is an ArtPollReply Packet. It contains device status information.
#define  ART_DIAG_DATA            0x2300      //Diagnostics and data logging packet.
#define  ART_COMMAND              0x2400      //Used to send text based parameter commands.
#define  ART_DMX                  0x5000      //This is an ArtDmx data packet. It contains zero start code DMX512 information for a single Universe.
#define  ART_NZS                  0x5100      //This is an ArtNzs data packet. It contains non-zero start code (except RDM) DMX512 information for a single Universe.
#define  ART_SYNC                 0x5200      //This is an ArtSync data packet. It is used to force synchronous transfer of ArtDmx packets to a node’s output.
#define  ART_ADDRESS              0x6000      //This is an ArtAddress packet. It contains remote programming information for a Node.
#define  ART_INPUT                0x7000      //This is an ArtInput packet. It contains enable – disable data for DMX inputs.
#define  RDM_TOD_REQ              0x8000      //This is an ArtTodRequest packet. It is used to request a Table of Devices (ToD) for RDM discovery.
#define  RDM_TOD_DATA             0x8100      //This is an ArtTodData packet. It is used to send a Table of Devices (ToD) for RDM discovery.
#define  RDM_TOD_CONTROL          0x8200      //This is an ArtTodControl packet. It is used to send RDM discovery control messages.
#define  RDM                      0x8300      //This is an ArtRdm packet. It is used to send all non discovery RDM messages.
#define  RDM_SUB                  0x8400      //This is an ArtRdmSub packet. It is used to send compressed, RDM Sub-Device data.
#define  ART_VIDEO_SETUP          0xa010      //This is an ArtVideoSetup packet. It contains video screen setup information for nodes that implement the extended video features.
#define  ART_VIDEO_PALLET         0xa020      //This is an ArtVideoPalette packet. It contains colour palette setup information for nodes that implement the extended video features.
#define  ART_VIDEO_DATA           0xa040      //This is an ArtVideoData packet. It contains display data for nodes that implement the extended video features.
#define  ART_MAC_MASTER           0xf000      //This packet is deprecated.
#define  ART_MAC_SLAVE            0xf100      //This packet is deprecated.
#define  ART_FW_MASTER            0xf200      //This is an ArtFirmwareMaster packet. It is used to upload new firmware or firmware extensions to the Node.
#define  ART_FW__REPLY            0xf300      //This is an ArtFirmwareReply packet. It is returned by the node to acknowledge receipt of an ArtFirmwareMaster packet or ArtFileTnMaster packet.
#define  ART_FW_TNMASTER          0xf400      //Uploads user file to node
#define  ART_FW_FNMASTER          0xf500      //Downloads user file from node.
#define  ART_FW_FNREPLY           0xf600      //Server to Node acknowledge for download packets.
#define  ART_IPPROG               0xf800      //This is an ArtIpProg packet. It is used to re-programme the IP address and Mask of the Node.
#define  ART_IPPROG_REPLY         0xf900      //This is an ArtIpProgReply packet. It is returned by the node to acknowledge receipt of an ArtIpProg packet.
#define  ART_MEDIA                0x9000      //This is an ArtMedia packet. It is Unicast by a Media Server and acted upon by a Controller.
#define  ART_MEDIA_PATCH          0x9100      //This is an ArtMediaPatch packet. It is Unicast by a Controller and acted upon by a Media Server.
#define  ART_MEDIA_CNTRL          0x9200      //This is an ArtMediaControl packet. It is Unicast by a Controller and acted upon by a Media Server.
#define  ART_MEDIA_CNTRL_REPLY    0x9300      //This is an ArtMediaControlReply packet. It is Unicast by a Media Server and acted upon by a Controller.
#define  ART_TIME_CODE            0x9700      //This is an ArtTimeCode packet. It is used to transport time code over the network.
#define  ART_TIME_SYNC            0x9800      //Used to synchronise real time date and clock
#define  ART_TRIGGER              0x9900      //Used to send trigger macros
#define  ART_DIRECTORY            0x9900      //Requests a node's file list
#define  ART_DIRECTORY_REPLY      0x9900      //Replies to OpDirectory with file list

// *** Art-Net NodeReport Codes (Used in the ArtPollReply)
#define RC_DEBUG                  0x0000      //Booted in debug mode (Only used in development)
#define RC_PWR_OK                 0x0001      //Power On Tests successful
#define RC_PWR_FAIL               0x0002      //Hardware tests failed at Power On
#define RC_SOCKET_WR1             0x0003      //Last UDP from Node failed due to truncated length, Most likely caused by a collision.
#define RC_PARSE_FAIL             0x0004      //Unable to identify last UDP transmission. Check OpCode and packet length.
#define RC_UD¨P_FAIL              0x0005      //Unable to open Udp Socket in last transmission attempt
#define RC_SHNAME_OK              0x0006      //Confirms that Short Name programming via ArtAddress, was successful.
#define RC_LONAME_OK              0x0007      //Confirms that Long Name programming via ArtAddress, was successful.
#define RC_DMX_ERROR              0x0008      //DMX512 receive errors detected.
#define RC_DMX_TX_FULL            0x0009      //Ran out of internal DMX transmit buffers.
#define RC_DMX_RX_FULL            0x000a      //Ran out of internal DMX Rx buffers.
#define RC_SWITCH_ERROR           0x000b      //Rx Universe switches conflict.
#define RC_CONFIG_ERROR           0x000c      //Product configuration does not match firmware.
#define RC_DMX_SHORT              0x000d      //DMX output short detected. See GoodOutput field.
#define RC_FW_FAIL                0x000e      //Last attempt to upload new firmware failed.
#define RC_USER_FAIL              0x000f      //User changed switch settings when address locked by remote programming. User changes ignored.
#define RC_FACTORY_RESET          0x0010      //Factory reset has occurred.

// *** Art-Net Styles
#define ART_ST_NODE               0x00        //A DMX to / from Art-Net device (RECOMMENDED)
#define ART_ST_CNTRL              0x01        //A lighting console.
#define ART_ST_MEDIA              0x02        //A Media Server.
#define ART_ST_ROUTE              0x03        //A network routing device.
#define ART_ST_BACKUP             0x04        //A backup device.
#define ART_ST_CONFIG             0x05        //A configuration or diagnostic tool.
#define ART_ST_VISUAL             0x06        //A visualiser.

struct artnet_reply_s {
  uint8_t  id[8];           //#1 Array of 8 characters, the final character is a null termination. Value = ‘A’ ‘r’ ‘t’ ‘-‘ ‘N’ ‘e’ ‘t’ 0x00
  uint16_t opCode;          //#2 OpPollRepl (Transmitted low byte first.)
  uint8_t  ip[4];           //#3 Array containing the Node’s IP address. First array entry is most significant byte of address.
  uint16_t port;            //#4 The Port is always 0x1936 (Transmitted low byte first.)
  uint8_t  verH;            //#5 High byte of Node’s firmware revision number. The Controller should only use this field to decide if a firmware update should proceed. The convention is that a higher number is a more recent release of firmware.
  uint8_t  verL;            //#6 Low byte of Node’s firmware revision number.
  uint8_t  subH;            //#7 Bits 14-8 of the 15 bit Port-Address are encoded into the bottom 7 bits of this field. This is used in combination with SubSwitch and SwIn[] or SwOut[] to produce the full universe address.
  uint8_t  sub;             //#8 Bits 7-4 of the 15 bit Port-Address are encoded into the bottom 4 bits of this field. This is used in combination with NetSwitch and SwIn[] or SwOut[] to produce the full universe address.
  uint8_t  oemH;            //#9 The high byte of the Oem value.
  uint8_t  oemL;            //#10 The low byte of the Oem value. The Oem word describes the equipment vendor and the feature set available. Bit 15 high indicates extended features available. Current registered codes are defined in Table 2.
  uint8_t  ubea;            //#11 This field contains the firmware version of the User Bios Extension Area (UBEA). If the UBEA is not programmed, this field contains zero.
  uint8_t  status1;         //#12 General Status register containing bit fields refer to Art-Net 4 specification page 20.
  uint8_t  etsaman[2];      //#13,14 The ESTA manufacturer code. These codes are used to represent equipment manufacturer. They are assigned by ESTA. This field can be interpreted as two ASCII bytes representing the manufacturer initials.
  uint8_t  shortname[18];   //#15 The array represents a null terminated short name for the Node. The Controller uses the ArtAddress packet to program this string. Max length is 17 characters plus the null. This is a fixed length field, although the string it contains can be shorter than the field.
  uint8_t  longname[64];    //#16 The array represents a null terminated long name for the Node. The Controller uses the ArtAddress packet to program this string. Max length is 63 characters plus the null. This is a fixed length field, although the string it contains can be shorter than the field.
  uint8_t  nodereport[64];  //#17 The array is a textual report of the Node’s operating status or operational errors. It is primarily intended for ‘engineering’ data.
  uint8_t  numbportsH;      //#18 The high byte of the word describing the number of input or output ports. The high byte is for future expansion and is currently zero.
  uint8_t  numbports;       //#19 The low byte of the word describing the number of input or output ports. If number of inputs is not equal to number of outputs, the largest value is taken. Zero is a legal value if no input or output ports are implemented. The maximum value is 4. Nodes can ignore this field as the information is implicit in PortTypes[].
  uint8_t  porttypes[4];    //#20 This array defines the operation and protocol of each channel. (A product with 4 inputs and 4 outputs would report 0xc0, 0xc0, 0xc0, 0xc0). The array length is fixed, independent of the number of inputs or outputs physically available on the Node.
  uint8_t  goodinput[4];    //#21 This array defines input status of the node.
  uint8_t  goodoutput[4];   //#22 This array defines output status of the node.
  uint8_t  swin[4];         //#23 Bits 3-0 of the 15 bit Port-Address for each of the 4 possible input ports are encoded into the low nibble.
  uint8_t  swout[4];        //#24 Bits 3-0 of the 15 bit Port-Address for each of the 4 possible output ports are encoded into the low nibble.
  uint8_t  swvideo;         //#25 Set to 00 when video display is showing local data. Set to 01 when video is showing ethernet data. The field is now deprecated
  uint8_t  swmacro;         //#26 If the Node supports macro key inputs, this byte represents the trigger values. The Node is responsible for ‘debouncing’ inputs. When the ArtPollReply is set to transmit
  uint8_t  swremote;        //#27 If the Node supports remote trigger inputs, this byte represents the trigger values. The Node is responsible for ‘debouncing’ inputs.
  uint8_t  sp1;             //#28 Spare:: not used set to zero
  uint8_t  sp2;             //#29 Spare:: not used set to zero
  uint8_t  sp3;             //#30 Spare:: not used set to zero  
  uint8_t  style;           //#31 The Style code defines the equipment style of the device. See Table 4 on page x for current Style codes.
  uint8_t  mac[6];          //#32,33, 34, 35,36, 37 Mac Address
  uint8_t  bindip[4];       //#38 If this unit is part of a larger or modular product, this is the IP of the root device.
  uint8_t  bindindex;       //#39 This number represents the order of bound devices. A lower number means closer to root device. A value of 1 means root device.
  uint8_t  status2;         //#40 Satus 2 field, refer to Art-Net Specification p25.
  uint8_t  filler[26];      //#41 Transmit as zero. For future expansion.
} __attribute__((packed));

class Artnet
{
  public:
    Artnet();

      void begin(byte mac[], byte ip[]);
      void begin();
      void setBroadcast(byte bc[]);
      void setBroadcast(IPAddress bc);
      uint16_t read();
      void printPacketHeader();
      void printPacketContent();

    // **** Function Artnet::setgetDmxFrame() ****
    // Descr: This function allows the user to get the pointer to the DMX data
    inline uint8_t* getDmxFrame(void)
    {
      return artnetPacket + ART_DMX_START;
    }

    // **** Function Artnet::getOpcode() ****
    // Descr: This function returns the last received opcode
    inline uint16_t getOpcode(void)
    {
      return opcode;
    }
    
    // **** Function Artnet::getSequence() ****
    // Descr: This function returns the last received sequence
    inline uint8_t getSequence(void)
    {
      return sequence;
    }

    // **** Function Artnet::getUniverse() ****
    // Descr: This function returns the last received universe
    inline uint16_t getUniverse(void)
    {
      return incomingUniverse;
    }

    // **** Function Artnet::getLength() ****
    // Descr: This function returns the length of the received data DMX data packet.
    inline uint16_t getLength(void)
    {
      return dmxDataLength;
    }

    // **** Function Artnet::getControllerIP() ****
    // Descr: This function returns IP address of the controller from the most recent OpPoll.
    inline IPAddress getControllerIP(void)
    {
      return controllerIP;
    }

    // **** Function Artnet::setArtDmxCallback() ****
    // Descr: ---
    inline void setArtDmxCallback(void (*fptr)(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data, IPAddress controllerIP))
    {
      artDmxCallback = fptr;
    }

    // **** Function Artnet::setArtDmxCallback() ****
    // Descr: This function sets the rountine that is to be called once a OpSync command was received.
    inline void setArtSyncCallback(void (*fptr)(IPAddress controllerIP))
    {
      artSyncCallback = fptr;
    }

    // **** Function Artnet::getDchpStatus() ****
    // Descr: Returns current dchp status.
    inline uint16_t getDchpStatus(void)
    {
      return (uint16_t)dchpStatus;
    }

  private:
    //Create a struct for this node.
/*     struct artnet_node {
      uint8_t  ip[4];           //Array containing the Node’s IP address. First array entry is most significant byte of address.
      uint8_t  mac[6];          //Mac Address
      uint8_t  FWverH;          //High byte of Node’s firmware revision number. The Controller should only use this field to decide if a firmware update should proceed. The convention is that a higher number is a more recent release of firmware.
      uint8_t  FWverL;          //Low byte of Node’s firmware revision number.
      uint8_t  oemH;            //The high byte of the Oem value.
      uint8_t  oemL;            //The low byte of the Oem value. The Oem word describes the equipment vendor and the feature set available. Bit 15 high indicates extended features available. Current registered codes are defined in Table 2.
      uint8_t  ubea = 0;        //This field contains the firmware version of the User Bios Extension Area (UBEA). If the UBEA is not programmed, this field contains zero.
      uint8_t  etsaman[2];      //The ESTA manufacturer code. These codes are used to represent equipment manufacturer. They are assigned by ESTA. This field can be interpreted as two ASCII bytes representing the manufacturer initials.
      uint8_t  shortname[18];   //The array represents a null terminated short name for the Node. The Controller uses the ArtAddress packet to program this string. Max length is 17 characters plus the null. This is a fixed length field, although the string it contains can be shorter than the field.
      uint8_t  longname[64];    //The array represents a null terminated long name for the Node. The Controller uses the ArtAddress packet to program this string. Max length is 63 characters plus the null. This is a fixed length field, although the string it contains can be shorter than the field.
      uint8_t  style;           //#The Style code defines the equipment style of the device. See Table 4 on page x for current Style codes.
    } node; */

    uint8_t  node_ip_address[4];
    uint8_t  id[8];
  
    #if defined(ARDUINO_SAMD_ZERO) || defined(ESP8266) || defined(ESP32)
      WiFiUDP Udp;
    #else
      EthernetUDP Udp;
    #endif
  
    // Create the Art-Net ArtPollReply packet
    struct artnet_reply_s ArtPollReply;

    // Create global private variables
    uint8_t   artnetPacket[MAX_BUFFER_ARTNET];
    uint8_t   sequence;
    uint16_t  packetSize;
    uint16_t  opcode;
    uint16_t  incomingUniverse;
    uint16_t  dmxDataLength;
    enum Dchp {noChange, renewFailed, renewSucces, rebindFail, rebindSucces};
    Dchp  dchpStatus;
    bool DCHP;

    //Create nodeIP, broadcastIP and controllerIP address
    IPAddress nodeIP;
    IPAddress broadcastIP;
    IPAddress controllerIP;

    void (*artDmxCallback)(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data, IPAddress controllerIP);
    void (*artSyncCallback)(IPAddress controllerIP);
    void setDefaults();
    inline uint16_t maintainDCHP();
    inline IPAddress getIP();

};

#endif
