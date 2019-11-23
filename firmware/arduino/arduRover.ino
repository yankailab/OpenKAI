#include <Ethernet.h>
#include <EthernetUdp.h>
#include "include/arduOK.h"
#include "include/arduPPMreader.h"
#include "include/arduMotorPWM.h"

//#define SERIAL_DEBUG

//----------------------------------------------------------------------
// IO override for Ethernet
// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress g_myIP(192, 168, 8, 250);
unsigned int g_myPort = 8888;
EthernetUDP g_UDP;

bool ioAvailable(void)
{
  return g_UDP.available();
}

byte ioRead(void)
{
  return g_UDP.read();
}

//----------------------------------------------------------------------
// Pin assign
#define PIN_LED_STOP A2
#define PIN_LED_MANUAL A0
#define PIN_LED_AUTO A1
#define PIN_LED_INDICATOR A3

//#define PIN_IN_STEER 4
//#define PIN_IN_SPEED 7
//#define PIN_IN_MODE 8
//#define PIN_IN_ACTION 2

#define PIN_IN_PPM 3
#define CHAN_STEER 0
#define CHAN_SPEED 1
#define CHAN_MODE 2
#define CHAN_ACTION 3
PPMreader g_PPM;

//----------------------------------------------------------------------
// Motor
#define N_MOT 4
MOTOR g_pMot[N_MOT];

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
// Mode
#define MODE_STOP 1
#define MODE_MANUAL 2
#define MODE_AUTO_1 3
#define MODE_AUTO_2 4
uint8_t g_Mode;

#define ACTION_1 1
#define ACTION_2 2
#define ACTION_3 3
uint8_t g_Action;

#define AP_TIMEOUT 1000
#define PWM_TIMEOUT 100000
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
    g_Action = ACTION_2;
  else if (g_pwmAction > PWM_MID)
    g_Action = ACTION_3;
  else
    g_Action = ACTION_1;


  if (g_tNow - g_tAP > AP_TIMEOUT)
  {
    g_Mode = MODE_STOP;
    return;
  }

  if (abs(g_pwmMode - PWM_MID) < PWM_MID_DZ_BTN)
    g_Mode = MODE_MANUAL;
  else if (g_pwmMode > PWM_MID)
    g_Mode = MODE_AUTO_1;
  else
    g_Mode = MODE_AUTO_2;

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
  }float g_pMotAP[N_MOT];
  
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
  g_pMot[1].init(PWM_MID, PWM_MID - PWM_D, PWM_MID + PWM_D, 5, 1); //RB
  g_pMot[2].init(PWM_MID, PWM_MID - PWM_D, PWM_MID + PWM_D, 6, -1); //RF
  g_pMot[3].init(PWM_MID, PWM_MID - PWM_D, PWM_MID + PWM_D, 9, 1); //LB

  g_pMotAP[0] = 0.0;
  g_pMotAP[1] = 0.0;
  g_pMotAP[2] = 0.0;
  g_pMotAP[3] = 0.0;

  g_nSpeed = 0.0;
  g_dSpeed = 0.0;
  g_dirSpeed = -1.0;
  g_dirSteer = -1.0;

  g_tAP = 0;
  g_Mode = MODE_STOP;
  g_Action = ACTION_1;

  pinMode(PIN_LED_STOP, OUTPUT);
  pinMode(PIN_LED_MANUAL, OUTPUT);
  pinMode(PIN_LED_AUTO, OUTPUT);
  pinMode(PIN_LED_INDICATOR, OUTPUT);

//  pinMode(PIN_IN_MODE, INPUT);
//  pinMode(PIN_IN_ACTION, INPUT);
//  pinMode(PIN_IN_STEER, INPUT);
//  pinMode(PIN_IN_SPEED, INPUT);

  pinMode(PIN_IN_PPM, INPUT);
  g_PPM.init();

  Serial.begin(115200);

  // configure the CS pin
  //Ethernet.init(10);  // Most Arduino shields

  while (1)
  {
    Ethernet.begin(mac, g_myIP);

    if (Ethernet.hardwareStatus() == EthernetNoHardware)
    {
      Serial.println("Ethernet device not found");
      continue;
    }

    if (Ethernet.hardwareStatus() == EthernetW5100)
      Serial.println("W5100");
    else if (Ethernet.hardwareStatus() == EthernetW5200)
      Serial.println("W5200");
    else if (Ethernet.hardwareStatus() == EthernetW5500)
      Serial.println("W5500");
    else
      Serial.println("Unknown Ethernet");

    if (Ethernet.linkStatus() == LinkOFF)
    {
      Serial.println("Ethernet cable is not connected.");
      continue;
    }

    break;
  }

  g_UDP.begin(g_myPort);
  g_nMsg = 2;
}

//----------------------------------------------------------------------
// Loop
void loop()
{
//  g_pwmSteer = pulseIn(PIN_IN_STEER, HIGH, PWM_TIMEOUT);
//  g_pwmSpeed = pulseIn(PIN_IN_SPEED, HIGH, PWM_TIMEOUT);
//  g_pwmMode = pulseIn(PIN_IN_MODE, HIGH, PWM_TIMEOUT);
//  g_pwmAction = pulseIn(PIN_IN_ACTION, HIGH, PWM_TIMEOUT);

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