#include <Arduino.h>
#include <Servo.h>

#define SERIAL_DEBUG

//0 ARDU_CMD_BEGIN
//1 COMMAND
//2 PAYLOAD LENGTH
//3 Payload...

//Protocol
#define ARDU_CMD_N_BUF 256
#define ARDU_CMD_BEGIN 0xFE
#define ARDU_CMD_N_HEADER 3
#define ARDU_CMD_PWM 0
#define ARDU_CMD_HB 1

//Pinout
#define PWM_OUT_A 5
#define PWM_OUT_B 6
#define PWM_IN_L 8
#define PWM_IN_R 9
#define PWM_IN_M 10

//Constants
#define PWM_TOUT 100000

struct ARDUINO_CMD
{
  uint8_t m_cmd;
  uint8_t m_nPayload;
  uint16_t m_iB;
  uint8_t m_pB[ARDU_CMD_N_BUF];
};
ARDUINO_CMD m_cmd;

Servo g_servo1;
Servo g_servo2;
uint16_t g_pwmOutA;
uint16_t g_pwmOutB;
uint16_t g_pwmInL;
uint16_t g_pwmInR;
uint16_t g_pwmInM;

void command(void)
{
#include <Arduino.h>
#include <Servo.h>

#define SERIAL_DEBUG

//0 ARDU_CMD_BEGIN
//1 COMMAND
//2 PAYLOAD LENGTH
//3 Payload...

//Protocol
#define ARDU_CMD_N_BUF 256
#define ARDU_CMD_BEGIN 0xFE
#define ARDU_CMD_N_HEADER 3
#define ARDU_CMD_PWM 0
#define ARDU_CMD_HB 1

//Pinout
#define PWM_OUT_A 5
#define PWM_OUT_B 6
#define PWM_IN_L 8
#define PWM_IN_R 9
#define PWM_IN_M 10

//Constants
#define PWM_TOUT 100000

struct ARDUINO_CMD
{
  uint8_t m_cmd;
  uint8_t m_nPayload;
  uint16_t m_iB;
  uint8_t m_pB[ARDU_CMD_N_BUF];
};
ARDUINO_CMD m_cmd;

Servo g_servo1;
Servo g_servo2;
uint16_t g_pwmOutA;
uint16_t g_pwmOutB;
uint16_t g_pwmInL;
uint16_t g_pwmInR;
uint16_t g_pwmInM;

void command(void)
{
  switch (m_cmd.m_pB[1])
  {
  case ARDU_CMD_PWM:
    g_pwmOutA = *((uint16_t*)(&m_cmd.m_pB[3]));
    g_pwmOutB = *((uint16_t*)(&m_cmd.m_pB[5]));
    break;
  default:
    break;
  }
}

void receive(void)
{
  byte  inByte;
  int iMsg = 0;

  while (Serial1.available() > 0)
  {
    inByte = Serial1.read();

    if (m_cmd.m_cmd != 0)
    {
      m_cmd.m_pB[m_cmd.m_iB] = inByte;
      m_cmd.m_iB++;

      if (m_cmd.m_iB == 3) //Payload length
      {
        m_cmd.m_nPayload = inByte;
      }
      else if (m_cmd.m_iB == m_cmd.m_nPayload + ARDU_CMD_N_HEADER)
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
      m_cmd.m_pB[0] = inByte;
      m_cmd.m_iB = 1;
      m_cmd.m_nPayload = 0;
    }
  }
}

void send()
{
  Serial1.write(ARDU_CMD_BEGIN);            //start mark
  Serial1.write((uint8_t)ARDU_CMD_HB);      //cmd
  Serial1.write(6);                         //payload len
  Serial1.write((uint8_t)(g_pwmInL & 0xFF));
  Serial1.write((uint8_t)((g_pwmInL >> 8) & 0xFF));
  Serial1.write((uint8_t)(g_pwmInR & 0xFF));
  Serial1.write((uint8_t)((g_pwmInR >> 8) & 0xFF));
  Serial1.write((uint8_t)(g_pwmInM & 0xFF));
  Serial1.write((uint8_t)((g_pwmInM >> 8) & 0xFF));

#ifdef SERIAL_DEBUG
  Serial.print("pwmIn: ");
  Serial.print(g_pwmInL);
  Serial.print(", ");
  Serial.print(g_pwmInR);
  Serial.print(", ");
  Serial.print(g_pwmInM);
  Serial.print("; pwmOut: ");
  Serial.print(g_pwmOutA);
  Serial.print(", ");
  Serial.print(g_pwmOutB);
  Serial.println();

//  Serial.write((uint8_t)(g_pwmOutA & 0xFF));
//  Serial.write((uint8_t)((g_pwmOutA >> 8) & 0xFF));
//  Serial.write((uint8_t)(g_pwmOutB & 0xFF));
//  Serial.write((uint8_t)((g_pwmOutB >> 8) & 0xFF));
//  Serial.write((uint8_t)(g_pwmOutB & 0xFF));
//  Serial.write((uint8_t)((g_pwmOutB >> 8) & 0xFF));
#endif

}

void setup()
{
  g_servo1.attach(PWM_OUT_A);
  g_servo2.attach(PWM_OUT_B);
  pinMode(PWM_IN_L, INPUT);
  pinMode(PWM_IN_R, INPUT);
  pinMode(PWM_IN_M, INPUT);

  g_pwmInL = 0;
  g_pwmInR = 0;
  g_pwmInM = 0;
  g_pwmOutA = 1500;
  g_pwmOutB = 1500;

  g_servo1.writeMicroseconds(g_pwmOutA);
  g_servo2.writeMicroseconds(g_pwmOutB);

  Serial1.begin(115200);

#ifdef SERIAL_DEBUG
  Serial.begin(115200);
#endif
}

void loop()
{
  g_pwmInL = pulseIn(PWM_IN_L, HIGH, PWM_TOUT);
  g_pwmInR = pulseIn(PWM_IN_R, HIGH, PWM_TOUT);
  g_pwmInM = pulseIn(PWM_IN_M, HIGH, PWM_TOUT);
  send();

  receive();

  if(g_pwmInM < 1500)
  {
    g_servo1.writeMicroseconds(g_pwmInL);
    g_servo2.writeMicroseconds(g_pwmInR);    
  }
  else
  {
    g_servo1.writeMicroseconds(g_pwmOutA);
    g_servo2.writeMicroseconds(g_pwmOutB);
  }
}
}

