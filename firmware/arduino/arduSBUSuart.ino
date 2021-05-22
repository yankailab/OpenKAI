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
  uint8_t m_prevB;
  bool m_bLostFrame;
  bool m_bFailSafe;
  bool m_bCh17;
  bool m_bCh18;

  uint16_t m_pC[16];

  void init(void)
  {
    m_iB = 0;
    m_bLostFrame = false;
    m_bFailSafe = false;
    m_bCh17 = false;
    m_bCh18 = false;

    m_prevB = FOOTER_;
  }

  bool input(uint8_t c)
  {
    if (m_iB == 0)
    {
      if ((c == HEADER_))// && ((m_prevB == FOOTER_) || ((m_prevB & 0x0F) == FOOTER2_)))
      {
        m_pB[m_iB++] = c;
        m_prevB = c;
      }

      return false;
    }

    if (m_iB < LEN_)
    {
      m_pB[m_iB++] = c;
      m_prevB = c;
    }

    if (m_iB < LEN_)
      return false;

    m_iB = 0;
    return true;
    
    if ((m_pB[LEN_ - 1] == FOOTER_) || ((m_pB[LEN_ - 1] & 0x0F) == FOOTER2_))
    {
      return true;
    }

    return false;
  }

  void decode(void)
  {
    for(int i=0; i<16; i++)
    {
      m_pC[i] = static_cast<uint16_t>(m_pB[i*2+1] << 8 | m_pB[i*2+2]);
    }
    
//    m_bLostFrame = m_pB[23] & LOST_FRAME_;
//    m_bLostFrame = m_pB[23] & FAILSAFE_;
  }

  void encode(void)
  {
    for(int i=0; i<16; i++)
    {
      m_pB[i*2 + 1] = static_cast<uint8_t>(m_pC[i] >> 8);
      m_pB[i*2 + 2] = static_cast<uint8_t>(m_pC[i] & 0x00FF);      
    }

    m_pB[34] = checksum(&m_pB[1], 33);
  }

uint8_t checksum(uint8_t *buf, uint8_t len)
{
    uint8_t checksum = 0;

    for (int i = 0; i < len; ++i)
        checksum ^= buf[i];

    return checksum;
}

};

SBUS_FRAME g_sf;
int g_iL;

void setup()
{
  g_sf.init();
  g_iL = 0;
  Serial1.begin(115200);

#ifdef DEBUG_
  Serial.begin(115200);
#endif
}

void loop()
{
  if(Serial1.available() > 0)
  {
    uint8_t c = Serial1.read();

    if (g_sf.input(c))
    {
      g_sf.decode();
      g_sf.init();

      g_sf.m_pC[2] = 1700;
      g_sf.encode();
      Serial1.write(g_sf.m_pB, LEN_);  

#ifdef DEBUG_
      Serial.print("C: ");
      for (int i = 0; i < 16; i++)
      {
        Serial.print(g_sf.m_pC[i], DEC);
        Serial.print(", ");
      }
      Serial.print("bFS=");
      Serial.print(g_sf.m_bFailSafe, BIN);
      Serial.print(", bLS=");
      Serial.print(g_sf.m_bLostFrame, BIN);

      Serial.println();
#endif
    }
  }
}
