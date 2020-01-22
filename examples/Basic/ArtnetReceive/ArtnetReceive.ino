/*
This is a basic example that will print out the header and the content of an ArtDmx packet.
This example uses the read() function and the different getter functions to read the data.
This example may be copied under the terms of the MIT license, see the LICENSE file for details
*/

#include <Artnet.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <SPI.h>

#define WIZ_RESET     1

//#define DEBUG         true

Artnet artnet;

//Set broadcast and mac address according to your setup.
byte broadcast[] = {192, 255, 255, 255};
byte mac[] = {0x04, 0xE9, 0xE5, 0x00, 0x69, 0xEC};

void setup()
{
  // Port directions
  pinMode(WIZ_RESET, OUTPUT);

  // Port Init
  digitalWrite(WIZ_RESET, LOW);   //Start Wiz850io Reset
  delay(5);
  digitalWrite(WIZ_RESET, HIGH);   //End Wiz850io Reset
  
  Serial.begin(115200);
  while (!Serial) 
  {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println( "Compiled: " __DATE__ ", " __TIME__ ", " __VERSION__);

  delay(5);
  artnet.begin(mac);
  artnet.setBroadcast(broadcast);
  delay(10);
  
  //Check if Wiz850io is present
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("No Wiz850io (or 820io) found. Sorry, can't run without hardware. :(");
    while (true) {
      delay(1); // do nothing, no point running without Ethernet hardware
    }
  }
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet cable is not connected.");
  }
}

void loop()
{
  uint16_t r = artnet.read();
  if(r == ART_POLL)
  {
    Serial.println("POLL");
  }
  if(r == ART_DMX)
  {
    // print out our data
    Serial.print("universe number = ");
    Serial.print(artnet.getUniverse());
    Serial.print("\tdata length = ");
    Serial.print(artnet.getLength());
    Serial.print("\tIP = ");
    Serial.print(artnet.getIP());
    Serial.print("\tsequence n0. = ");
    Serial.println(artnet.getSequence());
    Serial.print("DMX data: ");
    for (int i = 0 ; i < artnet.getLength() ; i++)
    {
      Serial.print(artnet.getDmxFrame()[i]);
      Serial.print("  ");
    }
    Serial.println();
    Serial.println();
  }
}
