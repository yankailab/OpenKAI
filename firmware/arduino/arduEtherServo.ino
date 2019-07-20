#include <Ethernet.h>
#include <EthernetUdp.h>
#include <Servo.h>
#include "arduOK.h"

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress g_myIP(192, 168, 1, 177);
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

Servo g_servo1;
Servo g_servo2;
uint16_t g_pwm1;
uint16_t g_pwm2;

void runCMD(void)
{
  switch (g_cmd.m_pBuf[1])
  {
    case ARDU_CMD_PWM:
      g_pwm1 = *((uint16_t*)(&g_cmd.m_pBuf[3]));
      g_pwm2 = *((uint16_t*)(&g_cmd.m_pBuf[5]));
      break;
    default:
      break;
  }
}

void setup()
{
  g_pwm1 = 1500;
  g_pwm2 = 1500;
  g_servo1.attach(5);
  g_servo2.attach(6);
  g_servo1.writeMicroseconds(g_pwm1);
  g_servo2.writeMicroseconds(g_pwm2);

  Serial.begin(115200);

  // configure the CS pin
  //Ethernet.init(10);  // Most Arduino shields

  while (1)
  {
    Ethernet.begin(mac, g_myIP);

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
  g_nMsg = 1;

}

void loop()
{
  g_servo1.writeMicroseconds(g_pwm1);
  g_servo2.writeMicroseconds(g_pwm2);

  g_UDP.parsePacket();
  decodeCMD();

  g_UDP.beginPacket(g_UDP.remoteIP(), g_UDP.remotePort());
  g_UDP.write(ARDU_CMD_BEGIN);            //start mark
  g_UDP.write((uint8_t)ARDU_CMD_STATUS);  //cmd
  g_UDP.write(4);                         //payload len
  g_UDP.write((uint8_t)(g_pwm1 & 0xFF));
  g_UDP.write((uint8_t)((g_pwm1 >> 8) & 0xFF));
  g_UDP.write((uint8_t)(g_pwm2 & 0xFF));
  g_UDP.write((uint8_t)((g_pwm2 >> 8) & 0xFF));
  g_UDP.endPacket();
}