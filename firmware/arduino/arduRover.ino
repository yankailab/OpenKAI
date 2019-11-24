#include <Arduino.h>
#include "include/arduOK.h"
#include "include/arduPPMreader.h"
#include "include/arduMotorPWM.h"
#include "include/arduUDP.h"

//#define SERIAL_DEBUG

//----------------------------------------------------------------------
// Pin assign
#define PIN_LED_STOP A2
#define PIN_LED_MANUAL A0
#define PIN_LED_AUTO A1
#define PIN_LED_INDICATOR A3
#define PIN_IN_PPM 3

#define CHAN_STEER 0
#define CHAN_SPEED 1
#define CHAN_MODE 2
#define CHAN_ACTION 3
PPMReader g_PPM;

//----------------------------------------------------------------------
// PWM input from transmitter
int16_t g_pwmSteer;
int16_t g_pwmSpeed;
int16_t g_pwmMode;
int16_t g_pwmAction;

float g_dirSpeed;
float g_dirSteer;
float g_nSpeed;
float g_dSpeed;

//----------------------------------------------------------------------
// Motor
#define N_MOT 4
MOTOR g_pMot[N_MOT];

//----------------------------------------------------------------------
// Mode
#define MODE_STOP 0
#define MODE_MANUAL 1
#define MODE_AUTO 2

uint8_t g_Mode;
uint8_t g_Action;

#define AP_TIMEOUT 1000
#define RC_TIMEOUT 100000
long g_tAP;
long g_tNow;

//----------------------------------------------------------------------
// CMD Protocol
#define ROVERCMD_STATE 0
#define ROVERCMD_SPEED 1
#define ROVERCMD_PINOUT 2
#define ROVERCMD_DEBUG 3
#define FLT_SCALE 1000
#define FLT_SCALE_INV 0.001

void runCMD(void)
{
  switch (g_cmd.m_pBuf[1])
  {
    case ROVERCMD_SPEED:
      g_nSpeed = ((float)(*((int16_t*)(&g_cmd.m_pBuf[3])))) * FLT_SCALE_INV;
      g_dSpeed = ((float)(*((int16_t*)(&g_cmd.m_pBuf[5])))) * FLT_SCALE_INV;
      g_tAP = millis();
      break;

    case ROVERCMD_PINOUT:
      digitalWrite(g_cmd.m_pBuf[3], g_cmd.m_pBuf[4]);
      g_tAP = millis();
      break;

    default:
      break;
  }
}

//----------------------------------------------------------------------
// Update
void updateMotor(void)
{
  int i;

  if (g_Mode == MODE_STOP)
  {
    for (i = 0; i < N_MOT; i++)
      g_pMot[i].speed(0.0);

    return;
  }

  if (g_Mode == MODE_MANUAL)
  {
    float pwm = (float)(g_pwmSpeed - PWM_MID);
    if (abs(pwm) > (float)PWM_MID_DZ)
      g_nSpeed = (pwm / (float)PWM_D) * (float)g_dirSpeed;
    else
      g_nSpeed = 0.0;

    pwm = (float)(g_pwmSteer - PWM_MID);
    if (abs(pwm) > (float)PWM_MID_DZ)
      g_dSpeed = (pwm / (float)PWM_D) * (float)g_dirSteer;
    else
      g_dSpeed = 0.0;
  }

  if(g_nSpeed < 0.0)
    g_dSpeed *= -1.0;

  g_pMot[0].speed(g_nSpeed + g_dSpeed);  //LF
  g_pMot[1].speed(g_nSpeed - g_dSpeed);  //RB
  g_pMot[2].speed(g_nSpeed - g_dSpeed);  //RF
  g_pMot[3].speed(g_nSpeed + g_dSpeed);  //LB
}

void updateMode()
{
  if (g_pwmMode <= PWM_LOWLIM
      || g_pwmAction <= PWM_LOWLIM
      || g_pwmSteer <= PWM_LOWLIM
      || g_pwmSpeed <= PWM_LOWLIM)
  {
    g_Mode = MODE_STOP;
    return;
  }

  if (abs(g_pwmAction - PWM_MID) < PWM_MID_DZ_BTN)
    g_Action = 2;
  else if (g_pwmAction > PWM_MID)
    g_Action = 3;
  else
    g_Action = 1;

  if (g_tNow - g_tAP > AP_TIMEOUT)
  {
    g_Mode = MODE_STOP;
    return;
  }

  if (abs(g_pwmMode - PWM_MID) < PWM_MID_DZ_BTN)
    g_Mode = MODE_MANUAL;
  else if (g_pwmMode > PWM_MID)
    g_Mode = MODE_AUTO;
  else
    g_Mode = MODE_STOP;
}

