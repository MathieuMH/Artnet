/*
This is a basic example that will print out the header and the content of an ArtDmx packet.
This example uses the read() function and the different getter functions to read the data.
This example may be copied under the terms of the MIT license, see the LICENSE file for details
*/

#include <Artnet.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <SPI.h>

// Checked with schematic
//#define WIZ_CS        0
#define WIZ_RESET     1
//#define WIZ_INT       16
#define HAPPY_LED     14
//#define FRAME_SYNC    17
//#define WIZ_MOSI      11
//#define WIZ_MISO      12
//#define WIZ_SCK       13

//#define DEBUG         true

Artnet artnet;

// Change ip and mac address for your setup
//byte ip[] = {192, 168, 0, 105};
//byte gateway[] = {192, 168, 0, 1};
//byte dns[] = {192, 168, 0, 1};
//byte subnet[] = {255, 0, 0, 0};
byte broadcast[] = {192, 255, 255, 255};
byte mac[] = {0x04, 0xE9, 0xE5, 0x00, 0x69, 0xEC};

void setup()
{
  // Port directions
  pinMode(HAPPY_LED, OUTPUT);
  pinMode(WIZ_RESET, OUTPUT);
  //pinMode(WIZ_CS, OUTPUT);
  //pinMode(WIZ_INT, INPUT);
  //pinMode(FRAME_SYNC, INPUT);

  // Port Init
  //digitalWrite(WIZ_CS, HIGH);     // De-select Wiz850io
  digitalWrite(WIZ_RESET, LOW);   //Start Wiz850io Reset
  //delay(50);
  digitalWrite(WIZ_RESET, HIGH);   //End Wiz850io Reset
  //digitalWrite(WIZ_CS, LOW);      // Select Wiz850io
  digitalWrite(HAPPY_LED, HIGH);
  
  
  Serial.begin(115200);
  while (!Serial) 
  {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println( "Compiled: " __DATE__ ", " __TIME__ ", " __VERSION__);

  Ethernet.init(0);
  //Ethernet.begin(mac, ip, dns, gateway, subnet);
  delay(50);
  artnet.begin(mac);
  //artnet.begin(mac, ip);
  artnet.setBroadcast(broadcast);
  delay(100);
  
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
