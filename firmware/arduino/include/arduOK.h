#include <Arduino.h>

//0 PROTOCOL_BEGIN
//1 COMMAND
//2 PAYLOAD LENGTH
//3 Payload...

//Protocol
#define PROTOCOL_BEGIN 0xFE
#define PROTOCOL_N_HEADER 3
#define PROTOCOL_N_BUF 256

//Functions to be implemented
void runCMD(void);
bool ioAvailable(void);
byte ioRead(void);

int g_nMsg = 1;

struct OKLINK_CMD
{
  uint8_t m_cmd;
  uint8_t m_nPayload;
  uint16_t m_iByte;
  uint8_t m_pBuf[PROTOCOL_N_BUF];
};
OKLINK_CMD g_cmd;

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
      else if (g_cmd.m_iByte == g_cmd.m_nPayload + PROTOCOL_N_HEADER)
      {
        runCMD();
        g_cmd.m_cmd = 0;

        if (++iMsg > g_nMsg)return;
      }
    }
    else if (inByte == PROTOCOL_BEGIN)
    {
      g_cmd.m_cmd = inByte;
      g_cmd.m_pBuf[0] = inByte;
      g_cmd.m_iByte = 1;
      g_cmd.m_nPayload = 0;
    }
  }
}
