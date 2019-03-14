#include <Arduino.h>
#include <Servo.h>

//temporal
//0 START MARK
//1 PAYLOAD LENGTH
//2 COMMAND
//3 Payload...

//Protocol
#define CMD_BUF_LEN 256
#define CMD_BEGIN 0xFE
#define CMD_HEADDER_LEN 3

//Commands
#define CMD_PWM 0
#define CMD_PIN_OUTPUT 1
#define CMD_STATUS 2

struct CMD_STREAM
{
  uint8_t m_cmd;
  uint8_t m_payloadLen;
  uint16_t m_iByte;
  uint8_t m_pBuf[CMD_BUF_LEN];
};
CMD_STREAM m_cmd;

union int16Bytes
{
    byte m_pByte[2];
    uint16_t m_int;
};

//Pin assign
#define PIN_MOT_L 5
#define PIN_MOT_R 6
#define PIN_PWM_L A0
#define PIN_PWM_R A1
#define PIN_PWM_MODE A2
#define PIN_LED1 10
#define PIN_LED2 11
#define PIN_LED3 12
#define PWM_MID 1500
#define PWM_MID_DZ 100
#define PWM_LOW 600
#define AP_TIMEOUT 2000
#define PWM_TIMEOUT 3000

#define MODE_IDLE 0
#define MODE_MANUAL 1
#define MODE_FORWARD 2
#define MODE_BACKWARD 3

int8_t g_Mode;

int g_pwmLin;
int g_pwmRin;
int g_pwmModeIn;

int16_t g_pwmLap;
int16_t g_pwmRap;
long g_tAP;
long g_tNow;

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

  g_tAP = millis();
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
      else if (m_cmd.m_iByte == m_cmd.m_payloadLen + CMD_HEADDER_LEN)
      {
        //decode the command
        command();
        m_cmd.m_cmd = 0;

        //Execute one command at a time
        return true;
      }
    }
    else if (inByte == CMD_BEGIN)
    {
      m_cmd.m_cmd = inByte;
      m_cmd.m_pBuf[0] = inByte;
      m_cmd.m_iByte = 1;
      m_cmd.m_payloadLen = 0;
    }
  }

  return false;
}

void mode()
{
  if(g_pwmModeIn <= PWM_LOW)
  {
    g_Mode = MODE_IDLE;
    return;
  }
  
  if(abs(g_pwmModeIn-PWM_MID) < PWM_MID_DZ)
  {
    g_Mode = MODE_MANUAL;
  }
  else if(g_tNow - g_tAP < AP_TIMEOUT)
  {
    if(g_pwmModeIn > PWM_MID)
      g_Mode = MODE_FORWARD;
    else
      g_Mode = MODE_BACKWARD;
    
  }
  else
  {
    g_Mode = MODE_MANUAL;
  }
}

void setup()
{
  g_tAP = 0;
  g_Mode = MODE_IDLE;
  
  g_motorL.attach(PIN_MOT_L);
  g_motorR.attach(PIN_MOT_R);
  g_motorL.writeMicroseconds(PWM_MID);
  g_motorR.writeMicroseconds(PWM_MID);

  pinMode(PIN_LED1, OUTPUT);
  pinMode(PIN_LED2, OUTPUT);
  pinMode(PIN_LED3, OUTPUT);

  pinMode(PIN_PWM_L, INPUT);
  pinMode(PIN_PWM_R, INPUT);
  pinMode(PIN_PWM_MODE, INPUT);

  Serial.begin(115200);

  digitalWrite(PIN_LED1, HIGH);
}

void loop()
{  
  g_pwmLin = pulseIn(PIN_PWM_L, HIGH, PWM_TIMEOUT);
  g_pwmRin = pulseIn(PIN_PWM_R, HIGH, PWM_TIMEOUT);
  g_pwmModeIn = pulseIn(PIN_PWM_MODE, HIGH, PWM_TIMEOUT);
  
  receive();
  g_tNow = millis();
  mode();


  int pwmL;
  int pwmR;
  if(g_Mode == MODE_MANUAL)
  {
    pwmL = g_pwmLin;
    pwmR = g_pwmRin;
    digitalWrite(13, LOW);
  }
  else if(g_Mode == MODE_IDLE)
  {
    pwmL = PWM_MID;
    pwmR = PWM_MID;
    digitalWrite(13, HIGH);
  }
  else
  {
    pwmL = g_pwmLap;
    pwmR = g_pwmRap;
    digitalWrite(13, LOW);
  }
  
  g_motorL.writeMicroseconds(pwmL);
  g_motorR.writeMicroseconds(pwmR);

  Serial.write(CMD_BEGIN);            //start mark
  Serial.write(6);                    //payload len
  Serial.write((uint8_t)CMD_STATUS);  //cmd
  Serial.write((uint8_t)(g_pwmModeIn & 0xFF));
  Serial.write((uint8_t)((g_pwmModeIn >> 8) & 0xFF));
  Serial.write((uint8_t)(g_pwmLin & 0xFF));
  Serial.write((uint8_t)((g_pwmLin >> 8) & 0xFF));
  Serial.write((uint8_t)(g_pwmRin & 0xFF));
  Serial.write((uint8_t)((g_pwmRin >> 8) & 0xFF));
}
