#include <Arduino.h>
#include <Servo.h>

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

struct ARDUINO_CMD
{
  uint8_t m_cmd;
  uint8_t m_nPayload;
  uint16_t m_iByte;
  uint8_t m_pBuf[ARDU_CMD_N_BUF];
};
ARDUINO_CMD m_cmd;

Servo g_servo1;
Servo g_servo2;
uint16_t g_pwm1;
uint16_t g_pwm2;

void command(void)
{
  switch (m_cmd.m_pBuf[1])
  {
  case ARDU_CMD_PWM:
    g_pwm1 = *((uint16_t*)(&m_cmd.m_pBuf[3]));
    g_pwm2 = *((uint16_t*)(&m_cmd.m_pBuf[5]));
    break;
  default:
    break;
  }
}

void receive(void)
{
  byte  inByte;
  int iMsg = 0;

  while (Serial.available() > 0)
  {
    inByte = Serial.read();
s
    if (m_cmd.m_cmd != 0)
    {
      m_cmd.m_pBuf[m_cmd.m_iByte] = inByte;
      m_cmd.m_iByte++;

      if (m_cmd.m_iByte == 3) //Payload length
      {
        m_cmd.m_nPayload = inByte;
      }
      else if (m_cmd.m_iByte == m_cmd.m_nPayload + ARDU_CMD_N_HEADER)
      {
        //decode the command
        command();
        m_cmd.m_cmd = 0;

        iMsg++;
        if(iMsg >= 1)return;
      }
    }
    else if (inByte == ARDU_CMD_BEGIN)
    {
      m_cmd.m_cmd = inByte;
      m_cmd.m_pBuf[0] = inByte;
      m_cmd.m_iByte = 1;
      m_cmd.m_nPayload = 0;
    }
  }
}

void setup()
{
  g_servo1.attach(5);
  g_servo2.attach(6);

  g_pwm1 = 1500;
  g_pwm2 = 1500;

  Serial.begin(115200);
}

void loop()
{
    g_servo1.writeMicroseconds(g_pwm1);
    g_servo2.writeMicroseconds(g_pwm2);    

  receive();

  Serial.write(ARDU_CMD_BEGIN);            //start mark
  Serial.write((uint8_t)ARDU_CMD_STATUS);  //cmd
  Serial.write(4);                         //payload len
  Serial.write((uint8_t)(g_pwm1 & 0xFF));
  Serial.write((uint8_t)((g_pwm1 >> 8) & 0xFF));
  Serial.write((uint8_t)(g_pwm2 & 0xFF));
  Serial.write((uint8_t)((g_pwm2 >> 8) & 0xFF));

}
