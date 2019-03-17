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

union int16Bytes
{
    byte m_pByte[2];
    uint16_t m_int;
};

//Pin assign
#define PIN_MOT_L 5
#define PIN_MOT_R 6
#define PIN_PWM_L 7
#define PIN_PWM_R 8
#define PIN_PWM_MODE 9
#define PIN_LED_IDLE A0
#define PIN_LED_MANUAL A1
#define PIN_LED_AUTO A2
#define PIN_LED_INDICATOR A3
#define PWM_MID 1500
#define PWM_MID_DZ 100
#define PWM_LOW 500
#define PWM_LIM 300

#define AP_TIMEOUT 1000
#define PWM_TIMEOUT 1000000

#define MODE_IDLE 0
#define MODE_MANUAL 1
#define MODE_AUTO 2
#define MODE_FORWARD 3
#define MODE_BACKWARD 4

uint8_t g_Mode;

int g_pwmLin;
int g_pwmRin;
int g_pwmModeIn;

uint16_t g_pwmLap;
uint16_t g_pwmRap;
long g_tAP;
long g_tNow;

Servo g_motorL;
Servo g_motorR;


void command(void)
{
  switch (m_cmd.m_pBuf[1])
  {
  case ARDU_CMD_PWM:
    g_pwmLap = *((uint16_t*)(&m_cmd.m_pBuf[3]));
    g_pwmRap = *((uint16_t*)(&m_cmd.m_pBuf[5]));
    g_tAP = millis();
    break;

  case ARDU_CMD_PIN_OUTPUT:
    digitalWrite(m_cmd.m_pBuf[3], m_cmd.m_pBuf[4]);
    g_tAP = millis();
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

      if (m_cmd.m_iByte == 3) //Payload length
      {
        m_cmd.m_nPayload = inByte;
      }
      else if (m_cmd.m_iByte == m_cmd.m_nPayload + ARDU_CMD_N_HEADER)
      {
        //decode the command
        command();
        m_cmd.m_cmd = 0;

        //Execute one command at a time
        return true;
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

  return false;
}

void mode()
{
  if(g_pwmModeIn <= PWM_LOW || g_pwmLin <= PWM_LOW || g_pwmRin <= PWM_LOW)
  {
    g_Mode = MODE_IDLE;
    return;
  }
  
  if(abs(g_pwmModeIn-PWM_MID) < PWM_MID_DZ)
  {
    g_Mode = MODE_MANUAL;
    return;
  }
  
  if(g_tNow - g_tAP > AP_TIMEOUT)
  {
    g_Mode = MODE_IDLE;
    return;
  }

  if(g_pwmModeIn > PWM_MID)
      g_Mode = MODE_FORWARD;
  else
      g_Mode = MODE_BACKWARD;

}

void updateLED(void)
{
  digitalWrite(PIN_LED_IDLE, LOW);
  digitalWrite(PIN_LED_MANUAL, LOW);
  digitalWrite(PIN_LED_AUTO, LOW);
  digitalWrite(PIN_LED_INDICATOR, LOW);

  if(g_Mode == MODE_IDLE)
  {
    digitalWrite(PIN_LED_IDLE, HIGH);
  }
  else if(g_Mode == MODE_MANUAL)
  {
    digitalWrite(PIN_LED_MANUAL, HIGH);
  }
  else
  {
    digitalWrite(PIN_LED_AUTO, HIGH);
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

  pinMode(PIN_LED_IDLE, OUTPUT);
  pinMode(PIN_LED_MANUAL, OUTPUT);
  pinMode(PIN_LED_AUTO, OUTPUT);
  pinMode(PIN_LED_INDICATOR, OUTPUT);

  pinMode(PIN_PWM_L, INPUT);
  pinMode(PIN_PWM_R, INPUT);
  pinMode(PIN_PWM_MODE, INPUT);

  Serial.begin(115200);
}

void loop()
{
  g_pwmLin = pulseIn(PIN_PWM_L, HIGH,PWM_TIMEOUT);
  g_pwmRin = pulseIn(PIN_PWM_R, HIGH, PWM_TIMEOUT);
  g_pwmModeIn = pulseIn(PIN_PWM_MODE, HIGH, PWM_TIMEOUT);
  
  receive();
  g_tNow = millis();
  mode();
  updateLED();

  uint16_t pwmL;
  uint16_t pwmR;

  if(g_Mode == MODE_IDLE)
  {
    pwmL = PWM_MID;
    pwmR = PWM_MID;
  }
  else if(g_Mode == MODE_MANUAL)
  {
    pwmL = g_pwmLin;
    pwmR = g_pwmRin;
  }
  else
  {
    pwmL = g_pwmLap;
    pwmR = g_pwmRap;
    pwmL = constrain(pwmL, PWM_MID-PWM_LIM, PWM_MID+PWM_LIM);
    pwmR = constrain(pwmR, PWM_MID-PWM_LIM, PWM_MID+PWM_LIM);
  }

  g_motorL.writeMicroseconds(pwmL);
  g_motorR.writeMicroseconds(pwmR);


  Serial.write(ARDU_CMD_BEGIN);            //start mark
  Serial.write((uint8_t)ARDU_CMD_STATUS);  //cmd
  Serial.write(11);                         //payload len
  Serial.write((uint8_t)g_Mode);
  Serial.write((uint8_t)(g_pwmModeIn & 0xFF));
  Serial.write((uint8_t)((g_pwmModeIn >> 8) & 0xFF));
  Serial.write((uint8_t)(g_pwmLin & 0xFF));
  Serial.write((uint8_t)((g_pwmLin >> 8) & 0xFF));
  Serial.write((uint8_t)(g_pwmRin & 0xFF));
  Serial.write((uint8_t)((g_pwmRin >> 8) & 0xFF));
  Serial.write((uint8_t)(pwmL & 0xFF));
  Serial.write((uint8_t)((pwmL >> 8) & 0xFF));
  Serial.write((uint8_t)(pwmR & 0xFF));
  Serial.write((uint8_t)((pwmR >> 8) & 0xFF));

}
