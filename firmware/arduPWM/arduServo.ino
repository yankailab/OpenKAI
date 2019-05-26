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

void command(void)
{
  switch (m_cmd.m_pBuf[1])
  {
  case ARDU_CMD_PWM:
  	uint16_t pwm;
    pwm = *((uint16_t*)(&m_cmd.m_pBuf[3]));
    g_servo1.writeMicroseconds(pwm);
    pwm = *((uint16_t*)(&m_cmd.m_pBuf[5]));
    g_servo2.writeMicroseconds(pwm);    
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
        if(iMsg >= 20)return;
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
  g_servo1.writeMicroseconds(1500);
  g_servo2.writeMicroseconds(1500);

  Serial.begin(115200);
}

void loop()
{
  receive();
}
