#include <Arduino.h>

#define DEBUG_

//SBus definition
#define HEADER_ 0x0F
#define FOOTER_ 0x00
#define FOOTER2_ 0x04
#define LEN_ 25
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
    m_prevB = FOOTER_;
    m_bLostFrame = false;
    m_bFailSafe = false;
    m_bCh17 = false;
    m_bCh18 = false;
  }

  bool input(uint8_t c)
  {
    if (m_iB == 0)
    {
      if ((c == HEADER_) && ((m_prevB == FOOTER_) || ((m_prevB & 0x0F) == FOOTER2_)))
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
    if ((m_pB[LEN_ - 1] == FOOTER_) || ((m_pB[LEN_ - 1] & 0x0F) == FOOTER2_))
      return true;

    return false;
  }

  void decode(void)
  {
    m_pC[0] = static_cast<uint16_t>(m_pB[1] | m_pB[2] << 8 & 0x07FF);
    m_pC[1] = static_cast<uint16_t>(m_pB[2] >> 3 | m_pB[3] << 5 & 0x07FF);
    m_pC[2] = static_cast<uint16_t>(m_pB[3] >> 6 | m_pB[4] << 2 |
                                    m_pB[5] << 10 & 0x07FF);
    m_pC[3] = static_cast<uint16_t>(m_pB[5] >> 1 | m_pB[6] << 7 & 0x07FF);
    m_pC[4] = static_cast<uint16_t>(m_pB[6] >> 4 | m_pB[7] << 4 & 0x07FF);
    m_pC[5] = static_cast<uint16_t>(m_pB[7] >> 7 | m_pB[8] << 1 |
                                    m_pB[9] << 9 & 0x07FF);
    m_pC[6] = static_cast<uint16_t>(m_pB[9] >> 2 | m_pB[10] << 6 & 0x07FF);
    m_pC[7] = static_cast<uint16_t>(m_pB[10] >> 5 | m_pB[11] << 3 & 0x07FF);
    m_pC[8] = static_cast<uint16_t>(m_pB[12] | m_pB[13] << 8 & 0x07FF);
    m_pC[9] = static_cast<uint16_t>(m_pB[13] >> 3 | m_pB[14] << 5 & 0x07FF);
    m_pC[10] = static_cast<uint16_t>(m_pB[14] >> 6 | m_pB[15] << 2 |
                                     m_pB[16] << 10 & 0x07FF);
    m_pC[11] = static_cast<uint16_t>(m_pB[16] >> 1 | m_pB[17] << 7 & 0x07FF);
    m_pC[12] = static_cast<uint16_t>(m_pB[17] >> 4 | m_pB[18] << 4 & 0x07FF);
    m_pC[13] = static_cast<uint16_t>(m_pB[18] >> 7 | m_pB[19] << 1 |
                                     m_pB[20] << 9 & 0x07FF);
    m_pC[14] = static_cast<uint16_t>(m_pB[20] >> 2 | m_pB[21] << 6 & 0x07FF);
    m_pC[15] = static_cast<uint16_t>(m_pB[21] >> 5 | m_pB[22] << 3 & 0x07FF);

    m_bLostFrame = m_pB[23] & LOST_FRAME_;
    m_bLostFrame = m_pB[23] & FAILSAFE_;
  }

  void encode(void)
  {
    m_pB[0] = HEADER_;
    m_pB[1] = static_cast<uint8_t>((m_pC[0] & 0x07FF));
    m_pB[2] = static_cast<uint8_t>((m_pC[0] & 0x07FF) >> 8 |
                                   (m_pC[1] & 0x07FF) << 3);
    m_pB[3] = static_cast<uint8_t>((m_pC[1] & 0x07FF) >> 5 |
                                   (m_pC[2] & 0x07FF) << 6);
    m_pB[4] = static_cast<uint8_t>((m_pC[2] & 0x07FF) >> 2);
    m_pB[5] = static_cast<uint8_t>((m_pC[2] & 0x07FF) >> 10 |
                                   (m_pC[3] & 0x07FF) << 1);
    m_pB[6] = static_cast<uint8_t>((m_pC[3] & 0x07FF) >> 7 |
                                   (m_pC[4] & 0x07FF) << 4);
    m_pB[7] = static_cast<uint8_t>((m_pC[4] & 0x07FF) >> 4 |
                                   (m_pC[5] & 0x07FF) << 7);
    m_pB[8] = static_cast<uint8_t>((m_pC[5] & 0x07FF) >> 1);
    m_pB[9] = static_cast<uint8_t>((m_pC[5] & 0x07FF) >> 9 |
                                   (m_pC[6] & 0x07FF) << 2);
    m_pB[10] = static_cast<uint8_t>((m_pC[6] & 0x07FF) >> 6 |
                                    (m_pC[7] & 0x07FF) << 5);
    m_pB[11] = static_cast<uint8_t>((m_pC[7] & 0x07FF) >> 3);
    m_pB[12] = static_cast<uint8_t>((m_pC[8] & 0x07FF));
    m_pB[13] = static_cast<uint8_t>((m_pC[8] & 0x07FF) >> 8 |
                                    (m_pC[9] & 0x07FF) << 3);
    m_pB[14] = static_cast<uint8_t>((m_pC[9] & 0x07FF) >> 5 |
                                    (m_pC[10] & 0x07FF) << 6);
    m_pB[15] = static_cast<uint8_t>((m_pC[10] & 0x07FF) >> 2);
    m_pB[16] = static_cast<uint8_t>((m_pC[10] & 0x07FF) >> 10 |
                                    (m_pC[11] & 0x07FF) << 1);
    m_pB[17] = static_cast<uint8_t>((m_pC[11] & 0x07FF) >> 7 |
                                    (m_pC[12] & 0x07FF) << 4);
    m_pB[18] = static_cast<uint8_t>((m_pC[12] & 0x07FF) >> 4 |
                                    (m_pC[13] & 0x07FF) << 7);
    m_pB[19] = static_cast<uint8_t>((m_pC[13] & 0x07FF) >> 1);
    m_pB[20] = static_cast<uint8_t>((m_pC[13] & 0x07FF) >> 9 |
                                    (m_pC[14] & 0x07FF) << 2);
    m_pB[21] = static_cast<uint8_t>((m_pC[14] & 0x07FF) >> 6 |
                                    (m_pC[15] & 0x07FF) << 5);
    m_pB[22] = static_cast<uint8_t>((m_pC[15] & 0x07FF) >> 3);
    m_pB[23] = 0x00 | (m_bCh17 * CH17_) | (m_bCh18 * CH18_) |
               (m_bFailSafe * FAILSAFE_) | (m_bLostFrame * LOST_FRAME_);
    m_pB[24] = FOOTER_;
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
  while (Serial1.available() > 0)
  {
    if (g_sf.input(Serial1.read()))
    {
      g_sf.decode();
      g_sf.init();

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