void updateLED(void)
{
  digitalWrite(PIN_LED_STOP, LOW);
  digitalWrite(PIN_LED_MANUAL, LOW);
  digitalWrite(PIN_LED_AUTO, LOW);

  if (g_Mode == MODE_STOP)
  {
    digitalWrite(PIN_LED_STOP, HIGH);
  }
  else if (g_Mode == MODE_MANUAL)
  {
    digitalWrite(PIN_LED_MANUAL, HIGH);
  }
  else
  {
    digitalWrite(PIN_LED_AUTO, HIGH);
  }
}


//----------------------------------------------------------------------
// Setup

void setup()
{
  g_pMot[0].init(PWM_MID, PWM_MID - PWM_D, PWM_MID + PWM_D, 3, -1); //LF
  g_pMot[1].init(PWM_MID, PWM_MID - PWM_D, PWM_MID + PWM_D, 5, 1);  //RB
  g_pMot[2].init(PWM_MID, PWM_MID - PWM_D, PWM_MID + PWM_D, 6, -1); //RF
  g_pMot[3].init(PWM_MID, PWM_MID - PWM_D, PWM_MID + PWM_D, 9, 1);  //LB

  g_nSpeed = 0.0;
  g_dSpeed = 0.0;
  g_dirSpeed = -1.0;
  g_dirSteer = -1.0;

  g_tAP = 0;
  g_Mode = MODE_STOP;
  g_Action = 1;

  pinMode(PIN_LED_STOP, OUTPUT);
  pinMode(PIN_LED_MANUAL, OUTPUT);
  pinMode(PIN_LED_AUTO, OUTPUT);
  pinMode(PIN_LED_INDICATOR, OUTPUT);

  pinMode(PIN_IN_PPM, INPUT);
  g_PPM.init(PIN_IN_PPM, 1, true);

  Serial.begin(115200);

  setupUDP();
  
}

void updateUDP(void)
{
  g_UDP.beginPacket(g_UDP.remoteIP(), g_UDP.remotePort());
  g_UDP.write(PROTOCOL_BEGIN);            //start mark
  g_UDP.write((uint8_t)ROVERCMD_STATE);   //cmd
  g_UDP.write(6);                         //payload len
  g_UDP.write(g_Mode);
  g_UDP.write(g_Action);
  int16_t v;
  v = g_nSpeed * FLT_SCALE;
  g_UDP.write((uint8_t)(v & 0xFF));
  g_UDP.write((uint8_t)((v >> 8) & 0xFF));
  v = g_dSpeed * FLT_SCALE;
  g_UDP.write((uint8_t)(v & 0xFF));
  g_UDP.write((uint8_t)((v >> 8) & 0xFF));
  g_UDP.endPacket();  
}

void debug(void)
{
#ifdef SERIAL_DEBUG
  Serial.print("steer=");
  Serial.print(g_pwmSteer);
  Serial.print(", speed=");
  Serial.print(g_pwmSpeed);
  Serial.print(", mode=");
  Serial.print(g_pwmMode);
  Serial.print(", action=");
  Serial.print(g_pwmAction);

  Serial.print(";  mode=");
  Serial.print(g_Mode);
  Serial.print(",  action=");
  Serial.print(g_Action);

  Serial.print(";  nSpeed=");
  Serial.print(g_nSpeed);
  Serial.print(", dSpeed=");
  Serial.print(g_dSpeed);

  Serial.print(";  m1=");
  Serial.print(g_pMot[0].m_pwm);
  Serial.print(", m2=");
  Serial.print(g_pMot[1].m_pwm);
  Serial.print(", m3=");
  Serial.print(g_pMot[2].m_pwm);
  Serial.print(", m4=");
  Serial.println(g_pMot[3].m_pwm);
#endif  
}

//----------------------------------------------------------------------
// Loop
void loop()
{
  g_pwmSteer = g_PPM.get(CHAN_STEER);
  g_pwmSpeed = g_PPM.get(CHAN_SPEED);
  g_pwmMode = g_PPM.get(CHAN_MODE);
  g_pwmAction = g_PPM.get(CHAN_ACTION);

  g_tNow = millis();

  updateMode();
  updateMotor();
  updateLED();

  g_UDP.parsePacket();
  decodeCMD();

  updateUDP();

  debug();
}
