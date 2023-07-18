#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>
/*
   This sample sketch demonstrates the normal use of a TinyGPSPlus (TinyGPSPlus) object.
   It requires the use of SoftwareSerial, and assumes that you have a
   4800-baud serial GPS device hooked up on pins 4(rx) and 3(tx).
*/
static const int RXPin = 4, TXPin = 3;
static const uint32_t GPSBaud = 4800;

// The TinyGPSPlus object
TinyGPSPlus gps;

// The serial connection to the GPS device
SoftwareSerial ss(RXPin, TXPin);

#define _DEBUG

uint16_t swap_uint16(uint16_t val)
{
  return (val << 8) | (val >> 8);
}

uint64_t swap_uint64(uint64_t val)
{
  val = ((val << 8) & 0xFF00FF00FF00FF00ULL) | ((val >> 8) & 0x00FF00FF00FF00FFULL);
  val = ((val << 16) & 0xFFFF0000FFFF0000ULL) | ((val >> 16) & 0x0000FFFF0000FFFFULL);
  return (val << 32) | (val >> 32);
}

#define XB_DELIM 0x7E
#define XB_N_FRAME 128
#define XB_N_PAYLOAD 49

struct XBframe_transitRequest
{
  // buffer
  uint8_t m_pF[XB_N_FRAME];
  uint16_t m_nF;

  // header
  uint16_t m_length;

  // API ID
  uint8_t m_fType = 0x10;
  uint8_t m_fID = 1;
  uint64_t m_destAddr = 0x000000000000FFFF; // Broadcast address
  uint16_t m_destAddr16 = 0xFFFE;
  uint8_t m_brRadius = 0;
  uint8_t m_options = 0;
  uint8_t m_pD[XB_N_PAYLOAD];
  uint8_t m_nD;

  void encode(uint8_t *pD, int nD)
  {
    // copy data
    memcpy(m_pD, pD, nD);
    m_nD = nD;
    m_length = 14 + m_nD;
    m_nF = m_length + 4;

    // Header
    m_pF[0] = XB_DELIM;
    m_pF[1] = m_length >> 8;
    m_pF[2] = m_length & 0xFF;

    // Transimit request
    m_pF[3] = m_fType;
    m_pF[4] = m_fID;
    *((uint64_t *)(&m_pF[5])) = swap_uint64(m_destAddr);
    *((uint16_t *)(&m_pF[13])) = swap_uint16(m_destAddr16);
    m_pF[15] = m_brRadius;
    m_pF[16] = m_options;
    memcpy(&m_pF[17], m_pD, m_nD);

    // Checksum
    uint32_t s = 0;
    int nS = m_length + 3;
    for (int i = 3; i < nS; i++)
    {
      s += m_pF[i];
    }
    s &= 0xFF;
    s = 0xFF - s;
    m_pF[17 + m_nD] = s;
  }
};

void setup()
{
  Serial.begin(115200); // debug
  ss.begin(GPSBaud);    // GPS
  Serial1.begin(57600); // XBee

#ifdef _DEBUG
  Serial.println(F("DeviceExample.ino"));
  Serial.println(F("A simple demonstration of TinyGPSPlus with an attached GPS module"));
  Serial.print(F("Testing TinyGPSPlus library v. "));
  Serial.println(TinyGPSPlus::libraryVersion());
  Serial.println(F("by Mikal Hart"));
  Serial.println();
#endif
}

int tLastBroadcast = 0;
static const int tInterval = 1000;
uint32_t beaconID = 100;
uint8_t msgType = 200;
XBframe_transitRequest xbTR;
#define MSG_N 21

void loop()
{
  // This sketch displays information every time a new sentence is correctly encoded.
  while (ss.available() > 0)
  {
    if (gps.encode(ss.read()))
      displayInfo();
  }

  int tNow = millis();
  if (tNow - tLastBroadcast > tInterval)
  {
    if (gps.location.isValid())
    {
      uint64_t lat = gps.location.lat() * 10e7;
      uint64_t lng = gps.location.lng() * 10e7;

      // msgType, srcIDx4, latx8, lngx8,
      uint8_t pB[MSG_N];
      pB[0] = msgType;
      *((uint32_t *)&pB[1]) = beaconID;
      *((uint64_t *)&pB[5]) = lat;
      *((uint64_t *)&pB[13]) = lat;

      xbTR.encode(pB, MSG_N);
      Serial1.write(pB, MSG_N);

      tLastBroadcast = tNow;
    }
  }

  if (tNow > 5000 && gps.charsProcessed() < 10)
  {
    Serial.println(F("No GPS detected: check wiring."));
    while (true)
      ;
  }
}

void displayInfo()
{
  Serial.print(F("Location: "));
  if (gps.location.isValid())
  {
    Serial.print(gps.location.lat(), 6);
    Serial.print(F(","));
    Serial.print(gps.location.lng(), 6);
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.print(F("  Date/Time: "));
  if (gps.date.isValid())
  {
    Serial.print(gps.date.month());
    Serial.print(F("/"));
    Serial.print(gps.date.day());
    Serial.print(F("/"));
    Serial.print(gps.date.year());
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.print(F(" "));
  if (gps.time.isValid())
  {
    if (gps.time.hour() < 10)
      Serial.print(F("0"));
    Serial.print(gps.time.hour());
    Serial.print(F(":"));
    if (gps.time.minute() < 10)
      Serial.print(F("0"));
    Serial.print(gps.time.minute());
    Serial.print(F(":"));
    if (gps.time.second() < 10)
      Serial.print(F("0"));
    Serial.print(gps.time.second());
    Serial.print(F("."));
    if (gps.time.centisecond() < 10)
      Serial.print(F("0"));
    Serial.print(gps.time.centisecond());
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.println();
}
