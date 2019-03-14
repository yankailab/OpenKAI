#include <Arduino.h>
#include <Servo.h>

//Protocol
#define CMD_BUF_LEN 256
#define MAVLINK_BEGIN 0xFE
#define MAVLINK_HEADDER_LEN 3
//temporal
//0 START MARK
//1 PAYLOAD LENGTH
//2 COMMAND
//3 Payload...

//Commands
#define CMD_PWM 0
#define CMD_PIN_OUTPUT 1

struct CMD_STREAM
{
  uint8_t m_cmd;
  uint8_t m_payloadLen;
  uint16_t m_iByte;
  uint8_t m_pBuf[CMD_BUF_LEN];
};
CMD_STREAM m_cmd;

union longBytes
{
    byte m_pByte[4];
    long m_long;
};
longBytes lb;

int16_t g_pwmLin;
int16_t g_pwmRin;
int16_t g_pwmModeIn;

int16_t g_pwmLap;
int16_t g_pwmRap;

Servo g_motorL;
Servo g_motorR;

void command(void)
{
  switch (m_cmd.m_pBuf[2])
  {
  case CMD_PWM:
    g_pwmLap = *((int16_t*)(&m_cmd.m_pBuf[3]));
    g_pwmRap = *((int16_t*)(&m_cmd.m_pBuf[5]));
    break;

  case CMD_PIN_OUTPUT:
    digitalWrite(m_cmd.m_pBuf[3], m_cmd.m_pBuf[4]);
    break;

  default:
    break;
  }
}

bool receive(void)
{
  byte  inByte;

  while (Serial.available() > 0)
  {
    inByte = Serial.read();

    if (m_cmd.m_cmd != 0)
    {
      m_cmd.m_pBuf[m_cmd.m_iByte] = inByte;
      m_cmd.m_iByte++;

      if (m_cmd.m_iByte == 2) //Payload length
      {
        m_cmd.m_payloadLen = inByte;
      }
      else if (m_cmd.m_iByte == m_cmd.m_payloadLen + MAVLINK_HEADDER_LEN)
      {
        //decode the command
        command();
        m_cmd.m_cmd = 0;

        //Execute one command at a time
        return true;
      }
    }
    else if (inByte == MAVLINK_BEGIN)
    {
      m_cmd.m_cmd = inByte;
      m_cmd.m_pBuf[0] = inByte;
      m_cmd.m_iByte = 1;
      m_cmd.m_payloadLen = 0;
    }
  }

  return false;
}

void setup()
{
  pinMode(11,OUTPUT);
  pinMode(12,OUTPUT);
  pinMode(13,OUTPUT);

  g_motorL.attach(9);
  g_motorR.attach(10);

  Serial.begin(115200);

}

void loop()
{
  receive();

  int pwmL;
  int pwmR;
  
  g_motorL.writeMicroseconds(pwmL);
  g_motorR.writeMicroseconds(pwmR);
  

//        Serial.write(MAVLINK_BEGIN);      //start mark
//        Serial.write(13);           //payload len
//        Serial.write((uint8_t)CMD_CAN_SEND);  //cmd
//        Serial.write(lb.m_pByte[0]);
//        Serial.write(lb.m_pByte[1]);
//        Serial.write(lb.m_pByte[2]);
//        Serial.write(lb.m_pByte[3]);
//        Serial.write(len);            //len
//        Serial.write(buf, 8);         //data
}