void receive(void)
{
  byte  inByte;
  int iMsg = 0;

  while (Serial1.available() > 0)
  {
    inByte = Serial1.read();

    if (m_cmd.m_cmd != 0)
    {
      m_cmd.m_pB[m_cmd.m_iB] = inByte;
      m_cmd.m_iB++;

      if (m_cmd.m_iB == 3) //Payload length
      {
        m_cmd.m_nPayload = inByte;
      }
      else if (m_cmd.m_iB == m_cmd.m_nPayload + ARDU_CMD_N_HEADER)
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
      m_cmd.m_pB[0] = inByte;
      m_cmd.m_iB = 1;
      m_cmd.m_nPayload = 0;
    }
  }
}

void send()
{
  Serial1.write(ARDU_CMD_BEGIN);            //start mark
  Serial1.write((uint8_t)ARDU_CMD_HB);      //cmd
  Serial1.write(6);                         //payload len
  Serial1.write((uint8_t)(g_pwmInL & 0xFF));
  Serial1.write((uint8_t)((g_pwmInL >> 8) & 0xFF));
  Serial1.write((uint8_t)(g_pwmInR & 0xFF));
  Serial1.write((uint8_t)((g_pwmInR >> 8) & 0xFF));
  Serial1.write((uint8_t)(g_pwmInM & 0xFF));
  Serial1.write((uint8_t)((g_pwmInM >> 8) & 0xFF));

#ifdef SERIAL_DEBUG
  Serial.print("pwmIn: ");
  Serial.print(g_pwmInL);
  Serial.print(", ");
  Serial.print(g_pwmInR);
  Serial.print(", ");
  Serial.print(g_pwmInM);
  Serial.print("; pwmOut: ");
  Serial.print(g_pwmOutA);
  Serial.print(", ");
  Serial.print(g_pwmOutB);
  Serial.println();

//  Serial.write((uint8_t)(g_pwmOutA & 0xFF));
//  Serial.write((uint8_t)((g_pwmOutA >> 8) & 0xFF));
//  Serial.write((uint8_t)(g_pwmOutB & 0xFF));
//  Serial.write((uint8_t)((g_pwmOutB >> 8) & 0xFF));
//  Serial.write((uint8_t)(g_pwmOutB & 0xFF));
//  Serial.write((uint8_t)((g_pwmOutB >> 8) & 0xFF));
#endif

}

void setup()
{
  g_servo1.attach(PWM_OUT_A);
  g_servo2.attach(PWM_OUT_B);
  pinMode(PWM_IN_L, INPUT);
  pinMode(PWM_IN_R, INPUT);
  pinMode(PWM_IN_M, INPUT);

  g_pwmInL = 0;
  g_pwmInR = 0;
  g_pwmInM = 0;
  g_pwmOutA = 1500;
  g_pwmOutB = 1500;

  g_servo1.writeMicroseconds(g_pwmOutA);
  g_servo2.writeMicroseconds(g_pwmOutB);

  Serial1.begin(115200);

#ifdef SERIAL_DEBUG
  Serial.begin(115200);
#endif
}

void loop()
{
  g_pwmInL = pulseIn(PWM_IN_L, HIGH, PWM_TOUT);
  g_pwmInR = pulseIn(PWM_IN_R, HIGH, PWM_TOUT);
  g_pwmInM = pulseIn(PWM_IN_M, HIGH, PWM_TOUT);
  send();

  receive();

  if(g_pwmInM < 1500)
  {
    g_servo1.writeMicroseconds(g_pwmInL);
    g_servo2.writeMicroseconds(g_pwmInR);    
  }
  else
  {
    g_servo1.writeMicroseconds(g_pwmOutA);
    g_servo2.writeMicroseconds(g_pwmOutB);
  }
}