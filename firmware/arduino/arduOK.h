#include <Arduino.h>

//0 ARDU_CMD_BEGIN
//1 COMMAND
//2 PAYLOAD LENGTH
//3 Payload...

//Protocol
#define ARDU_CMD_N_BUF 256
#define ARDU_CMD_BEGIN 0xFE
#define ARDU_CMD_N_HEADER 3
#define ARDU_CMD_PWM 0
#define ARDU_CMD_PIN_OUTPUT 1
#define ARDU_CMD_STATUS 2

void runCMD(void);
bool ioAvailable(void);
byte ioRead(void);
int g_nMsg = 1;

struct ARDUINO_CMD
{
  uint8_t m_cmd;
  uint8_t m_nPayload;
  uint16_t m_iByte;
  uint8_t m_pBuf[ARDU_CMD_N_BUF];
};
ARDUINO_CMD g_cmd;

void decodeCMD(void)
{
  byte inByte;
  int iMsg = 0;

  while (ioAvailable() > 0)
  {
    inByte = ioRead();

    if (g_cmd.m_cmd != 0)
    {
      g_cmd.m_pBuf[g_cmd.m_iByte] = inByte;
      g_cmd.m_iByte++;

      if (g_cmd.m_iByte == 3) //Payload length
      {
        g_cmd.m_nPayload = inByte;
      }
      else if (g_cmd.m_iByte == g_cmd.m_nPayload + ARDU_CMD_N_HEADER)
      {
        runCMD();
        g_cmd.m_cmd = 0;

        if (++iMsg > g_nMsg)return;
      }
    }
    else if (inByte == ARDU_CMD_BEGIN)
    {
      g_cmd.m_cmd = inByte;
      g_cmd.m_pBuf[0] = inByte;
      g_cmd.m_iByte = 1;
      g_cmd.m_nPayload = 0;
    }
  }
}
