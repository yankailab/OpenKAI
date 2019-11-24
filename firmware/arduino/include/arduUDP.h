#ifndef ARDU_UDP_h
#define ARDU_UDP_h

#include <Ethernet.h>
#include <EthernetUdp.h>

// IO override for Ethernet
// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:

byte g_mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress g_myIP(192, 168, 8, 250);
unsigned int g_myPort = 8888;
EthernetUDP g_UDP;

bool ioAvailable(void)
{
  return g_UDP.available();
}

byte ioRead(void)
{
  return g_UDP.read();
}

void setupUDP(void)
{
  // configure the CS pin
  //Ethernet.init(10);  // Most Arduino shields

  while (1)
  {
    Ethernet.begin(g_mac, g_myIP);

    if (Ethernet.hardwareStatus() == EthernetNoHardware)
    {
      Serial.println("Ethernet device not found");
      continue;
    }

    if (Ethernet.hardwareStatus() == EthernetW5100)
      Serial.println("W5100");
    else if (Ethernet.hardwareStatus() == EthernetW5200)
      Serial.println("W5200");
    else if (Ethernet.hardwareStatus() == EthernetW5500)
      Serial.println("W5500");
    else
      Serial.println("Unknown Ethernet");

    if (Ethernet.linkStatus() == LinkOFF)
    {
      Serial.println("Ethernet cable is not connected.");
      continue;
    }

    break;
  }

  g_UDP.begin(g_myPort);
  g_nMsg = 2;

}

#endif
