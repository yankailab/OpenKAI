#include <Arduino.h>

#define DEBUG_

//SBus definition
#define HEADER_ 0x0F
#define FOOTER_ 0x00
#define FOOTER2_ 0x04
#define LEN_ 35
#define CH17_ 0x01
#define CH18_ 0x02
#define LOST_FRAME_ 0x04
#define FAILSAFE_ 0x08

struct SBUS_FRAME
{
  uint8_t m_pB[LEN_];
  uint8_t m_iB;
  uint8_t m_flag;
  bool m_bLostFrame;
  bool m_bFailSafe;
  bool m_bCh17;
  bool m_bCh18;
  uint16_t m_pC[16];

  void init(void)
  {
    m_iB = 0;
    m_flag = 0;
    m_bLostFrame = false;
    m_bFailSafe = false;
    m_bCh17 = false;
    m_bCh18 = false;
  }

  bool input(uint8_t c)
  {
    if (m_iB == 0)
    {
      if (c != HEADER_)
        return false;
    }

    m_pB[m_iB++] = c;

    if (m_iB < LEN_)
      return false;

    m_iB = 0;
    uint8_t cs = checksum(&m_pB[1], 33);
    if (m_pB[LEN_ - 1] != cs)
      return false;

    return true;
  }

  void decode(void)
  {
    for (int i = 0; i < 16; i++)
    {
      int j = i * 2;
      m_pC[i] = static_cast<uint16_t>(m_pB[j + 1] << 8 | m_pB[j + 2]);
    }

    m_flag = m_pB[LEN_ - 2];
  }

  void encode(void)
  {
    for (int i = 0; i < 16; i++)
    {
      int j = i * 2;
      m_pB[j + 1] = static_cast<uint8_t>(m_pC[i] >> 8);
      m_pB[j + 2] = static_cast<uint8_t>(m_pC[i] & 0x00FF);
    }

    m_pB[34] = checksum(&m_pB[1], 33);
  }

  uint8_t checksum(uint8_t *pB, uint8_t n)
  {
    uint8_t checksum = 0;

    for (int i = 0; i < n; ++i)
      checksum ^= pB[i];

    return checksum;
  }
};

SBUS_FRAME g_sf;

void setup()
{
  g_sf.init();
  Serial1.begin(115200);

#ifdef DEBUG_
  Serial.begin(115200);
#endif
}

void loop()
{
  if (Serial1.available() > 0)
  {
    uint8_t c = Serial1.read();
    if (g_sf.input(c))
    {
      g_sf.decode();
      g_sf.init();

      //      g_sf.m_pC[2] = 1700;
      g_sf.encode();
      Serial1.write(g_sf.m_pB, LEN_);

#ifdef DEBUG_
      Serial.print("C: ");
      for (int i = 0; i < 16; i++)
      {
        Serial.print(g_sf.m_pC[i], DEC);
        Serial.print(", ");
      }

      Serial.print("flag=");
      Serial.print(g_sf.m_flag, BIN);
      Serial.println();
#endif
    }
  }
}